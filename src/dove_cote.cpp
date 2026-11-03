#include "dove_cote.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace deadtime {
bool DoveCote::skip() {
    if (complete_)
        return false;
    // Native 4C2140 sends Finish/Reinit; the solved view retains the bird.
    phase_ = Phase::Idle;
    age_ = 0;
    events_.clear();
    excludedDoor_.reset();
    fed_ = inside_ = complete_ = true;
    path_ = reference_;
    std::fill(closed_.begin(), closed_.end(), true);
    updateVisuals();
    return true;
}
namespace {
std::string numbered(const std::string& prefix, size_t index) {
    return prefix + (index < 9 ? "0" : "") + std::to_string(index + 1);
}
std::vector<size_t> indexes(const std::string& text, size_t count, char separator) {
    std::istringstream input(text);
    std::vector<size_t> result;
    std::string field;
    while (std::getline(input, field, separator)) {
        size_t used = 0;
        const auto value = std::stoul(field, &used);
        if (used != field.size() || value < 1 || value > count)
            throw std::runtime_error("Invalid dove cote roll index");
        result.push_back(value - 1);
    }
    return result;
}
} // namespace
DoveCote::DoveCote(const Assets& assets, const std::string& path, uint32_t seed)
    : base_(normalize(path)), random_(seed) {
    pugi::xml_document doc;
    const auto data = assets.read(base_ + "/_config.xml");
    if (!doc.load_buffer(data.data(), data.size()) || !doc.child("dove_cote"))
        throw std::runtime_error("Invalid dove cote configuration");
    const auto root = doc.child("dove_cote");
    auto hook = [&](const char* name) {
        const std::string source = root.child(name).attribute("script").value();
        const auto value = source.empty() ? std::string{} : resolve(base_, source) + ".lua";
        if (!value.empty() && !assets.contains(value))
            throw std::runtime_error("Missing dove cote script: " + value);
        return value;
    };
    onEnter = hook("onEnter");
    onExit = hook("onExit");
    onFinish = hook("onFinish");
    const auto rolls = root.child("rolls");
    count_ = rolls.attribute("quantity").as_uint();
    if (count_ < 2 || count_ > 20)
        throw std::runtime_error("Invalid dove cote roll count");
    reference_ = indexes(rolls.attribute("correct_sequence").value(), count_, ',');
    if (reference_.size() < 2 || reference_.front() == reference_.back())
        throw std::runtime_error("Invalid dove cote reference endpoints");
    std::istringstream input(rolls.attribute("locks").value());
    std::string pair;
    while (std::getline(input, pair, ';')) {
        const auto values = indexes(pair, count_, ',');
        if (values.size() != 2 || values[0] == values[1])
            throw std::runtime_error("Invalid dove cote lock");
        locks_.emplace_back(std::min(values[0], values[1]), std::max(values[0], values[1]));
    }
    if (locks_.empty() || locks_.size() > 10)
        throw std::runtime_error("Invalid dove cote lock count");
    closed_.resize(locks_.size(), false);
    arrivalDelay = root.child("dove_fly_inside").attribute("delay_ms").as_double() / 1000;
    if (!std::isfinite(arrivalDelay) || arrivalDelay < 0)
        throw std::runtime_error("Invalid dove arrival delay");
    objects_ = loadActivityLayout(assets, base_ + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (const auto* name : {"back", "exit", "bird_feeder", "seeds", "dove"})
        object(name);
    for (size_t i = 0; i < count_; ++i) {
        roll(i);
        object(numbered("rope_knot_", i));
    }
    for (size_t i = 0; i < locks_.size(); ++i) {
        object(numbered("door_open_", i));
        object(numbered("door_closed_", i));
        object(numbered("lock_pipe_", i));
        incoming_.push_back(std::make_unique<AnimationSet>(assets, base_ + "/dove_fly_inside/" +
                                                                       numbered("", i) + ".animset"));
        outgoing_.push_back(
            std::make_unique<AnimationSet>(assets, base_ + "/dove_fly_away/" + numbered("", i) + ".animset"));
    }
    maxLength_ = pathLength(reference_) + 60;
    std::vector<ActivityObject> rope(1);
    rope.front().texture = base_ + "/rope_tile";
    measureActivityTextures(assets, rope);
    ropeHeight_ = rope.front().height;
    firstRope_ = objects_.size();
    updateVisuals();
}
const ActivityObject& DoveCote::object(const std::string& name) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == name; });
    if (found == objects_.end())
        throw std::runtime_error("Missing dove cote object: " + name);
    return *found;
}
ActivityObject& DoveCote::mutableObject(const std::string& name) {
    return const_cast<ActivityObject&>(object(name));
}
const ActivityObject& DoveCote::roll(size_t index) const {
    if (index >= count_)
        throw std::runtime_error("Unknown dove roll");
    return object(numbered("roll_", index));
}
std::pair<float, float> DoveCote::center(size_t index) const {
    const auto& item = roll(index);
    return {item.x + item.width / 2, item.y + item.height / 2};
}
double DoveCote::pathLength(const std::vector<size_t>& path) const {
    double length = 0;
    for (size_t i = 1; i < path.size(); ++i) {
        const auto& a = roll(path[i - 1]);
        const auto& b = roll(path[i]);
        length += std::hypot(b.x - a.x, b.y - a.y);
    }
    return length;
}
double DoveCote::length() const {
    return pathLength(path_);
}
bool DoveCote::secured() const {
    return path_.size() >= 2 && ((path_.front() == reference_.front() && path_.back() == reference_.back()) ||
                                 (path_.front() == reference_.back() && path_.back() == reference_.front()));
}
bool DoveCote::locked(size_t pipe) const {
    const auto [a, b] = locks_.at(pipe);
    for (size_t i = 1; i < path_.size(); ++i)
        if ((path_[i - 1] == a && path_[i] == b) || (path_[i - 1] == b && path_[i] == a))
            return true;
    return false;
}
bool DoveCote::correct() const {
    for (size_t i = 0; i < locks_.size(); ++i)
        if (!locked(i))
            return false;
    return secured();
}
void DoveCote::message(const std::string& key) {
    events_.push_back({DoveEvent::Hint, "puzzles/dove_cote/hints/" + key});
}
bool DoveCote::feed() {
    if (busy() || complete_)
        return false;
    if (inside_) {
        message("feeder_already_full");
        return false;
    }
    fed_ = true;
    phase_ = Phase::ArrivalWait;
    age_ = 0;
    events_.push_back({DoveEvent::Sound, "sound/sfx/mg/dove_cote/mg_seeds_use"});
    updateVisuals();
    return true;
}
bool DoveCote::togglePipe(size_t pipe) {
    if (pipe >= locks_.size() || busy() || complete_ || pulling())
        return false;
    events_.push_back({DoveEvent::Sound, "sound/sfx/sfx_m_click_idle"});
    if (!inside_) {
        message("need_to_lure_dove_before_closing_the_doors");
        return false;
    }
    closed_[pipe] = !closed_[pipe];
    updateVisuals();
    return true;
}
void DoveCote::watch(std::optional<size_t> excluded) {
    excludedDoor_ = excluded;
    phase_ = Phase::Watch;
    age_ = 0;
}
bool DoveCote::clickDoor(size_t door) {
    if (door >= closed_.size() || busy() || complete_ || pulling())
        return false;
    events_.push_back({DoveEvent::Sound, "sound/sfx/sfx_m_click_idle"});
    if (!inside_) {
        message("need_to_lure_dove_before_closing_the_doors");
        return false;
    }
    if (closed_[door]) {
        message("door_already_closed");
        return false;
    }
    watch(door);
    return true;
}
bool DoveCote::connect(size_t index) {
    if (index >= count_ || busy() || complete_)
        return false;
    if (path_.empty()) {
        if (!inside_) {
            message("need_to_lure_dove_before_using_rope");
            return false;
        }
        if (std::find(closed_.begin(), closed_.end(), false) != closed_.end()) {
            message("need_to_close_all_doors_before_locking_them");
            return false;
        }
        if (index != reference_.front() && index != reference_.back()) {
            message("wrong_start_roll");
            return false;
        }
        path_.push_back(index);
        auto [x, y] = center(index);
        mouseX_ = x;
        mouseY_ = y;
        message("rope_secured_on_first_ring");
        updateVisuals();
        return true;
    }
    if (index == path_.back())
        return false;
    auto [x, y] = center(index);
    const auto [fromX, fromY] = center(path_.back());
    if (length() + std::hypot(x - fromX, y - fromY) > maxLength_) {
        message("rope_too_short");
        return false;
    }
    for (size_t i = 0; i < locks_.size(); ++i) {
        const auto pair = std::minmax(index, path_.back());
        if (pair.first == locks_[i].first && pair.second == locks_[i].second && locked(i)) {
            message("pipe_already_locked");
            return false;
        }
    }
    if (index == path_.front()) {
        message("ring_already_used");
        return false;
    }
    if (std::count(path_.begin(), path_.end(), index) >= 2) {
        message("cannot_use_one_roll_more_than_twice");
        return false;
    }
    if (path_.size() > 1 && index == path_[path_.size() - 2])
        path_.pop_back();
    else
        path_.push_back(index);
    mouseX_ = x;
    mouseY_ = y;
    if (secured()) {
        message(correct() ? "rope_secured_on_both_rings_correct" : "rope_secured_on_both_rings_wrong");
        watch();
    } else if (path_.size() > 1 && !helpShown_) {
        helpShown_ = true;
        events_.push_back({DoveEvent::Help, "puzzles/dove_cote/hints/rope_help"});
    }
    events_.push_back({DoveEvent::Sound, "sound/sfx/sfx_m_click_idle"});
    updateVisuals();
    return true;
}
bool DoveCote::undo() {
    if (!pulling() || busy())
        return false;
    path_.pop_back();
    if (!path_.empty()) {
        auto [x, y] = center(path_.back());
        mouseX_ = x;
        mouseY_ = y;
    }
    events_.push_back({DoveEvent::Sound, "sound/sfx/sfx_m_click_idle"});
    updateVisuals();
    return true;
}
void DoveCote::moveRope(float x, float y) {
    if (!std::isfinite(x) || !std::isfinite(y))
        throw std::runtime_error("Invalid dove rope position");
    mouseX_ = x;
    mouseY_ = y;
    if (pulling()) {
        const auto [a, b] = center(path_.back());
        const double distance = std::hypot(x - a, y - b), remaining = std::max(0., maxLength_ - length());
        if (distance > remaining) {
            mouseX_ = float(a + (x - a) * remaining / distance);
            mouseY_ = float(b + (y - b) * remaining / distance);
        }
        updateVisuals();
    }
}
void DoveCote::updateVisuals() {
    objects_.resize(firstRope_);
    mutableObject("seeds").visible = fed_ || inside_;
    for (size_t i = 0; i < locks_.size(); ++i) {
        auto& open = mutableObject(numbered("door_open_", i));
        auto& closed = mutableObject(numbered("door_closed_", i));
        open.alpha = closed_[i] ? 0 : 1;
        closed.alpha = closed_[i] ? 1 : 0;
        if (phase_ == Phase::DoorOpening && i == flightDoor_ && openingWasClosed_) {
            open.alpha = float(std::clamp(age_, 0., 1.));
            closed.alpha = 1 - open.alpha;
        }
        auto& pipe = mutableObject(numbered("lock_pipe_", i));
        pipe.texture =
            base_ + "/lock_pipes/" + (closed_[i] ? "up_level_" : "down_level_") + std::to_string(i + 1);
        pipe.width = pipe.height = 0;
    }
    for (size_t i = 0; i < count_; ++i)
        mutableObject(numbered("rope_knot_", i)).visible =
            std::find(path_.begin(), path_.end(), i) != path_.end();
    auto line = [&](float ax, float ay, float bx, float by, float z) {
        ActivityObject segment;
        segment.id = "rope_segment";
        segment.texture = base_ + "/rope_tile";
        segment.x = ax;
        segment.y = ay;
        segment.z = z;
        segment.width = std::hypot(bx - ax, by - ay);
        segment.height = ropeHeight_;
        segment.pivotY = ropeHeight_ / 2;
        segment.repeatX = true;
        segment.angle = float(std::atan2(by - ay, bx - ax) * 180 / 3.14159265358979323846);
        segment.interactive = false;
        objects_.push_back(std::move(segment));
    };
    for (size_t i = 1; i < path_.size(); ++i) {
        const auto [ax, ay] = center(path_[i - 1]);
        const auto [bx, by] = center(path_[i]);
        line(ax, ay, bx, by, 0);
    }
    if (pulling()) {
        const auto [ax, ay] = center(path_.back());
        line(ax, ay, mouseX_, mouseY_, -50);
    }
    auto& dove = mutableObject("dove");
    dove.visible = phase_ == Phase::Incoming || phase_ == Phase::Outgoing;
    if (phase_ == Phase::Incoming)
        incoming_[flightDoor_]->apply(dove, age_);
    if (phase_ == Phase::Outgoing)
        outgoing_[flightDoor_]->apply(dove, age_);
}
void DoveCote::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid dove cote time");
    while (seconds > 0 && busy()) {
        const auto duration = phase_ == Phase::ArrivalWait   ? arrivalDelay
                              : phase_ == Phase::Incoming    ? incoming_[flightDoor_]->duration()
                              : phase_ == Phase::Watch       ? 1.5
                              : phase_ == Phase::DoorOpening ? 1.
                                                             : outgoing_[flightDoor_]->duration();
        const auto step = std::min(seconds, std::max(0., duration - age_));
        seconds -= step;
        age_ += step;
        if (age_ + 1e-10 < duration)
            break;
        age_ = 0;
        if (phase_ == Phase::ArrivalWait) {
            std::vector<size_t> open;
            for (size_t i = 0; i < closed_.size(); ++i)
                if (!closed_[i])
                    open.push_back(i);
            if (open.empty())
                throw std::runtime_error("No open dove arrival door");
            flightDoor_ = open[random_() % open.size()];
            phase_ = Phase::Incoming;
            events_.push_back({DoveEvent::Sound, "sound/sfx/mg/dove_cote/mg_dove_arrived"});
        } else if (phase_ == Phase::Incoming) {
            inside_ = true;
            phase_ = Phase::Idle;
            message("dove_appear");
        } else if (phase_ == Phase::Watch) {
            if (correct()) {
                complete_ = true;
                phase_ = Phase::Idle;
            } else {
                std::vector<size_t> unlocked;
                for (size_t i = 0; i < locks_.size(); ++i)
                    if (!locked(i) && excludedDoor_ != i)
                        unlocked.push_back(i);
                if (unlocked.empty())
                    throw std::runtime_error("No unlocked dove escape door");
                flightDoor_ = unlocked[random_() % unlocked.size()];
                openingWasClosed_ = closed_[flightDoor_];
                closed_[flightDoor_] = false;
                phase_ = Phase::DoorOpening;
            }
        } else if (phase_ == Phase::DoorOpening) {
            phase_ = Phase::Outgoing;
            events_.push_back({DoveEvent::Sound, "sound/sfx/mg/dove_cote/mg_dove_gone"});
        } else {
            inside_ = fed_ = false;
            path_.clear();
            phase_ = Phase::Idle;
            events_.push_back({DoveEvent::ReturnRope, {}});
            message("dove_escaped");
        }
    }
    updateVisuals();
}
void DoveCote::leave() {
    if (pulling())
        path_.clear();
    phase_ = Phase::Idle;
    fed_ = inside_;
    updateVisuals();
}
void DoveCote::enter() {
    if (!complete_ && secured())
        watch();
    updateVisuals();
}
std::vector<DoveEvent> DoveCote::takeEvents() {
    return std::exchange(events_, {});
}
} // namespace deadtime
