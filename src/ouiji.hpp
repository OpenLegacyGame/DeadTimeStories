#pragma once
#include "animation_set.hpp"
#include <memory>

namespace deadtime {
class OuijiBoard {
    friend class ActivityCheckpoints;

  public:
    explicit OuijiBoard(const Assets& assets);
    void enter();
    void leave();
    bool select(const std::string& id);
    bool skip();
    void tick(double seconds);
    void setPlanchetteSize(float width, float height);
    ActivityObject& planchette();
    const auto& objects() const {
        return objects_;
    }
    const auto& entered() const {
        return entered_;
    }
    bool moving() const {
        return moving_;
    }
    bool solved() const {
        return solved_;
    }
    bool complete() const {
        return claimed_;
    }
    bool busy() const {
        return moving_ || opening_;
    }
    bool opened() const {
        return solved() && !opening_;
    }
    const ActivityObject& object(const std::string& id) const;
    bool exitRequested = false, rewarded = false;
    std::string onFinish, feedback;

  private:
    ActivityObject& mutableObject(const std::string& id);
    void updateVisuals();
    void tickOpening(double seconds);
    std::array<std::unique_ptr<AnimationSet>, 2> fades_;
    bool opening_ = false, claimed_ = false, solved_ = false;
    unsigned fadePhase_ = 0;
    double fadeAge_ = 0;
    std::vector<ActivityObject> objects_;
    std::string alphabet_, phrase_, entered_, selected_;
    double speed_ = 400, age_ = 0, duration_ = 0;
    float startX_ = 0, startY_ = 0, startAngle_ = 0, targetX_ = 0, targetY_ = 0, targetAngle_ = 0;
    bool moving_ = false;
};
} // namespace deadtime
