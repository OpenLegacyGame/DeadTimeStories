#pragma once
#include "animation_set.hpp"
#include <optional>
#include <random>
#include <set>

namespace deadtime {
class HexBones {
    friend class ActivityCheckpoints;

  public:
    HexBones(const Assets& assets, const std::string& path, uint32_t seed);
    bool throwBones();
    bool skip();
    bool skippable() const;
    bool select(size_t slot);
    void tick(double seconds);
    void leave();
    bool thrown() const {
        return thrown_;
    }
    bool busy() const {
        return phase_ != Phase::Idle;
    }
    bool complete() const {
        return complete_;
    }
    bool locked(size_t slot) const {
        return current_.at(slot) == phrase_.at(slot);
    }
    size_t correctCount() const;
    const std::string& phrase() const {
        return phrase_;
    }
    const std::string& current() const {
        return current_;
    }
    std::optional<size_t> selected() const {
        return selected_;
    }
    const ActivityObject& bone(size_t slot) const {
        return objects_.at(firstBone_ + slot);
    }
    const ActivityObject& object(const std::string& name) const;
    const auto& objects() const {
        return objects_;
    }
    std::vector<std::string> takeSounds();
    std::string onEnter, onFinish, feedback;
    bool rewarded = false;
    double swapSeconds = 0, deviation = 0;
    unsigned discretization = 0, randomLocked = 0;
    const auto& fixed() const {
        return fixed_;
    }

  private:
    enum class Phase { Idle, FadeIn, FadeOut, Swap };
    Phase phase_ = Phase::Idle;
    std::vector<ActivityObject> objects_, positions_;
    size_t firstBone_ = 0, swapFirst_ = 0, swapSecond_ = 0;
    std::string base_, alphabet_, phrase_, current_;
    std::set<size_t> fixed_;
    std::map<std::string, std::pair<float, float>> textureSizes_;
    std::vector<float> angles_, depths_;
    std::optional<size_t> selected_;
    bool thrown_ = false, bonesVisible_ = false, complete_ = false;
    double age_ = 0;
    AnimationSet fadeIn_, fadeOut_;
    std::vector<std::string> sounds_;
    void shuffle(std::mt19937& random);
    void updateVisuals();
    void updateSwap();
    ActivityObject& mutableObject(const std::string& name);
};
} // namespace deadtime
