#pragma once
#include "activity_layout.hpp"
#include <optional>
#include <random>

namespace deadtime {
struct CellarEvent {
    enum Kind { Sound, Hint, Finish } kind;
    std::string value;
};
class CellarDoors {
    friend class ActivityCheckpoints;

  public:
    CellarDoors(const Assets& assets, const std::string& path, uint32_t seed);
    bool applyAsh();
    bool skip();
    bool select(size_t index);
    void idleClick();
    void wrongItem();
    void tick(double seconds);
    void enter();
    bool busy() const {
        return phase_ != Phase::Idle;
    }
    bool revealed() const {
        return revealed_;
    }
    bool complete() const {
        return complete_;
    }
    bool holdingDialogs() const {
        return resultCorrect_ && (phase_ == Phase::Watch || phase_ == Phase::Fade);
    }
    const auto& selected() const {
        return selected_;
    }
    const auto& answer() const {
        return answer_;
    }
    const auto& syllables() const {
        return syllables_;
    }
    const auto& objects() const {
        return objects_;
    }
    const ActivityObject& object(const std::string& name) const;
    const ActivityObject& syllable(size_t index) const;
    std::vector<CellarEvent> takeEvents();
    std::string onFinish;
    bool rewarded = false;

  private:
    enum class Phase { Idle, Reveal, Watch, Fade };
    Phase phase_ = Phase::Idle;
    std::vector<ActivityObject> objects_;
    std::vector<std::string> syllables_, sounds_, wrongSounds_;
    std::vector<size_t> answer_, selected_, fading_;
    std::vector<double> highlightAge_;
    std::vector<float> fadeStart_;
    std::vector<CellarEvent> events_;
    std::mt19937 random_;
    std::optional<double> resultDelay_;
    double age_ = 0, watchSeconds_ = 0, wrongDelay_ = 0, correctSoundSeconds_ = 0;
    bool revealed_ = false, complete_ = false, resultCorrect_ = false;
    ActivityObject& mutableObject(const std::string& name);
    std::string name(size_t index, bool highlight = false) const;
    void hint(const std::string& key);
    void watch();
    void fade();
    void advance(double seconds);
    void updateVisuals();
};
} // namespace deadtime
