#pragma once
#include "runtime.hpp"

namespace deadtime {
class InventoryPanel {
  public:
    enum class Phase { Closed, Raising, Raised, FadeIn, Open, Lowering };
    enum class Sound { Open, Close };
    explicit InventoryPanel(Assets& assets);
    void request(bool open, bool animated = true);
    // Replacing/hiding the HUD cancels presentation without a queued replay.
    void reset(bool open = false);
    void tick(double seconds);
    bool open() const {
        return phase_ == Phase::Open;
    }
    bool busy() const {
        return phase_ != Phase::Open && phase_ != Phase::Closed;
    }
    Phase phase() const {
        return phase_;
    }
    float overlayAlpha() const;
    const Object* model() const;
    std::vector<ParticleBatch> particles();
    std::vector<Sound> takeSounds();
    uint64_t overlayGeneration() const {
        return overlayGeneration_;
    }
    double raiseDuration() const {
        return raiseDuration_;
    }
    double lowerDuration() const {
        return lowerDuration_;
    }

  private:
    SceneParticles particles_;
    Object raising_, lowering_;
    double raiseDuration_ = 0, lowerDuration_ = 0, fadeSeconds_ = 0;
    Phase phase_ = Phase::Closed;
    std::optional<std::pair<bool, bool>> deferred_;
    std::vector<Sound> sounds_;
    uint64_t overlayGeneration_ = 0;
};
} // namespace deadtime
