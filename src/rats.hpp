#pragma once
#include "activity_layout.hpp"
#include <optional>

namespace deadtime {
struct RatsEvent {
    enum Kind { Sound, Hint, ClearHint, Finish } kind;
    std::string value;
};
class Rats {
    friend class ActivityCheckpoints;

  public:
    Rats(const Assets& assets, const std::string& path);
    bool turn(size_t mirror);
    bool skip();
    void idleClick(bool seeds = false);
    void tick(double seconds);
    void enter();
    void leave();
    bool busy() const {
        return fading_.has_value();
    }
    bool complete() const {
        return complete_;
    }
    bool holdingDialogs() const {
        return complete_ && busy();
    }
    bool gone(size_t rat) const {
        return gone_.at(rat);
    }
    size_t goneCount() const;
    size_t ratCount() const {
        return gone_.size();
    }
    size_t mirrorCount() const {
        return mirrors_.size();
    }
    size_t state(size_t mirror) const {
        return mirrors_.at(mirror).state;
    }
    const ActivityObject& mirror(size_t index) const;
    const ActivityObject& object(const std::string& id) const;
    const auto& objects() const {
        return objects_;
    }
    const auto& beams() const {
        return beams_;
    }
    std::vector<RatsEvent> takeEvents();
    std::string onFinish;
    bool rewarded = false;

  private:
    enum class Kind { Window, Wall, Mirror, Rat };
    struct Target {
        Kind kind;
        size_t index = 0;
        bool operator==(const Target&) const = default;
    };
    struct Rule {
        Target from, to;
        size_t beam = 0;
    };
    struct Mirror {
        size_t state = 0;
        bool forward = true;
        std::vector<std::vector<Rule>> states;
    };
    std::vector<Mirror> mirrors_;
    std::vector<ActivityObject> objects_;
    std::vector<RatsEvent> events_;
    std::vector<bool> gone_, beams_;
    std::optional<size_t> fading_, retainedRat_;
    size_t windowBeam_ = 0, windowTarget_ = 0;
    unsigned mirrorSound_ = 0, ratSound_ = 0;
    double fadeAge_ = 0;
    bool complete_ = false;
    static std::string number(size_t index);
    ActivityObject& mutableObject(const std::string& id);
    Target parseTarget(const std::string& text) const;
    void reflect();
    void updateVisuals();
};
} // namespace deadtime
