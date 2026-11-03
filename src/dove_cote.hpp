#pragma once
#include "animation_set.hpp"
#include <memory>
#include <optional>
#include <random>

namespace deadtime {
struct DoveEvent {
    enum Kind { Sound, Hint, Help, ReturnRope } kind;
    std::string value;
};
class DoveCote {
    friend class ActivityCheckpoints;

  public:
    DoveCote(const Assets& assets, const std::string& path, uint32_t seed);
    bool feed();
    bool skip();
    bool togglePipe(size_t pipe);
    bool clickDoor(size_t door);
    bool connect(size_t roll);
    bool undo();
    void moveRope(float x, float y);
    void tick(double seconds);
    void leave();
    void enter();
    bool busy() const {
        return phase_ != Phase::Idle;
    }
    bool inside() const {
        return inside_;
    }
    bool complete() const {
        return complete_;
    }
    bool secured() const;
    bool correct() const;
    bool closed(size_t door) const {
        return closed_.at(door);
    }
    bool locked(size_t pipe) const;
    bool pulling() const {
        return !path_.empty() && !secured();
    }
    double length() const;
    double maxLength() const {
        return maxLength_;
    }
    const auto& reference() const {
        return reference_;
    }
    const auto& path() const {
        return path_;
    }
    const auto& locks() const {
        return locks_;
    }
    const auto& objects() const {
        return objects_;
    }
    const ActivityObject& object(const std::string& name) const;
    const ActivityObject& roll(size_t index) const;
    std::vector<DoveEvent> takeEvents();
    std::string onEnter, onExit, onFinish;
    bool rewarded = false;
    double arrivalDelay = 0;

  private:
    enum class Phase { Idle, ArrivalWait, Incoming, Watch, DoorOpening, Outgoing };
    Phase phase_ = Phase::Idle;
    std::string base_;
    std::vector<ActivityObject> objects_;
    std::vector<size_t> reference_, path_;
    std::vector<std::pair<size_t, size_t>> locks_;
    std::vector<bool> closed_;
    std::vector<std::unique_ptr<AnimationSet>> incoming_, outgoing_;
    std::vector<DoveEvent> events_;
    std::mt19937 random_;
    std::optional<size_t> excludedDoor_;
    size_t count_ = 0, flightDoor_ = 0, firstRope_ = 0;
    double age_ = 0, maxLength_ = 0;
    bool inside_ = false, fed_ = false, complete_ = false, helpShown_ = false, openingWasClosed_ = false;
    float mouseX_ = 0, mouseY_ = 0;
    float ropeHeight_ = 0;
    ActivityObject& mutableObject(const std::string& name);
    double pathLength(const std::vector<size_t>& path) const;
    std::pair<float, float> center(size_t index) const;
    void updateVisuals();
    void watch(std::optional<size_t> excluded = {});
    void message(const std::string& key);
};
} // namespace deadtime
