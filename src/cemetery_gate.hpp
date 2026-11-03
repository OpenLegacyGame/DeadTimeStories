#pragma once
#include "animation_set.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <optional>

namespace deadtime {
struct Object;
struct GateEvent {
    enum Kind { Sound, Hint, Finish } kind;
    std::string value;
};
class CemeteryGate {
    friend class ActivityCheckpoints;

  public:
    CemeteryGate(const Assets& assets, const std::string& directory, uint32_t seed);
    ~CemeteryGate();
    void enter();
    void leave();
    bool select(size_t block);
    bool skip();
    void idleClick();
    void tick(double seconds);
    bool entered() const {
        return entered_;
    }
    bool complete() const {
        return complete_;
    }
    bool busy() const {
        return rotating_.has_value();
    }
    double rotationRemaining() const {
        return busy() ? rotation_.duration() - age_ : 0;
    }
    bool locked(size_t index) const;
    char letter(size_t index) const;
    bool letterVisible(size_t index) const {
        return rotating_ != index;
    }
    const auto& blocks() const {
        return blocks_;
    }
    const auto& current() const {
        return current_;
    }
    const auto& answer() const {
        return answer_;
    }
    const auto& objects() const {
        return objects_;
    }
    const auto& font() const {
        return font_;
    }
    unsigned color() const {
        return color_;
    }
    float letterX() const {
        return letterX_;
    }
    float letterY() const {
        return letterY_;
    }
    const ActivityObject& block(size_t index) const;
    std::pair<float, float> letterSize(size_t index) const {
        return sizes_.at(index);
    }
    const ActivityObject& object(const std::string& id) const;
    Object& effect();
    std::vector<GateEvent> takeEvents();
    std::string onEnter, onExit, onFinish;
    bool rewarded = false;

  private:
    std::string base_, answer_, font_;
    std::vector<std::string> blocks_;
    std::vector<size_t> current_;
    std::vector<ActivityObject> objects_;
    std::vector<std::pair<float, float>> sizes_;
    std::map<std::string, std::pair<float, float>> textureSizes_;
    AnimationSet rotation_;
    std::unique_ptr<Object> effect_;
    std::vector<GateEvent> events_;
    std::optional<size_t> rotating_;
    double age_ = 0;
    unsigned color_ = 0xFFFFFFFF;
    float letterX_ = 0, letterY_ = 0;
    bool entered_ = false, complete_ = false;
    void updateVisuals();
};
} // namespace deadtime
