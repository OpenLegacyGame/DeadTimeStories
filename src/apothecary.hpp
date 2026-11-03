#pragma once
#include "activity_layout.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <optional>

namespace deadtime {
struct Object;
struct ApothecaryItem {
    std::string id, tooltip, inventory;
    bool reward = false;
};
struct ApothecaryEvent {
    enum Kind { Sound, Hint, PopupHint, Solve, Award, Finish } kind;
    std::string value;
};
class Apothecary {
    friend class ActivityCheckpoints;

  public:
    Apothecary(const Assets& assets, const std::string& directory, uint32_t seed);
    ~Apothecary();
    void enter();
    void leave();
    bool place(size_t slot, const std::string& inventory);
    bool select(size_t slot);
    bool skip();
    void cancel();
    void idleClick();
    void toggleSheet();
    void tick(double seconds);
    bool busy() const {
        return phase_ != Phase::Idle;
    }
    bool solved() const {
        return solved_;
    }
    bool complete() const {
        return complete_;
    }
    bool sheetOpen() const {
        return sheetOpen_;
    }
    bool locked(size_t slot) const;
    size_t correctCount() const;
    size_t missingCount() const;
    std::optional<size_t> selected() const {
        return selected_;
    }
    const auto& slots() const {
        return slots_;
    }
    const auto& items() const {
        return items_;
    }
    const auto& objects() const {
        return objects_;
    }
    const auto& riddleFont() const {
        return riddleFont_;
    }
    unsigned riddleColor() const {
        return riddleColor_;
    }
    auto& effects() {
        return effects_;
    }
    const Object* effect(const std::string& id) const;
    const ActivityObject& object(const std::string& id) const;
    const ActivityObject& item(size_t index) const;
    const ActivityObject& shelf(size_t index) const;
    std::vector<ApothecaryEvent> takeEvents();
    std::string onEnter, onExit, onSolve, onFinish;

  private:
    enum class Phase { Idle, FadeOut, FadeIn } phase_ = Phase::Idle;
    std::string base_, riddleFont_;
    unsigned riddleColor_ = 0xFF000000;
    std::vector<ApothecaryItem> items_;
    std::vector<std::optional<size_t>> slots_;
    std::vector<ActivityObject> objects_;
    std::vector<std::pair<float, float>> normalSizes_, lockedSizes_;
    std::map<std::string, std::unique_ptr<Object>> effects_;
    std::vector<ApothecaryEvent> events_;
    std::optional<size_t> selected_;
    size_t first_ = 0, second_ = 0;
    double swapSeconds_ = 0, age_ = 0;
    bool solved_ = false, complete_ = false, sheetOpen_ = false, swapCorrect_ = false;
    ActivityObject& mutableObject(const std::string& id);
    void updateVisuals();
    void checkSolved();
    void hint(const std::string& id, bool reactive = true);
    void sound(const std::string& id);
};
} // namespace deadtime
