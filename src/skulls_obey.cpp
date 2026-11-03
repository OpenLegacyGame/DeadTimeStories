#include "skulls_obey.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <sstream>
#include <stdexcept>

namespace deadtime {
namespace {
std::string numbered(size_t index) {
    return "skull_" + std::string(index < 9 ? "0" : "") + std::to_string(index + 1);
}
} // namespace
SkullsObey::~SkullsObey() = default;
SkullsObey::SkullsObey(const Assets& assets, const std::string& directory) : base_(normalize(directory)) {
    pugi::xml_document doc;
    const auto bytes = assets.read(base_ + "/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("skulls_obey"))
        throw std::runtime_error("Invalid skulls-obey configuration");
    const auto root = doc.child("skulls_obey");
    const auto count = root.child("skulls").attribute("quantity").as_uint();
    if (!count || count > 99)
        throw std::runtime_error("Invalid skulls-obey skull count");
    for (auto node : root.child("skulls").children("skull")) {
        if (node.attribute("index").as_uint(100) != angles_.size())
            throw std::runtime_error("Invalid skulls-obey index order");
        const double angle = node.attribute("angle").as_double();
        if (!std::isfinite(angle))
            throw std::runtime_error("Invalid skulls-obey angle");
        angles_.push_back(angle);
        sounds_.push_back(resolve(base_, node.attribute("sound").value()));
    }
    if (angles_.size() != count)
        throw std::runtime_error("Skulls-obey skull count mismatch");
    std::string sequence = root.child_value("sequence");
    std::replace(sequence.begin(), sequence.end(), ',', ' ');
    std::istringstream input(sequence);
    long long index;
    while (input >> index) {
        if (index < 0 || size_t(index) >= count)
            throw std::runtime_error("Invalid skulls-obey sequence index");
        sequence_.push_back(size_t(index));
    }
    if (!input.eof() || sequence_.size() < 2)
        throw std::runtime_error("Invalid skulls-obey sequence");
    speed_ = root.child("wheel_rotation_speed").attribute("deg_per_sec").as_double();
    fade_ = root.child("wheel_open").attribute("fade_in_time_ms").as_double() / 1000.;
    before_ = root.child("correct_phrase").attribute("delay_before_ms").as_double() / 1000.;
    after_ = root.child("correct_phrase").attribute("delay_after_start_ms").as_double() / 1000.;
    if (!std::isfinite(speed_) || speed_ <= 0 || !std::isfinite(fade_) || fade_ < 0 ||
        !std::isfinite(before_) || before_ < 0 || !std::isfinite(after_) || after_ < 0)
        throw std::runtime_error("Invalid skulls-obey timing");
    auto hook = [&](const char* name) {
        const std::string value = root.child(name).attribute("script").value();
        if (value.empty())
            return value;
        const auto path = resolve(base_, value) + ".lua";
        if (!assets.contains(path))
            throw std::runtime_error("Missing skulls-obey callback: " + path);
        return path;
    };
    onEnter = hook("onEnter");
    onExit = hook("onExit");
    onFinish = hook("onFinish");
    objects_ = loadActivityLayout(assets, base_ + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (const auto* id : {"back", "exit", "wheel", "lid", "reward", "fx_active_skull_eyes"})
        object(id);
    for (size_t i = 0; i < count; ++i)
        initialSkulls_.push_back(skull(i));
    eyes_ = std::make_unique<Object>();
    const auto& sprite = object("fx_active_skull_eyes");
    eyes_->id = sprite.id;
    eyes_->x = sprite.x;
    eyes_->y = sprite.y;
    eyes_->z = sprite.z;
    const auto layout = assets.read(base_ + "/layout.xml");
    doc.load_buffer(layout.data(), layout.size());
    for (auto node : doc.child("objects").children("object"))
        if (std::string(node.attribute("name").value()) == eyes_->id) {
            for (auto scene : node.children("scene")) {
                Object::SceneResource resource{resolve(base_, scene.attribute("src").value()),
                                               scene.attribute("looped").as_bool(true),
                                               {}};
                for (auto graph : scene.children("particles"))
                    resource.particles.push_back({resolve(base_, graph.attribute("src").value()), 0});
                eyes_->sceneResources.push_back(std::move(resource));
            }
            for (auto graph : node.children("particles"))
                eyes_->particleResources.push_back({resolve(base_, graph.attribute("src").value()), 0});
        }
    updateVisuals();
}
const ActivityObject& SkullsObey::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& o) { return o.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing skulls-obey object: " + id);
    return *found;
}
ActivityObject& SkullsObey::mutableObject(const std::string& id) {
    return const_cast<ActivityObject&>(object(id));
}
const ActivityObject& SkullsObey::skull(size_t index) const {
    return object(numbered(index));
}
Object& SkullsObey::eyes() {
    return *eyes_;
}
void SkullsObey::hint(const std::string& id) {
    events_.push_back({SkullEvent::Hint, "puzzles/skulls_obey/hints/" + id});
}
void SkullsObey::enter() {
    entered_ = true;
    phase_ = Idle;
    angle_ = angles_[selected_];
    history_.clear();
    events_.clear();
    eyes_->treeTime = 0;
    updateVisuals();
}
void SkullsObey::leave() {
    events_.push_back({SkullEvent::WheelLoop, {}});
    phase_ = Idle;
    angle_ = angles_[selected_];
    updateVisuals();
}
bool SkullsObey::select(size_t skull) {
    if (busy() || opened_ || skull >= angles_.size() || skull == selected_)
        return false;
    target_ = skull;
    angle_ = angles_[selected_];
    targetAngle_ = angles_[target_];
    // Native 51BF70 wraps only distances strictly greater than a half turn.
    if (std::abs(angle_ - targetAngle_) > 180) {
        if (targetAngle_ <= angle_)
            targetAngle_ += 360;
        else
            angle_ += 360;
    }
    velocity_ = targetAngle_ < angle_ ? -speed_ : speed_;
    phase_ = Turning;
    remaining_ = std::abs(targetAngle_ - angle_) / speed_;
    events_.push_back({SkullEvent::WheelLoop, "sound/sfx/mg/skulls_obey/mg_wheel_spin_loop"});
    updateVisuals();
    return true;
}
void SkullsObey::arrived() {
    selected_ = target_;
    angle_ = angles_[selected_];
    events_.push_back({SkullEvent::WheelLoop, {}});
    events_.push_back({SkullEvent::Sound, sounds_[selected_]});
    static const char* hints[] = {"first_skull",  "second_skull", "third_skull",
                                  "fourth_skull", "fifth_skull",  "sixth_skull"};
    if (selected_ < std::size(hints))
        hint(hints[selected_]);
    history_.push_back(selected_);
    // Native 517FD0 retains the longest suffix that is also an answer prefix.
    while (!history_.empty() && (history_.size() > sequence_.size() ||
                                 !std::equal(history_.begin(), history_.end(), sequence_.begin())))
        history_.erase(history_.begin());
    if (history_ == sequence_) {
        phase_ = BeforePhrase;
        remaining_ = before_;
    } else
        phase_ = Idle;
}
void SkullsObey::beginPhrase() {
    phase_ = AfterPhrase;
    remaining_ = after_;
    events_.push_back({SkullEvent::Sound, "sound/sfx/mg/skulls_obey/mg_i_listen_and_obey"});
}
void SkullsObey::open() {
    opened_ = true;
    phase_ = LidFade;
    remaining_ = fade_;
    events_.push_back({SkullEvent::Sound, "sound/sfx/mg/skulls_obey/mg_open_lid"});
    hint("full_phrase");
}
void SkullsObey::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid skulls-obey time advance");
    while (busy()) {
        const double step = std::min(seconds, remaining_);
        if (phase_ == Turning)
            angle_ += step * velocity_;
        remaining_ -= step;
        seconds -= step;
        if (remaining_ > 1e-10)
            break;
        if (phase_ == Turning)
            arrived();
        else if (phase_ == BeforePhrase)
            beginPhrase();
        else if (phase_ == AfterPhrase)
            open();
        else
            phase_ = Idle;
        if (seconds <= 1e-10 && remaining_ > 1e-10)
            break;
    }
    updateVisuals();
}
bool SkullsObey::takeReward() {
    if (busy() || !opened_ || complete_)
        return false;
    complete_ = true;
    events_.push_back({SkullEvent::Sound, "sound/sfx/mg/mg_finish"});
    events_.push_back({SkullEvent::Finish, {}});
    updateVisuals();
    return true;
}
bool SkullsObey::skip() {
    if (opened_)
        return false;
    // Native 517CA0 sends Open/Reinit directly, without the phrase or lid fade.
    events_.clear();
    phase_ = Idle;
    remaining_ = 0;
    angle_ = angles_[selected_];
    opened_ = true;
    events_.push_back({SkullEvent::WheelLoop, {}});
    events_.push_back({SkullEvent::Sound, "sound/sfx/mg/skulls_obey/mg_open_lid"});
    updateVisuals();
    return true;
}
void SkullsObey::inspectWheel() {
    if (!busy() && !opened_) {
        hint("wheel_click");
        idleClick();
    }
}
void SkullsObey::idleClick() {
    if (!busy())
        events_.push_back({SkullEvent::Sound, "sound/sfx/sfx_m_click_idle"});
}
void SkullsObey::updateVisuals() {
    double normalized = std::fmod(angle_, 360);
    if (normalized < 0)
        normalized += 360;
    auto& wheel = mutableObject("wheel");
    wheel.angle = float(normalized);
    wheel.interactive = !opened_;
    const double radians = normalized * 3.14159265358979323846 / 180;
    for (size_t i = 0; i < angles_.size(); ++i) {
        auto& mask = mutableObject(numbered(i));
        const auto& initial = initialSkulls_[i];
        const double dx = initial.x - wheel.x, dy = initial.y - wheel.y;
        mask.x = float(std::floor(wheel.x + dx * std::cos(radians) - dy * std::sin(radians) + .5));
        mask.y = float(std::floor(wheel.y + dx * std::sin(radians) + dy * std::cos(radians) + .5));
        mask.angle = float(normalized);
        mask.interactive = !opened_;
    }
    auto& lid = mutableObject("lid");
    lid.visible = !opened_ || phase_ == LidFade;
    lid.alpha = phase_ == LidFade && fade_ > 0 ? float(remaining_ / fade_) : 1;
    mutableObject("reward").visible = !complete_;
    eyes_->enabled = phase_ != Turning && (!opened_ || phase_ == LidFade);
}
std::vector<SkullEvent> SkullsObey::takeEvents() {
    auto result = std::move(events_);
    events_.clear();
    return result;
}
} // namespace deadtime
