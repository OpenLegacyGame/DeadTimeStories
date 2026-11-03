#include "inventory_panel.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace deadtime {
InventoryPanel::InventoryPanel(Assets& assets) : particles_(assets, 1) {
    raising_.sceneResources.push_back(
        {"particles_ready/graves/graves_up.dae", false, {{"particles_ready/graves/gui_graves.psysl", 0}}});
    lowering_.sceneResources.push_back({"particles_ready/graves/graves_dn.dae", false, {}});
    raiseDuration_ = ColladaScene(assets, raising_.sceneResources.front().path).duration();
    lowerDuration_ = ColladaScene(assets, lowering_.sceneResources.front().path).duration();
    if (raiseDuration_ <= 0 || lowerDuration_ <= 0)
        throw std::runtime_error("Inventory grave models have no animation");
    raising_.particleGenerating = false;
}
void InventoryPanel::request(bool wantOpen, bool animated) {
    // Native logical-open flag changes only after raising/fade-in, and at the
    // start of lowering. Test it before the busy/defer branch.
    if (wantOpen == open())
        return;
    if (busy()) {
        if (!deferred_)
            deferred_ = std::pair{wantOpen, animated};
        return;
    }
    if (!animated) {
        reset(wantOpen);
        return;
    }
    fadeSeconds_ = 0;
    if (wantOpen) {
        phase_ = Phase::Raising;
        raising_.treeTime = 0;
        raising_.treePlaying = true;
        raising_.particleGenerating = true;
        particles_.reset(raising_);
        particles_.tick(raising_, 0);
        sounds_.push_back(Sound::Open);
    } else {
        phase_ = Phase::Lowering;
        lowering_.treeTime = 0;
        lowering_.treePlaying = true;
        ++overlayGeneration_;
        sounds_.push_back(Sound::Close);
    }
}
void InventoryPanel::reset(bool wantOpen) {
    phase_ = wantOpen ? Phase::Open : Phase::Closed;
    deferred_.reset();
    raising_.treeTime = lowering_.treeTime = fadeSeconds_ = 0;
    raising_.particleGenerating = false;
    raising_.treePlaying = lowering_.treePlaying = false;
    particles_.reset(raising_);
    sounds_.clear();
}
void InventoryPanel::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0 || seconds > 3600)
        throw std::runtime_error("Invalid inventory panel tick");
    // Native 53C020 polls the previous frame before updating scenes. One update
    // stops dirt births; the following starts the item/web alpha animation.
    if (phase_ == Phase::Raising && raising_.treeTime >= raiseDuration_) {
        phase_ = Phase::Raised;
        raising_.treePlaying = false;
        raising_.particleGenerating = false;
        particles_.setGenerating(raising_, false);
    } else if (phase_ == Phase::Raised) {
        phase_ = Phase::FadeIn;
        fadeSeconds_ = 0;
        ++overlayGeneration_;
    } else if (phase_ == Phase::Lowering && lowering_.treeTime >= lowerDuration_) {
        phase_ = Phase::Closed;
        lowering_.treeTime = 0;
        lowering_.treePlaying = false;
    }
    if (!busy() && deferred_) {
        const auto requestData = std::exchange(deferred_, {}).value();
        request(requestData.first, requestData.second);
    }
    if (phase_ == Phase::Raising)
        raising_.treeTime += seconds;
    if (phase_ == Phase::Lowering) {
        lowering_.treeTime += seconds;
        fadeSeconds_ += seconds;
    }
    if (phase_ == Phase::FadeIn) {
        fadeSeconds_ += seconds;
        if (fadeSeconds_ >= .15) {
            phase_ = Phase::Open;
            raising_.treeTime = 0;
            raising_.treePlaying = false;
        }
    }
    if (!busy() && deferred_ && deferred_->first == open())
        deferred_.reset();
    particles_.tick(raising_, seconds);
}
float InventoryPanel::overlayAlpha() const {
    if (phase_ == Phase::FadeIn)
        return float(std::clamp(fadeSeconds_ / .15, 0., 1.));
    if (phase_ == Phase::Lowering)
        return float(std::clamp(1. - fadeSeconds_ / .15, 0., 1.));
    return 0;
}
const Object* InventoryPanel::model() const {
    if (phase_ == Phase::Raising || phase_ == Phase::Raised || phase_ == Phase::FadeIn)
        return &raising_;
    if (phase_ == Phase::Lowering)
        return &lowering_;
    return nullptr;
}
std::vector<ParticleBatch> InventoryPanel::particles() {
    return phase_ == Phase::Raising || phase_ == Phase::Raised || phase_ == Phase::FadeIn
               ? particles_.draw(raising_)
               : std::vector<ParticleBatch>{};
}
std::vector<InventoryPanel::Sound> InventoryPanel::takeSounds() {
    return std::exchange(sounds_, {});
}
} // namespace deadtime
