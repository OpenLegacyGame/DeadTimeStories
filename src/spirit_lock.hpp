#pragma once
#include "activity_layout.hpp"
#include <map>
#include <memory>

namespace deadtime {
struct Object;
struct SpiritLockEvent {
    enum Kind { Sound, FinalSound, Hint, PopupHint, Finish } kind;
    std::string value;
};
class SpiritLock {
    friend class ActivityCheckpoints;

  public:
    SpiritLock(const Assets& assets, const std::string& directory);
    ~SpiritLock();
    void enter(bool allPiecesFound);
    bool place(size_t slot, const std::string& item);
    bool skip();
    bool press(float x, float y);
    void move(float x, float y);
    void release();
    void idleClick();
    void wrongItem();
    void tick(double seconds);
    void useAudioCallback(bool enabled) {
        audioCallback_ = enabled;
    }
    void finishPronunciation();
    bool assembled() const;
    bool placed(size_t index) const {
        return placed_.at(index);
    }
    bool turning() const {
        return phase_ == Phase::Turning;
    }
    bool busy() const {
        return phase_ != Phase::Idle;
    }
    bool complete() const {
        return complete_;
    }
    float angle() const {
        return angle_;
    }
    size_t keysPassed() const {
        return key_;
    }
    const auto& pieces() const {
        return pieces_;
    }
    const auto& objects() const {
        return objects_;
    }
    auto& effects() {
        return effects_;
    }
    const Object* effect(const std::string& id) const;
    const ActivityObject& object(const std::string& id) const;
    const ActivityObject& piece(size_t index) const;
    std::vector<SpiritLockEvent> takeEvents();
    std::string onEnter, onExit, onFinish;
    bool rewarded = false;

  private:
    enum class Phase { Idle, Turning, Swinging } phase_ = Phase::Idle;
    struct Key {
        float angle;
        std::string sound;
        double duration;
    };
    std::vector<ActivityObject> objects_;
    std::map<std::string, std::unique_ptr<Object>> effects_;
    std::vector<std::string> pieces_;
    std::vector<bool> placed_;
    std::vector<Key> sequence_;
    std::vector<SpiritLockEvent> events_;
    float velocity_ = 0, swingVelocity_ = 0, minRadius_ = 0, maxRadius_ = 0, minAngle_ = 0, maxAngle_ = 0;
    float angle_ = 0, cursorX_ = 0, cursorY_ = 0, target_ = 0, applied_ = 0, lastKeyAngle_ = 0;
    double finalSeconds_ = 0, putDuration_ = 0;
    size_t key_ = 0;
    bool moved_ = false, entered_ = false, complete_ = false, allPiecesFound_ = false, audioCallback_ = false;
    ActivityObject& mutableObject(const std::string& id);
    void updateVisuals();
    void hint(const std::string& name, bool reactive = true);
    void sound(const std::string& path);
};
} // namespace deadtime
