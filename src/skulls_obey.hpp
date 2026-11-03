#pragma once
#include "activity_layout.hpp"
#include <memory>

namespace deadtime {
struct Object;
struct SkullEvent {
    enum Kind { Sound, WheelLoop, Hint, Finish } kind;
    std::string value;
};
class SkullsObey {
    friend class ActivityCheckpoints;

  public:
    enum Phase { Idle, Turning, BeforePhrase, AfterPhrase, LidFade };
    SkullsObey(const Assets& assets, const std::string& directory);
    ~SkullsObey();
    void enter();
    void leave();
    bool select(size_t skull);
    bool takeReward();
    bool skip();
    void inspectWheel();
    void idleClick();
    void tick(double seconds);
    bool entered() const {
        return entered_;
    }
    bool opened() const {
        return opened_;
    }
    bool complete() const {
        return complete_;
    }
    bool busy() const {
        return phase_ != Idle;
    }
    Phase phase() const {
        return phase_;
    }
    size_t selected() const {
        return selected_;
    }
    double angle() const {
        return angle_;
    }
    const auto& sequence() const {
        return sequence_;
    }
    const auto& history() const {
        return history_;
    }
    const auto& objects() const {
        return objects_;
    }
    const ActivityObject& object(const std::string& id) const;
    const ActivityObject& skull(size_t index) const;
    Object& eyes();
    std::vector<SkullEvent> takeEvents();
    std::string onEnter, onExit, onFinish;
    bool rewarded = false;

  private:
    std::string base_;
    std::vector<double> angles_;
    std::vector<std::string> sounds_;
    std::vector<size_t> sequence_, history_;
    std::vector<ActivityObject> objects_, initialSkulls_;
    std::unique_ptr<Object> eyes_;
    std::vector<SkullEvent> events_;
    size_t selected_ = 0, target_ = 0;
    Phase phase_ = Idle;
    double angle_ = 0, targetAngle_ = 0, velocity_ = 0, remaining_ = 0;
    double speed_ = 0, before_ = 0, after_ = 0, fade_ = 0;
    bool entered_ = false, opened_ = false, complete_ = false;
    ActivityObject& mutableObject(const std::string& id);
    void arrived();
    void beginPhrase();
    void open();
    void updateVisuals();
    void hint(const std::string& id);
};
} // namespace deadtime
