#pragma once
#include "activity_layout.hpp"
#include <optional>

namespace deadtime {
struct WellEvent {
    enum Kind { Sound, Hint, Finish } kind;
    std::string value;
};
struct WellPlaque {
    std::string text;
    float x = 0, y = 0, z = 0, alpha = 1;
    bool correct = false;
};
class BlockedWell {
    friend class ActivityCheckpoints;

  public:
    BlockedWell(const Assets& assets, const std::string& directory);
    void enter();
    bool select(size_t letter);
    bool allNamesKnown() const;
    bool skip();
    void hover(std::optional<size_t> letter);
    void tick(double seconds);
    void addKnownName(const std::string& name);
    std::optional<bool> isNameKnown(const std::string& name) const;
    std::optional<bool> isNameSpelled(const std::string& name) const;
    bool entered() const {
        return entered_;
    }
    bool complete() const {
        return complete_;
    }
    bool busy() const {
        return phase_ != Phase::Idle;
    }
    const auto& names() const {
        return names_;
    }
    const auto& spelled() const {
        return spelled_;
    }
    const auto& word() const {
        return word_;
    }
    const auto& alphabet() const {
        return alphabet_;
    }
    const auto& objects() const {
        return objects_;
    }
    const auto& font() const {
        return font_;
    }
    unsigned color(bool correct) const {
        return correct ? correctColor_ : color_;
    }
    float plaqueWidth() const {
        return plaqueWidth_;
    }
    std::vector<WellPlaque> plaques() const;
    const ActivityObject& letter(size_t index) const;
    const ActivityObject& object(const std::string& id) const;
    std::vector<WellEvent> takeEvents();
    std::string onEnter, onExit, onFinish;
    bool rewarded = false;

  private:
    enum class Phase { Idle, Letter, Invalid, Correct } phase_ = Phase::Idle;
    struct Position {
        float x, y, z;
    };
    std::vector<ActivityObject> objects_;
    std::vector<std::string> names_;
    std::vector<bool> known_;
    std::vector<size_t> spelled_;
    std::vector<Position> positions_;
    std::vector<WellEvent> events_;
    std::string alphabet_, font_, word_, previous_;
    unsigned color_ = 0xFFFFFFFF, correctColor_ = 0xFFFFFFFF;
    float plaqueWidth_ = 0;
    double letterSeconds_ = 0, invalidSeconds_ = 0, crossSeconds_ = 0, progressSeconds_ = 0, age_ = 0;
    bool complete_ = false, entered_ = false;
    std::optional<size_t> hovered_;
    std::optional<size_t> nameIndex(const std::string& name) const;
    void resolveWord();
    void updateVisuals();
};
} // namespace deadtime
