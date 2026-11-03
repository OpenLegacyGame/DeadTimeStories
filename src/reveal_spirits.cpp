#include "reveal_spirits.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>
#include <utility>

namespace deadtime {
RevealSpirits::RevealSpirits(const Assets& assets, std::vector<Target> targets, std::string item,
                             Object& container)
    : brush(std::move(item)), targets_(std::move(targets)), container_(&container) {
    const std::string base = "minigames/reveal_spirits";
    pugi::xml_document doc;
    const auto bytes = assets.read(base + "/config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("reveal_spirits"))
        throw std::runtime_error("Invalid reveal spirits configuration");
    const auto root = doc.child("reveal_spirits");
    revealSpeed = root.child("reveal_speed").attribute("alpha_per_second").as_double();
    firstTouchBoost = root.child("reveal_speed").attribute("first_touch_boost").as_double();
    vanishSpeed = root.child("vanish_speed").attribute("alpha_per_second").as_double();
    movementSeconds = root.child("min_mouse_move_frequency").attribute("ms").as_double() / 1000.;
    const auto sounds = root.child("sounds");
    vanishSoundDelay =
        sounds.child("spirit_vanish").attribute("no_repeat_sound_delay_ms").as_double() / 1000.;
    for (const auto value : {revealSpeed, firstTouchBoost, vanishSpeed, movementSeconds, vanishSoundDelay})
        if (!std::isfinite(value) || value < 0)
            throw std::runtime_error("Invalid reveal spirits rate or delay");
    if (revealSpeed == 0 || vanishSpeed == 0 || targets_.empty())
        throw std::runtime_error("Missing reveal spirits speed or targets");
    auto sound = [&](pugi::xml_node node, const char* attribute) {
        const std::string value = node.attribute(attribute).value();
        if (value.empty())
            return std::string{};
        const auto result = resolve(base, value);
        if (!assets.contains(result + ".ogg"))
            throw std::runtime_error("Missing reveal spirits audio: " + result);
        return result;
    };
    brushSound = sound(sounds.child("brush"), "src");
    revealedSound = sound(sounds.child("spirit_revealed"), "src");
    maleVanish_ = sound(sounds.child("spirit_vanish"), "male");
    femaleVanish_ = sound(sounds.child("spirit_vanish"), "female");
    for (auto spirit : root.child("female_spirits").children("spirit"))
        females_.insert(spirit.attribute("name").value());
    const std::string script = root.child("onExit").attribute("script").value();
    if (!script.empty()) {
        onExit = resolve(base, script) + ".lua";
        if (!assets.contains(onExit))
            throw std::runtime_error("Missing reveal spirits exit script");
    }
    std::set<Object*> unique;
    for (size_t i = 0; i < targets_.size(); ++i) {
        const auto& target = targets_[i];
        if (!target.spirit || !target.mask || !unique.insert(target.spirit).second)
            throw std::runtime_error("Invalid or duplicate reveal spirit");
        pending_.push_back(i);
    }
}
bool RevealSpirits::pending(size_t index) const {
    return std::find(pending_.begin(), pending_.end(), index) != pending_.end();
}
void RevealSpirits::move(int x, int y, std::optional<size_t> hit) {
    if (hit && *hit >= targets_.size())
        throw std::runtime_error("Unknown reveal spirit hit");
    position_ = {x, y};
    hit_ = hit;
}
void RevealSpirits::pauseMotion() {
    stationaryAge_ = movementSeconds + 1;
    previous_ = position_;
    hit_.reset();
}
void RevealSpirits::tick(double seconds, const std::function<void(Object&)>& onRevealed) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid reveal spirits tick duration");
    for (auto it = vanishCooldowns_.begin(); it != vanishCooldowns_.end();) {
        it->second -= seconds;
        if (it->second <= 0)
            it = vanishCooldowns_.erase(it);
        else
            ++it;
    }
    if (!position_)
        return; // Native state does not update alpha before its first motion event.
    if (position_ == previous_)
        stationaryAge_ += seconds;
    else
        stationaryAge_ = 0;
    previous_ = position_;
    const bool moving = stationaryAge_ <= movementSeconds;
    if (brushPlaying_ != moving) {
        brushPlaying_ = moving;
        events_.push_back({RevealEvent::Brush, moving ? brushSound : ""});
    }
    // Native walks backwards when retiring completed pairs. A newly completed
    // target remains in the list until the next update, preserving callback order.
    for (size_t at = pending_.size(); at-- > 0;) {
        const auto index = pending_[at];
        auto& spirit = *targets_[index].spirit;
        if (spirit.alpha == 1) {
            pending_.erase(pending_.begin() + at);
            continue;
        }
        if (moving && hit_ == index) {
            float value = spirit.alpha;
            if (value == 0)
                value += float(firstTouchBoost);
            value += float(revealSpeed * seconds);
            if (value >= 1) {
                if (!revealedSound.empty())
                    events_.push_back({RevealEvent::Sound, revealedSound});
                if (onRevealed)
                    onRevealed(spirit);
            }
            spirit.alpha = std::clamp(value, 0.f, 1.f);
        } else if (spirit.alpha > 0) {
            const auto value = spirit.alpha - float(vanishSpeed * seconds);
            if (value <= 0 && !maleVanish_.empty() && !femaleVanish_.empty() &&
                !vanishCooldowns_.contains(spirit.id)) {
                events_.push_back(
                    {RevealEvent::Sound, females_.contains(spirit.id) ? femaleVanish_ : maleVanish_});
                vanishCooldowns_[spirit.id] = vanishSoundDelay;
            }
            spirit.alpha = std::clamp(value, 0.f, 1.f);
        }
    }
}
std::vector<RevealEvent> RevealSpirits::takeEvents() {
    return std::exchange(events_, {});
}
} // namespace deadtime
