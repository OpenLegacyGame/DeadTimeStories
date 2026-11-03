#pragma once
#include "animation_set.hpp"
#include "particles.hpp"
#include <map>
#include <optional>

namespace deadtime {
struct LeechState {
    enum Phase { Waiting, Animating, Dormant } phase = Waiting;
    double delay = 0, age = 0;
    bool caught = false, removed = false;
};
struct LeechesEvent {
    enum Kind { Sound, Hint, Finish } kind;
    std::string value;
};
class Leeches {
    friend class ActivityCheckpoints;

  public:
    Leeches(const Assets& assets, const std::string& directory, uint32_t seed);
    void enter();
    void leave();
    void tick(double seconds);
    bool catchLeech(size_t index, const std::string& inventory);
    bool throwIntoSwamp();
    bool skip();
    void inspect(const std::string& target);
    void refuseCancel();
    void requireSwamp();
    bool holding() const {
        return held_.has_value();
    }
    const auto& held() const {
        return held_;
    }
    const auto& heldOrange() const {
        return orange_;
    }
    bool solved() const {
        return solved_;
    }
    bool complete() const {
        return complete_;
    }
    size_t removedCount() const;
    const auto& states() const {
        return states_;
    }
    const auto& objects() const {
        return objects_;
    }
    const auto& animations() const {
        return animations_;
    }
    const ActivityObject& leech(size_t index) const;
    const ActivityObject& object(const std::string& id) const;
    std::vector<LeechesEvent> takeEvents();
    static bool isOrange(const std::string& item);
    std::string onEnter, onExit, onFinish;
    bool rewarded = false;

  private:
    std::vector<LeechState> states_;
    std::vector<ActivityObject> objects_, initialLeeches_;
    std::vector<AnimationSet> animations_;
    std::map<std::string, std::pair<float, float>> textureSizes_;
    std::vector<LeechesEvent> events_;
    ParticleRandom random_;
    unsigned minMs_ = 0, maxMs_ = 0;
    std::optional<size_t> held_;
    std::string orange_;
    bool solved_ = false, complete_ = false;
    double delay();
    ActivityObject& mutableObject(const std::string& id);
    void updateVisuals();
    void hint(const std::string& id, bool reactive = true);
    void sound(const std::string& path);
};
} // namespace deadtime
