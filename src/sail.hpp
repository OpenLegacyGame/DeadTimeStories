#pragma once
#include "activity_layout.hpp"
#include <cstdint>
#include <map>
#include <optional>

namespace deadtime {
struct SailPiece {
    std::string id, texture, fixedTexture;
    float targetX = 0, targetY = 0;
    unsigned turns = 0;
    bool placed = false, fixedView = false;
};
struct SailEvent {
    enum Kind { Sound, Hint, ClearHint, Finish } kind;
    std::string value;
};
class Sail {
    friend class ActivityCheckpoints;

  public:
    Sail(const Assets& assets, const std::string& directory, uint32_t seed);
    void enter();
    void leave();
    bool place(const std::string& inventory, float x, float y);
    bool press(size_t piece, float x, float y);
    void move(float x, float y);
    bool release();
    void cancelGrip();
    bool sew(const std::string& inventory);
    bool skip();
    void tick(double seconds);
    bool correct(size_t piece) const;
    bool ready() const;
    bool complete() const {
        return complete_;
    }
    bool busy() const {
        return phase_ == Phase::Sewing;
    }
    bool gripping() const {
        return phase_ == Phase::Pressed || phase_ == Phase::Dragging;
    }
    bool dragging() const {
        return phase_ == Phase::Dragging;
    }
    float exitY() const {
        return exitY_;
    }
    float threshold() const {
        return threshold_;
    }
    const auto& pieces() const {
        return pieces_;
    }
    const auto& inventoryPieces() const {
        return inventory_;
    }
    const auto& sewingItem() const {
        return sewingItem_;
    }
    const auto& objects() const {
        return objects_;
    }
    const ActivityObject& piece(size_t index) const;
    const ActivityObject& object(const std::string& id) const;
    std::vector<SailEvent> takeEvents();
    std::string onEnter, onExit, onFinish;
    bool rewarded = false;

  private:
    enum class Phase { Idle, Pressed, Dragging, Sewing } phase_ = Phase::Idle;
    std::vector<SailPiece> pieces_;
    std::vector<ActivityObject> objects_;
    std::vector<std::pair<float, float>> normalSizes_, fixedSizes_;
    std::map<std::string, std::vector<size_t>> inventory_;
    std::vector<SailEvent> events_;
    std::string sewingItem_;
    float threshold_ = 0, exitY_ = 0, pressX_ = 0, pressY_ = 0, offsetX_ = 0, offsetY_ = 0;
    double sewAge_ = 0;
    size_t held_ = 0;
    bool readyAnnounced_ = false, complete_ = false;
    ActivityObject& mutableObject(const std::string& id);
    ActivityObject& mutablePiece(size_t index);
    void updateVisuals();
    void normalizeDepths();
    void snapCorrect();
    void fixView(size_t index, bool feedback);
    void checkReady();
    void hint(const std::string& id);
};
} // namespace deadtime
