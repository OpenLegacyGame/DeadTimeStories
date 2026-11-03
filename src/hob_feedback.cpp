#include "hob_feedback.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace deadtime {
HobFeedback::HobFeedback(const Assets& assets) {
    if (assets.contains("hobs/found.animset"))
        animation_.emplace(assets, "hobs/found.animset");
}
void HobFeedback::reset() {
    found_.reset();
    misses_.clear();
    toolHints_.clear();
    clock_ = age_ = 0;
    penalty_ = false;
    completion_ = Completion::None;
}
void HobFeedback::waitForReward() {
    if (transient())
        throw std::logic_error("HOB completion transition is already active");
    completion_ = Completion::Reward;
    age_ = 0;
    misses_.clear();
}
bool HobFeedback::confirmReward() {
    if (!waitingForReward())
        return false;
    completion_ = Completion::FadeOut;
    age_ = 0;
    return true;
}
float HobFeedback::fadeAlpha() const {
    if (fadingOut())
        return float(std::clamp(age_, 0., 1.));
    if (returning())
        return float(std::clamp(1 - age_, 0., 1.));
    return 0;
}
void HobFeedback::found(Piece piece) {
    if (transient())
        throw std::logic_error("HOB feedback is already active");
    found_ = piece;
    age_ = 0;
    misses_.clear();
}
void HobFeedback::toolHint(Piece piece) {
    if (toolHints_.insert(piece).second)
        misses_.clear();
}
bool HobFeedback::miss(float x, float y) {
    if (transient())
        return false;
    if (!std::isfinite(x) || !std::isfinite(y))
        throw std::runtime_error("Invalid HOB miss position");
    if (misses_.size() == 5)
        misses_.pop_front();
    misses_.push_back(clock_);
    // Native 51EEE0 keeps the last five misses, with a strict two-second span.
    if (misses_.size() != 5 || clock_ - misses_.front() >= 2)
        return false;
    misses_.clear();
    penalty_ = true;
    penaltyX_ = x;
    penaltyY_ = y;
    age_ = 0;
    return true;
}
std::optional<HobFeedback::Piece> HobFeedback::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid HOB feedback time");
    clock_ += seconds;
    if (!busy())
        return {};
    age_ += seconds;
    if (fadingOut()) {
        if (age_ >= 1) {
            completion_ = Completion::Return;
            age_ = 0;
        }
        return {};
    }
    if (returning()) {
        if (age_ >= 1) {
            completion_ = Completion::None;
            age_ = 0;
        }
        return {};
    }
    if (penalty_) {
        if (age_ > 2) {
            penalty_ = false;
            age_ = 0;
        }
        return {};
    }
    if (age_ < (animation_ ? animation_->duration() : .3))
        return {};
    const auto piece = found_;
    found_.reset();
    age_ = 0;
    return piece;
}
void HobFeedback::animate(ActivityObject& object) const {
    if (!found_)
        return;
    if (animation_)
        animation_->apply(object, age_);
    else {
        object.scale = 1 + float(age_ / .3);
        object.alpha = 1 - float(age_ / .3);
    }
}
std::pair<int, int> HobFeedback::penaltyPosition() const {
    // Native 522F30 uses float intermediates and truncates the final coordinates.
    const float remaining = float(2 - age_), radius = (2 - remaining) * .5f * 100;
    const float angle = remaining * .5f * 12.566370964050293f;
    return {int(std::cos(angle) * radius + penaltyX_), int(std::sin(angle) * radius + penaltyY_)};
}
} // namespace deadtime
