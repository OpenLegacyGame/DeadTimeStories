#include "leeches.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <pugixml.hpp>
#include <set>
#include <stdexcept>

namespace deadtime {
bool Leeches::skip() {
    if (complete_)
        return false;
    // Native 4F05E0 sends both Solve and Finish, including the wringer step.
    events_.clear();
    held_.reset();
    orange_.clear();
    for (auto& state : states_) {
        state.removed = true;
        state.caught = false;
        state.phase = LeechState::Dormant;
        state.age = state.delay = 0;
    }
    solved_ = complete_ = true;
    events_.push_back({LeechesEvent::Finish, {}});
    updateVisuals();
    return true;
}
namespace {
std::string leechName(size_t index) {
    return "leech_" + std::string(index < 9 ? "0" : "") + std::to_string(index + 1);
}
} // namespace
Leeches::Leeches(const Assets& assets, const std::string& base, uint32_t seed) : random_(seed) {
    pugi::xml_document doc;
    const auto bytes = assets.read(base + "/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("leeches_puzzle"))
        throw std::runtime_error("Invalid leeches configuration");
    const auto root = doc.child("leeches_puzzle"), config = root.child("leeches");
    const unsigned quantity = config.attribute("quantity").as_uint();
    minMs_ = config.attribute("appear_interval_min_ms").as_uint();
    maxMs_ = config.attribute("appear_interval_max_ms").as_uint();
    if (!quantity || quantity > 10 || !minMs_ || maxMs_ <= minMs_)
        throw std::runtime_error("Invalid leech count or appearance intervals");
    states_.resize(quantity);
    auto hook = [&](const char* name) {
        const std::string script = root.child(name).attribute("script").value();
        if (script.empty())
            return std::string{};
        const auto path = resolve(base, script) + ".lua";
        if (!assets.contains(path))
            throw std::runtime_error("Missing leeches callback: " + path);
        return path;
    };
    onEnter = hook("onEnter");
    onExit = hook("onExit");
    onFinish = hook("onFinish");
    objects_ = loadActivityLayout(assets, base + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (const auto* id : {"back", "exit", "bucket", "tablecloth", "roll", "swamp"})
        object(id);
    std::set<std::string> pictures;
    for (size_t i = 0; i < quantity; ++i) {
        initialLeeches_.push_back(leech(i));
        animations_.emplace_back(assets, base + "/leech/leech_" + std::to_string(i + 1) + ".animset");
        const auto& animation = animations_.back();
        if (animation.looped() || animation.duration() <= 0 || animation.pictures().empty())
            throw std::runtime_error("Invalid leech appearance animation");
        for (const auto& track : animation.pictures())
            for (const auto& key : track.keys)
                pictures.insert(key.texture);
    }
    std::vector<ActivityObject> frames;
    for (const auto& texture : pictures) {
        ActivityObject frame;
        frame.texture = texture;
        frames.push_back(frame);
    }
    measureActivityTextures(assets, frames);
    for (const auto& frame : frames)
        textureSizes_[frame.texture] = {frame.width, frame.height};
}
const ActivityObject& Leeches::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing leeches object: " + id);
    return *found;
}
ActivityObject& Leeches::mutableObject(const std::string& id) {
    return const_cast<ActivityObject&>(object(id));
}
const ActivityObject& Leeches::leech(size_t index) const {
    return object(leechName(index));
}
bool Leeches::isOrange(const std::string& item) {
    return item == "orange_1" || item == "orange_2" || item == "orange_3";
}
size_t Leeches::removedCount() const {
    return std::count_if(states_.begin(), states_.end(), [](const auto& state) { return state.removed; });
}
double Leeches::delay() {
    return double(minMs_ + random_.next() % (maxMs_ - minMs_)) * .001;
}
void Leeches::sound(const std::string& path) {
    events_.push_back({LeechesEvent::Sound, path});
}
void Leeches::hint(const std::string& id, bool reactive) {
    events_.push_back({LeechesEvent::Hint, "puzzles/leeches/hints/" + id});
    if (reactive)
        sound("sound/sfx/sfx_reactive_pop_up");
}
void Leeches::enter() {
    events_.clear();
    held_.reset();
    orange_.clear();
    for (size_t i = 0; i < states_.size(); ++i) {
        const bool removed = states_[i].removed;
        states_[i] = {};
        states_[i].removed = removed;
        states_[i].delay = delay();
        mutableObject(leechName(i)) = initialLeeches_[i];
    }
    updateVisuals();
}
void Leeches::leave() {
    held_.reset();
    orange_.clear();
    for (auto& state : states_) {
        state.caught = false;
        state.phase = LeechState::Dormant;
    }
    updateVisuals();
}
void Leeches::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid leeches time advance");
    for (unsigned guard = 0;; ++guard) {
        if (guard > 100000)
            throw std::runtime_error("Excessive leeches time advance");
        double next = std::numeric_limits<double>::infinity();
        for (size_t i = 0; i < states_.size(); ++i) {
            const auto& state = states_[i];
            if (state.phase == LeechState::Waiting)
                next = std::min(next, std::max(0., state.delay));
            else if (state.phase == LeechState::Animating)
                next = std::min(next, std::max(0., animations_[i].duration() - state.age));
        }
        const double step = std::min(seconds, next);
        for (auto& state : states_) {
            if (state.phase == LeechState::Waiting)
                state.delay -= step;
            else if (state.phase == LeechState::Animating)
                state.age += step;
        }
        seconds -= step;
        if (next > step + 1e-10)
            break;
        for (size_t i = 0; i < states_.size(); ++i) {
            auto& state = states_[i];
            if (state.phase == LeechState::Animating && state.age + 1e-10 >= animations_[i].duration()) {
                state.phase = LeechState::Waiting;
                state.delay = delay();
            } else if (state.phase == LeechState::Waiting && state.delay <= 1e-10) {
                if (state.removed || state.caught)
                    state.phase = LeechState::Dormant;
                else {
                    state.phase = LeechState::Animating;
                    state.age = 0;
                    sound("sound/sfx/mg/leeches/mg_leeches_come_up");
                }
            }
        }
        if (seconds <= 0)
            break;
    }
    updateVisuals();
}
bool Leeches::catchLeech(size_t index, const std::string& inventory) {
    if (holding() || complete_ || index >= states_.size() || !leech(index).visible)
        return false;
    if (inventory.empty()) {
        hint("leech_click_no_blood_orange");
        sound("sound/sfx/sfx_m_click_idle");
        return false;
    }
    if (!isOrange(inventory)) {
        hint("use_wrong_item", false);
        return false;
    }
    states_[index].caught = true;
    held_ = index;
    orange_ = inventory;
    sound("sound/sfx/sfx_inv_item_pick");
    if (!removedCount())
        hint("first_leech_caught", false);
    else if (removedCount() + 1 == states_.size())
        hint("last_leech_caught", false);
    updateVisuals();
    return true;
}
bool Leeches::throwIntoSwamp() {
    if (!held_ || complete_)
        return false;
    auto& state = states_[*held_];
    state.removed = true;
    state.caught = false;
    held_.reset();
    orange_.clear();
    if (removedCount() == states_.size() && !solved_) {
        solved_ = true;
        hint("all_leeches_removed");
    }
    sound("sound/sfx/mg/leeches/mg_leeches_thrown");
    updateVisuals();
    return true;
}
void Leeches::refuseCancel() {
    if (holding())
        sound("sound/sfx/sfx_m_click_idle");
}
void Leeches::requireSwamp() {
    if (holding())
        hint("need_to_throw_leech_into_swamp");
}
void Leeches::inspect(const std::string& target) {
    if (complete_)
        return;
    if (holding()) {
        if (target == "swamp")
            throwIntoSwamp();
        else
            requireSwamp();
        return;
    }
    if (target == "roll" && solved_) {
        complete_ = true;
        events_.push_back({LeechesEvent::Finish, {}});
        return;
    }
    if (target == "bucket" && !solved_)
        hint("bucket_click_leeches_still_there");
    else if (target == "tablecloth")
        hint(solved_ ? "tablecloth_stuck" : "tablecloth_covered_with_leeches");
    else if (target == "roll" && !solved_)
        hint("tablecloth_covered_with_leeches");
    sound("sound/sfx/sfx_m_click_idle");
}
void Leeches::updateVisuals() {
    for (size_t i = 0; i < states_.size(); ++i) {
        const auto& state = states_[i];
        auto& sprite = mutableObject(leechName(i));
        if (state.phase == LeechState::Animating) {
            animations_[i].apply(sprite, state.age);
            const auto size = textureSizes_.at(sprite.texture);
            sprite.width = size.first;
            sprite.height = size.second;
        }
        sprite.visible = sprite.interactive =
            state.phase == LeechState::Animating && !state.removed && !state.caught;
    }
}
std::vector<LeechesEvent> Leeches::takeEvents() {
    auto result = std::move(events_);
    events_.clear();
    return result;
}
} // namespace deadtime
