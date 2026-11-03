#pragma once
#include "animation_set.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <random>

namespace deadtime {
struct WindowCleaningEvent {
    enum class Kind { Sound, Hint, DrySponge };
    Kind kind;
    std::string value;
};
class WindowCleaning {
    friend class ActivityCheckpoints;

  public:
    enum class Phase { Idle, Rubbing, FadeClean, FadeDirty, Finished };
    WindowCleaning(const Assets& assets, const std::string& path, uint32_t seed);
    bool placeMixture();
    bool skip();
    bool begin(size_t section, bool wet, int x, int y);
    std::pair<int, int> move(int x, int y);
    void hover(std::optional<size_t> section) {
        hovered_ = section;
    }
    void cancel(bool hint = false);
    void leave();
    void tick(double seconds);
    bool rubbing() const {
        return phase_ == Phase::Rubbing;
    }
    bool busy() const {
        return phase_ == Phase::FadeClean || phase_ == Phase::FadeDirty;
    }
    bool complete() const {
        return phase_ == Phase::Finished;
    }
    bool mixturePlaced() const {
        return mixturePlaced_;
    }
    bool dirty(size_t section) const {
        return dirty_.at(section);
    }
    double remaining() const {
        return successRemaining_;
    }
    size_t section() const {
        return section_;
    }
    Phase phase() const {
        return phase_;
    }
    ActivityObject& object(const std::string& name);
    const ActivityObject& sectionObject(size_t section) const {
        return objects_.at(sections_.at(section));
    }
    const auto& objects() const {
        return objects_;
    }
    std::vector<WindowCleaningEvent> takeEvents();
    std::string onFinish;
    bool rewarded = false;
    double successSeconds = 0, failureSeconds = 0, removeFxSeconds = 0, movementSeconds = 0;
    float speedMinX = 0, speedMaxX = 0, speedMinY = 0, speedMaxY = 0, fxX = 0, fxY = 0;

  private:
    std::vector<ActivityObject> objects_;
    std::array<size_t, 10> sections_{};
    std::array<bool, 10> dirty_{};
    AnimationSet disappear_, reappear_;
    std::mt19937 random_;
    std::vector<WindowCleaningEvent> events_;
    Phase phase_ = Phase::Idle;
    bool mixturePlaced_ = false, wet_ = false, moved_ = false, inside_ = false;
    int x_ = 0, y_ = 0;
    size_t section_ = 0;
    std::optional<size_t> hovered_;
    double successRemaining_ = 0, failureRemaining_ = 0, idleAge_ = 0, phaseAge_ = 0, fxRemaining_ = 0;
    std::optional<double> floraAge_;
    void positionSponge();
    void hideSponge();
    void finishClean();
    void emit(WindowCleaningEvent::Kind kind, const std::string& value = {});
};
} // namespace deadtime
