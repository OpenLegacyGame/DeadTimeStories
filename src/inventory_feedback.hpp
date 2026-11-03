#pragma once
#include "runtime.hpp"

namespace deadtime {
// HUD presentation only: the original engine grants an item before its flight.
class InventoryFeedback {
  public:
    struct Flight {
        std::string item;
        float fromX = 0, fromY = 0, x = 0, y = 0;
        double elapsed = 0, duration = 0;
        double labelElapsed = 0;
        bool arrived = false, trail = false, feedbackFinished = false;
        Object effect;
    };
    explicit InventoryFeedback(Assets& assets);
    bool start(const std::string& item, float x, float y);
    std::vector<std::string> tick(double seconds);
    std::vector<std::string> takeCompletions();
    void finishLabel(const std::string& item);
    bool pending() const;
    static float labelAlpha(double seconds);
    bool flying(const std::string& item) const;
    bool busy() const;
    void clear();
    const auto& flights() const {
        return flights_;
    }
    std::vector<ParticleBatch> particles(const Flight& flight);
    float targetX() const {
        return targetX_;
    }
    float targetY() const {
        return targetY_;
    }

  private:
    SceneParticles particles_;
    std::vector<std::unique_ptr<Flight>> flights_;
    std::vector<std::string> completed_;
    float targetX_ = 0, targetY_ = 0;
};
} // namespace deadtime
