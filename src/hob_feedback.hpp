#pragma once
#include "animation_set.hpp"
#include <deque>
#include <optional>
#include <set>

namespace deadtime {
class HobFeedback {
  public:
    using Piece = std::pair<size_t, size_t>;
    explicit HobFeedback(const Assets& assets);
    void reset();
    bool busy() const {
        return found_.has_value() || penalty_ || fadingOut() || returning();
    }
    bool transient() const {
        return busy() || waitingForReward();
    }
    bool waitingForReward() const {
        return completion_ == Completion::Reward;
    }
    bool fadingOut() const {
        return completion_ == Completion::FadeOut;
    }
    bool returning() const {
        return completion_ == Completion::Return;
    }
    void waitForReward();
    bool confirmReward();
    float fadeAlpha() const;
    bool penalized() const {
        return penalty_;
    }
    const auto& found() const {
        return found_;
    }
    double foundSeconds() const {
        return age_;
    }
    void found(Piece piece);
    void applied() {
        misses_.clear();
    }
    void toolHint(Piece piece);
    bool miss(float x, float y);
    std::optional<Piece> tick(double seconds);
    void animate(ActivityObject& object) const;
    std::pair<int, int> penaltyPosition() const;

  private:
    enum class Completion { None, Reward, FadeOut, Return };
    Completion completion_ = Completion::None;
    std::optional<AnimationSet> animation_;
    std::optional<Piece> found_;
    std::deque<double> misses_;
    std::set<Piece> toolHints_;
    double clock_ = 0, age_ = 0;
    float penaltyX_ = 0, penaltyY_ = 0;
    bool penalty_ = false;
};
} // namespace deadtime
