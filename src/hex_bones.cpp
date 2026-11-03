#include "hex_bones.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>
#include <utility>

namespace deadtime {
namespace {
constexpr double pi = 3.14159265358979323846;
std::string glyphPath(const std::string& base, const std::string& state, size_t index) {
    return base + "/bones/" + state + (index < 10 ? "0" : "") + std::to_string(index);
}
char lower(char c) {
    return c >= 'A' && c <= 'Z' ? char(c + ('a' - 'A')) : c;
}
} // namespace
HexBones::HexBones(const Assets& assets, const std::string& path, uint32_t seed)
    : base_(normalize(path)), fadeIn_(assets, base_ + "/fade_in.animset"),
      fadeOut_(assets, base_ + "/fade_out.animset") {
    pugi::xml_document doc;
    const auto bytes = assets.read(base_ + "/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("hex_bones"))
        throw std::runtime_error("Invalid hex bones configuration");
    const auto root = doc.child("hex_bones");
    auto hook = [&](const char* name) {
        const std::string path = root.child(name).attribute("script").value();
        return path.empty() ? std::string{} : resolve(base_, path) + ".lua";
    };
    onEnter = hook("onEnter");
    onFinish = hook("onFinish");
    for (const auto& hook : {onEnter, onFinish})
        if (!hook.empty() && !assets.contains(hook))
            throw std::runtime_error("Missing hex bones script: " + hook);
    alphabet_ = root.child("alphabet").text().as_string();
    std::transform(alphabet_.begin(), alphabet_.end(), alphabet_.begin(), lower);
    if (alphabet_.empty() || std::set<char>(alphabet_.begin(), alphabet_.end()).size() != alphabet_.size())
        throw std::runtime_error("Invalid hex bones alphabet");
    for (char c : std::string(root.child("phrase").text().as_string()))
        if (alphabet_.find(lower(c)) != std::string::npos)
            phrase_ += lower(c);
    for (auto node : root.child("positions").children("bone")) {
        ActivityObject position;
        position.x = node.attribute("x").as_float();
        position.y = node.attribute("y").as_float();
        if (!std::isfinite(position.x) || !std::isfinite(position.y))
            throw std::runtime_error("Invalid hex bone position");
        positions_.push_back(position);
    }
    if (phrase_.empty() || positions_.size() != phrase_.size())
        throw std::runtime_error("Hex bone phrase and positions differ");
    for (auto node : root.child("locked_bones").children("bone")) {
        const auto index = node.attribute("index").as_uint();
        if (index >= phrase_.size())
            throw std::runtime_error("Hex bone fixed index out of range");
        fixed_.insert(index);
    }
    randomLocked = root.child("num_random_locked_bones").attribute("value").as_uint();
    if (fixed_.size() + randomLocked >= phrase_.size() || phrase_.size() - fixed_.size() < 2)
        throw std::runtime_error("Invalid hex bone locked count");
    const auto swap = root.child("swap_bones");
    swapSeconds = swap.attribute("time_seconds").as_double();
    deviation = swap.attribute("deviation").as_double();
    discretization = swap.attribute("discretization").as_uint();
    if (!std::isfinite(swapSeconds) || swapSeconds <= 0 || !std::isfinite(deviation) || deviation < 0 ||
        !discretization || discretization > 10000)
        throw std::runtime_error("Invalid hex bone swap configuration");
    objects_ = loadActivityLayout(assets, base_ + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (const auto* name : {"back", "exit", "sack", "fx_sack_sparkle", "fader"})
        object(name);
    std::vector<ActivityObject> templates;
    for (size_t i = 0; i < alphabet_.size(); ++i)
        for (const auto* state : {"", "highlighted/", "locked/"}) {
            ActivityObject sprite;
            sprite.texture = glyphPath(base_, state, i);
            templates.push_back(sprite);
        }
    measureActivityTextures(assets, templates);
    for (const auto& sprite : templates)
        textureSizes_[sprite.texture] = {sprite.width, sprite.height};
    firstBone_ = objects_.size();
    std::mt19937 random(seed);
    for (size_t i = 0; i < phrase_.size(); ++i) {
        ActivityObject sprite;
        sprite.id = "bone_" + std::to_string(i);
        objects_.push_back(sprite);
        angles_.push_back(-float(random() % 2000) / 100);
        depths_.push_back(-float(i) * 0.01f);
    }
    current_ = phrase_;
    shuffle(random);
    updateVisuals();
}
const ActivityObject& HexBones::object(const std::string& name) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == name; });
    if (found == objects_.end())
        throw std::runtime_error("Missing hex bones object: " + name);
    return *found;
}
ActivityObject& HexBones::mutableObject(const std::string& name) {
    return const_cast<ActivityObject&>(object(name));
}
size_t HexBones::correctCount() const {
    size_t count = 0;
    for (size_t i = 0; i < phrase_.size(); ++i)
        count += locked(i);
    return count;
}
void HexBones::shuffle(std::mt19937& random) {
    std::vector<size_t> movable;
    for (size_t i = 0; i < phrase_.size(); ++i)
        if (!fixed_.contains(i))
            movable.push_back(i);
    auto exchange = [&] {
        const auto first = size_t(random() % movable.size());
        size_t second;
        do {
            second = random() % movable.size();
        } while (second == first);
        std::swap(current_[movable[first]], current_[movable[second]]);
    };
    for (unsigned i = 0; i < 500; ++i)
        exchange();
    for (unsigned i = 0; i < 100000 && correctCount() != fixed_.size() + randomLocked; ++i)
        exchange();
    if (correctCount() != fixed_.size() + randomLocked)
        throw std::runtime_error("Cannot generate configured hex bone locked count");
}
void HexBones::updateVisuals() {
    for (size_t i = 0; i < phrase_.size(); ++i) {
        auto& sprite = objects_[firstBone_ + i];
        const bool swapping = phase_ == Phase::Swap && (i == swapFirst_ || i == swapSecond_);
        const auto state = locked(i) ? "locked/" : selected_ == i || swapping ? "highlighted/" : "";
        sprite.texture = glyphPath(base_, state, alphabet_.find(current_[i]));
        const auto size = textureSizes_.at(sprite.texture);
        sprite.width = size.first;
        sprite.height = size.second;
        sprite.x = positions_[i].x;
        sprite.y = positions_[i].y;
        sprite.z = depths_[i];
        sprite.angle = angles_[i];
        sprite.visible = bonesVisible_;
        sprite.interactive = bonesVisible_ && !locked(i) && !busy();
    }
    mutableObject("sack").visible = mutableObject("sack").interactive = !thrown_;
    mutableObject("fx_sack_sparkle").visible = !thrown_;
}
bool HexBones::throwBones() {
    if (thrown_ || busy())
        return false;
    thrown_ = true;
    selected_.reset();
    phase_ = Phase::FadeIn;
    age_ = 0;
    auto& fader = mutableObject("fader");
    fader.visible = true;
    fadeIn_.apply(fader, 0);
    updateVisuals();
    return true;
}
bool HexBones::select(size_t slot) {
    if (!thrown_ || busy() || complete_ || slot >= phrase_.size() || locked(slot))
        return false;
    if (!selected_) {
        selected_ = slot;
        sounds_.push_back("sound/sfx/sfx_m_click_idle");
        updateVisuals();
        return true;
    }
    if (*selected_ == slot) {
        selected_.reset();
        sounds_.push_back("sound/sfx/sfx_m_click_idle");
        updateVisuals();
        return true;
    }
    swapFirst_ = *selected_;
    swapSecond_ = slot;
    phase_ = Phase::Swap;
    age_ = 0;
    sounds_.push_back("sound/sfx/mg/mg_board_move");
    updateVisuals();
    updateSwap();
    return true;
}
void HexBones::updateSwap() {
    const double t = std::clamp(age_ / swapSeconds, 0., 1.);
    const double q = t * discretization, a = std::floor(q), f = q - a;
    const double wave = (1 - f) * std::sin(pi * a / discretization) +
                        f * std::sin(pi * std::min(a + 1, double(discretization)) / discretization);
    const double dx = positions_[swapSecond_].x - positions_[swapFirst_].x,
                 dy = positions_[swapSecond_].y - positions_[swapFirst_].y;
    const double length = std::hypot(dx, dy);
    for (unsigned second = 0; second < 2; ++second) {
        const auto from = second ? swapSecond_ : swapFirst_, to = second ? swapFirst_ : swapSecond_;
        auto& sprite = objects_[firstBone_ + from];
        const double sign = second ? -1 : 1;
        sprite.x = float((1 - t) * positions_[from].x + t * positions_[to].x +
                         (length ? dy / length : 0) * deviation * wave * sign);
        sprite.y = float((1 - t) * positions_[from].y + t * positions_[to].y -
                         (length ? dx / length : 0) * deviation * wave * sign);
        const double firstAngle = angles_[from] * pi / 360, lastAngle = angles_[to] * pi / 360;
        sprite.angle = float(2 *
                             std::atan2((1 - t) * std::sin(firstAngle) + t * std::sin(lastAngle),
                                        (1 - t) * std::cos(firstAngle) + t * std::cos(lastAngle)) *
                             180 / pi);
        sprite.z = -20;
    }
}
void HexBones::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid hex bones time");
    if (!busy())
        return;
    age_ += seconds;
    auto& fader = mutableObject("fader");
    if (phase_ == Phase::FadeIn) {
        fadeIn_.apply(fader, age_);
        if (age_ >= fadeIn_.duration()) {
            const auto remaining = age_ - fadeIn_.duration();
            age_ = 0;
            phase_ = Phase::FadeOut;
            bonesVisible_ = true;
            updateVisuals();
            fadeOut_.apply(fader, 0);
            sounds_.push_back("sound/sfx/mg/hex_bones/mg_bones_thrown");
            tick(remaining);
        }
    } else if (phase_ == Phase::FadeOut) {
        fadeOut_.apply(fader, age_);
        if (age_ >= fadeOut_.duration()) {
            fader.visible = false;
            phase_ = Phase::Idle;
            feedback = "puzzles/hex_bones/hints/message_scrambled";
            updateVisuals();
        }
    } else {
        updateSwap();
        if (age_ >= swapSeconds) {
            const auto before = correctCount();
            std::swap(current_[swapFirst_], current_[swapSecond_]);
            depths_[swapFirst_] -= 1;
            depths_[swapSecond_] -= 1;
            if (correctCount() > before)
                sounds_.push_back("sound/sfx/mg/mg_high_light");
            selected_.reset();
            phase_ = Phase::Idle;
            complete_ = current_ == phrase_;
            updateVisuals();
        }
    }
}
void HexBones::leave() {
    phase_ = Phase::Idle;
    selected_.reset();
    bonesVisible_ = thrown_;
    auto& fader = mutableObject("fader");
    fader.visible = false;
    fader.alpha = 0;
    updateVisuals();
}
bool HexBones::skip() {
    // Native 4AFAB0/4AFC00 only finishes an already thrown, unsolved board.
    if (!skippable())
        return false;
    phase_ = Phase::Idle;
    age_ = 0;
    selected_.reset();
    feedback.clear();
    sounds_.clear();
    current_ = phrase_;
    complete_ = bonesVisible_ = true;
    mutableObject("fader").visible = false;
    mutableObject("fader").alpha = 0;
    updateVisuals();
    return true;
}
bool HexBones::skippable() const {
    // Native FxThrowBones (4B5E00) sets thrown before its fades; the following
    // Control::Init (4B6610) is what activates the skip HUD. Swaps keep it active.
    return thrown_ && !complete_ && (phase_ == Phase::Idle || phase_ == Phase::Swap);
}
std::vector<std::string> HexBones::takeSounds() {
    return std::exchange(sounds_, {});
}
} // namespace deadtime
