#include "inventory_feedback.hpp"
#include "gui.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace deadtime {
InventoryFeedback::InventoryFeedback(Assets& assets) : particles_(assets, 1) {
    const auto slots = loadGui(assets, "hud/common/hud_inventory_objects.gui");
    const auto first =
        std::find_if(slots.begin(), slots.end(), [](const auto& e) { return e.id == "inventory_object_1"; });
    if (first == slots.end())
        throw std::runtime_error("Inventory has no first slot");
    targetX_ = first->x;
    targetY_ = first->y;
}
bool InventoryFeedback::start(const std::string& item, float x, float y) {
    if (item.empty() || !std::isfinite(x) || !std::isfinite(y))
        throw std::runtime_error("Invalid inventory flight");
    if (flying(item))
        return false;
    auto flight = std::make_unique<Flight>();
    flight->item = item;
    flight->fromX = flight->x = x;
    flight->fromY = flight->y = y;
    // Native HUD 53B1A0 + position track 583710: linear, distance / 550 px/s.
    flight->duration = std::hypot(double(targetX_) - x, double(targetY_) - y) / 550.;
    // Native HUD owns five trail instances; additional items still fly.
    flight->trail =
        std::count_if(flights_.begin(), flights_.end(), [](const auto& f) { return f->trail; }) < 5;
    if (flight->trail) {
        flight->effect.particleResources.push_back({"particles_ready/item_found/item_found.psysl", 0});
        flight->effect.x = x + 37.5f;
        flight->effect.y = y + 42;
        particles_.tick(flight->effect, 0);
    }
    flights_.push_back(std::move(flight));
    return true;
}
std::vector<std::string> InventoryFeedback::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid inventory feedback tick");
    std::vector<std::string> arrivals;
    // Small steps preserve world-emitted trail positions on large offline ticks.
    double remaining = seconds;
    do {
        const auto step = std::min(.1, remaining);
        for (auto& f : flights_) {
            if (!f->arrived) {
                const auto advance = std::min(step, std::max(0., f->duration - f->elapsed));
                f->elapsed += advance;
                const auto ratio = f->duration > 0 ? std::min(1., f->elapsed / f->duration) : 1.;
                f->x = f->fromX + float(ratio) * (targetX_ - f->fromX);
                f->y = f->fromY + float(ratio) * (targetY_ - f->fromY);
                f->effect.x = f->x + 37.5f;
                f->effect.y = f->y + 42;
                if (f->trail)
                    particles_.tick(f->effect, advance);
                if (f->elapsed >= f->duration) {
                    f->arrived = true;
                    arrivals.push_back(f->item);
                    if (std::count_if(flights_.begin(), flights_.end(), [](const auto& other) {
                            return other->arrived && !other->feedbackFinished;
                        }) > 8)
                        finishLabel(f->item);
                    f->effect.particleGenerating = false;
                    if (f->trail)
                        particles_.setGenerating(f->effect, false);
                }
                if (f->arrived)
                    f->labelElapsed += step - advance;
                if (f->trail && step > advance)
                    particles_.tick(f->effect, step - advance);
            } else {
                f->labelElapsed += step;
                if (f->trail)
                    particles_.tick(f->effect, step);
            }
            if (f->arrived && f->labelElapsed >= 2.25)
                finishLabel(f->item);
            if (f->arrived && f->trail) {
                const auto batches = particles_.draw(f->effect);
                if (std::none_of(batches.begin(), batches.end(),
                                 [](const auto& batch) { return !batch.sprites.empty(); })) {
                    particles_.reset(f->effect);
                    f->trail = false;
                }
            }
        }
        remaining -= step;
    } while (remaining > 1e-10);
    std::erase_if(flights_, [&](const auto& f) {
        if (!f->arrived || !f->feedbackFinished)
            return false;
        if (f->trail)
            return false;
        particles_.reset(f->effect);
        return true;
    });
    return arrivals;
}
std::vector<std::string> InventoryFeedback::takeCompletions() {
    return std::exchange(completed_, {});
}
void InventoryFeedback::finishLabel(const std::string& item) {
    for (auto& f : flights_)
        if (f->item == item && f->arrived && !f->feedbackFinished) {
            f->feedbackFinished = true;
            completed_.push_back(item);
        }
}
float InventoryFeedback::labelAlpha(double seconds) {
    // Native tooltip_fade_out: 0.75s hold, then a 1.5s linear fade.
    return float(std::clamp(1. - (seconds - .75) / 1.5, 0., 1.));
}
bool InventoryFeedback::pending() const {
    return std::any_of(flights_.begin(), flights_.end(), [](const auto& f) { return !f->feedbackFinished; });
}
bool InventoryFeedback::flying(const std::string& item) const {
    return std::any_of(flights_.begin(), flights_.end(),
                       [&](const auto& f) { return !f->arrived && f->item == item; });
}
bool InventoryFeedback::busy() const {
    return std::any_of(flights_.begin(), flights_.end(), [](const auto& f) { return !f->arrived; });
}
void InventoryFeedback::clear() {
    for (const auto& f : flights_)
        particles_.reset(f->effect);
    flights_.clear();
    completed_.clear();
}
std::vector<ParticleBatch> InventoryFeedback::particles(const Flight& flight) {
    return particles_.draw(flight.effect);
}
} // namespace deadtime
