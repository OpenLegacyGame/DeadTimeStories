#include "blocked_well.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>

namespace deadtime {
namespace {
std::string uppercase(std::string value) {
    for (auto& c : value)
        if (c >= 'a' && c <= 'z')
            c -= 'a' - 'A';
    return value;
}
std::string numbered(const std::string& prefix, size_t index) {
    return prefix + (index < 9 ? "0" : "") + std::to_string(index + 1);
}
float fraction(double age, double seconds) {
    return seconds <= 0 ? 1.f : std::clamp(float(age / seconds), 0.f, 1.f);
}
} // namespace
BlockedWell::BlockedWell(const Assets& assets, const std::string& base) {
    pugi::xml_document doc;
    const auto bytes = assets.read(base + "/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid blocked-well configuration");
    const auto root = doc.child("blocked_well");
    alphabet_ = uppercase(root.child_value("alphabet"));
    if (alphabet_.empty() || alphabet_.size() > 40)
        throw std::runtime_error("Invalid blocked-well alphabet");
    const auto names = root.child("names");
    font_ = resolve(base, names.attribute("font").value());
    color_ = unsigned(std::stoul(names.attribute("color").as_string("FFFFFFFF"), nullptr, 16));
    correctColor_ = unsigned(std::stoul(names.attribute("color_correct").as_string("FFFFFFFF"), nullptr, 16));
    for (auto name : names.children("name")) {
        const auto text = uppercase(name.child_value());
        if (text.empty() || std::find(names_.begin(), names_.end(), text) != names_.end())
            throw std::runtime_error("Empty or duplicate blocked-well name");
        names_.push_back(text);
    }
    if (names_.empty() || names_.size() > 20)
        throw std::runtime_error("Invalid blocked-well name count");
    known_.resize(names_.size());
    for (auto position : root.child("name_positions").children("position"))
        positions_.push_back({position.attribute("x").as_float(), position.attribute("y").as_float(),
                              position.attribute("z").as_float()});
    if (positions_.size() != names_.size())
        throw std::runtime_error("Blocked-well name/position count mismatch");
    plaqueWidth_ = root.child("name_plaque").attribute("width").as_float();
    const auto fade = root.child("fade_times");
    letterSeconds_ = fade.attribute("new_letter").as_double();
    invalidSeconds_ = fade.attribute("invalid_name").as_double();
    crossSeconds_ = fade.attribute("correct_name_cross_fade").as_double();
    progressSeconds_ = fade.attribute("progress_change_fade_in").as_double();
    for (auto value :
         {letterSeconds_, invalidSeconds_, crossSeconds_, progressSeconds_, double(plaqueWidth_)})
        if (!std::isfinite(value) || value <= 0)
            throw std::runtime_error("Invalid blocked-well fade/width");
    auto hook = [&](const char* name) {
        const std::string script = root.child(name).attribute("script").value();
        if (script.empty())
            return std::string{};
        const auto path = resolve(base, script) + ".lua";
        if (!assets.contains(path))
            throw std::runtime_error("Missing blocked-well callback: " + path);
        return path;
    };
    onEnter = hook("onEnter");
    onExit = hook("onExit");
    onFinish = hook("onFinish");
    objects_ = loadActivityLayout(assets, base + "/layout.xml");
    measureActivityTextures(assets, objects_);
    object("back");
    object("exit");
    for (size_t i = 0; i < alphabet_.size(); ++i) {
        letter(i);
        object(numbered("letter_", i) + "_hl");
    }
    updateVisuals();
}
std::optional<size_t> BlockedWell::nameIndex(const std::string& name) const {
    const auto text = uppercase(name);
    const auto found = std::find(names_.begin(), names_.end(), text);
    return found == names_.end() ? std::optional<size_t>{} : size_t(found - names_.begin());
}
void BlockedWell::addKnownName(const std::string& name) {
    if (const auto i = nameIndex(name))
        known_[*i] = true;
}
std::optional<bool> BlockedWell::isNameKnown(const std::string& name) const {
    const auto i = nameIndex(name);
    return i ? std::optional<bool>(known_[*i]) : std::nullopt;
}
std::optional<bool> BlockedWell::isNameSpelled(const std::string& name) const {
    const auto i = nameIndex(name);
    return i ? std::optional<bool>(std::find(spelled_.begin(), spelled_.end(), *i) != spelled_.end())
             : std::nullopt;
}
const ActivityObject& BlockedWell::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing blocked-well object: " + id);
    return *found;
}
const ActivityObject& BlockedWell::letter(size_t index) const {
    return object(numbered("letter_", index));
}
void BlockedWell::enter() {
    word_.clear();
    previous_.clear();
    phase_ = Phase::Idle;
    age_ = 0;
    hovered_.reset();
    events_.clear();
    if (!entered_)
        events_.push_back({WellEvent::Hint, "puzzles/blocked_well/hints/first_enter"});
    entered_ = true;
    updateVisuals();
}
void BlockedWell::hover(std::optional<size_t> letter) {
    hovered_ = letter && *letter < alphabet_.size() ? letter : std::nullopt;
    updateVisuals();
}
bool BlockedWell::allNamesKnown() const {
    return std::all_of(known_.begin(), known_.end(), [](bool known) { return known; });
}
bool BlockedWell::skip() {
    if (complete_ || !allNamesKnown())
        return false;
    // Native 4E1A50 preserves spelled names and appends each remaining name.
    events_.clear();
    phase_ = Phase::Idle;
    age_ = 0;
    word_.clear();
    previous_.clear();
    hovered_.reset();
    for (size_t i = 0; i < names_.size(); ++i)
        if (std::find(spelled_.begin(), spelled_.end(), i) == spelled_.end())
            spelled_.push_back(i);
    complete_ = true;
    events_.push_back({WellEvent::Finish, {}});
    updateVisuals();
    return true;
}
bool BlockedWell::select(size_t letter) {
    if (complete_ || busy() || letter >= alphabet_.size())
        return false;
    previous_ = word_;
    word_ += alphabet_[letter];
    age_ = 0;
    phase_ = Phase::Letter;
    events_.push_back({WellEvent::Sound, "sound/sfx/sfx_m_click_idle"});
    updateVisuals();
    return true;
}
void BlockedWell::resolveWord() {
    bool prefix = false;
    std::optional<size_t> match;
    for (size_t i = 0; i < names_.size(); ++i)
        if (known_[i] && std::find(spelled_.begin(), spelled_.end(), i) == spelled_.end() &&
            names_[i].starts_with(word_)) {
            prefix = true;
            if (names_[i] == word_)
                match = i;
        }
    previous_.clear();
    age_ = 0;
    if (!prefix) {
        phase_ = Phase::Invalid;
        events_.push_back({WellEvent::Sound, "sound/sfx/mg/mg_wrong_name"});
    } else if (!match)
        phase_ = Phase::Idle;
    else {
        spelled_.push_back(*match);
        phase_ = Phase::Correct;
        events_.push_back({WellEvent::Sound, "sound/sfx/mg/mg_high_light"});
        if (spelled_.size() == names_.size()) {
            complete_ = true;
            events_.push_back({WellEvent::Finish, {}});
        }
    }
}
void BlockedWell::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid blocked-well time advance");
    while (phase_ != Phase::Idle) {
        const double limit = phase_ == Phase::Letter    ? letterSeconds_
                             : phase_ == Phase::Invalid ? invalidSeconds_
                                                        : std::max(crossSeconds_, progressSeconds_);
        const double step = std::min(seconds, std::max(0., limit - age_));
        age_ += step;
        seconds -= step;
        if (age_ + 1e-10 < limit)
            break;
        if (phase_ == Phase::Letter)
            resolveWord();
        else {
            phase_ = Phase::Idle;
            age_ = 0;
            word_.clear();
            previous_.clear();
        }
        if (seconds <= 0)
            break;
    }
    updateVisuals();
}
void BlockedWell::updateVisuals() {
    for (auto& object : objects_) {
        if (object.id.starts_with("progress_")) {
            const auto index = size_t(std::stoul(object.id.substr(9)));
            // Native scene lookup collects every node with this ID. The two
            // progress_07 nodes reveal U and S together.
            object.alpha = index <= spelled_.size() ? 1 : 0;
            if (phase_ == Phase::Correct && index == spelled_.size())
                object.alpha = fraction(age_, progressSeconds_);
        } else if (object.id.starts_with("letter_") && object.id.ends_with("_hl")) {
            const auto index = size_t(std::stoul(object.id.substr(7))) - 1;
            object.visible = !busy() && hovered_ == index;
        }
    }
}
std::vector<WellPlaque> BlockedWell::plaques() const {
    std::vector<WellPlaque> result;
    for (size_t rank = 0; rank < spelled_.size(); ++rank) {
        const auto& p = positions_[rank];
        const float alpha =
            phase_ == Phase::Correct && rank + 1 == spelled_.size() ? fraction(age_, crossSeconds_) : 1;
        result.push_back({names_[spelled_[rank]], p.x, p.y, p.z, alpha, true});
    }
    if (!word_.empty()) {
        const size_t rank = phase_ == Phase::Correct ? spelled_.size() - 1 : spelled_.size();
        const auto& p = positions_.at(rank);
        const float alpha = phase_ == Phase::Letter    ? fraction(age_, letterSeconds_)
                            : phase_ == Phase::Invalid ? 1 - fraction(age_, invalidSeconds_)
                            : phase_ == Phase::Correct ? 1 - fraction(age_, crossSeconds_)
                                                       : 1;
        result.push_back({word_, p.x, p.y, p.z, alpha, false});
        if (!previous_.empty() && phase_ == Phase::Letter)
            result.push_back({previous_, p.x, p.y, p.z, 1 - fraction(age_, letterSeconds_), false});
    }
    return result;
}
std::vector<WellEvent> BlockedWell::takeEvents() {
    auto result = std::move(events_);
    events_.clear();
    return result;
}
} // namespace deadtime
