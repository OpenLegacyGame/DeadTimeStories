#pragma once
#include "assets.hpp"
#include <functional>
#include <map>
#include <optional>
#include <set>

namespace deadtime {
struct Object;
struct RevealEvent {
    enum Kind { Brush, Sound } kind;
    std::string value;
};
class RevealSpirits {
    friend class ActivityCheckpoints;

  public:
    struct Target {
        Object* spirit;
        Object* mask;
    };
    RevealSpirits(const Assets& assets, std::vector<Target> targets, std::string brush, Object& container);
    void move(int x, int y, std::optional<size_t> hit);
    void pauseMotion();
    void tick(double seconds, const std::function<void(Object&)>& onRevealed = {});
    bool complete() const {
        return pending_.empty();
    }
    bool brushing() const {
        return brushPlaying_;
    }
    bool pending(size_t index) const;
    const auto& targets() const {
        return targets_;
    }
    const auto& position() const {
        return position_;
    }
    Object& container() const {
        return *container_;
    }
    std::vector<RevealEvent> takeEvents();
    std::string brush, onExit, brushSound, revealedSound;
    double revealSpeed = 0, firstTouchBoost = 0, vanishSpeed = 0, movementSeconds = 0, vanishSoundDelay = 0;

  private:
    std::vector<Target> targets_;
    std::vector<size_t> pending_;
    std::vector<RevealEvent> events_;
    Object* container_;
    std::optional<std::pair<int, int>> position_, previous_;
    std::optional<size_t> hit_;
    std::map<std::string, double> vanishCooldowns_;
    std::set<std::string> females_;
    std::string maleVanish_, femaleVanish_;
    double stationaryAge_ = 0;
    bool brushPlaying_ = false;
};
} // namespace deadtime
