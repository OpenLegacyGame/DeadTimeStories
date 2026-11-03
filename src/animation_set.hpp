#pragma once
#include "activity_layout.hpp"
#include <array>

namespace deadtime {
enum class AnimatedProperty { Position, Scale, Alpha, Visible, Rotation };
struct PropertyKey {
    double seconds = 0;
    std::array<float, 3> value{};
};
struct PropertyTrack {
    AnimatedProperty property;
    double start = 0;
    bool looped = false;
    bool positionOffset = false;
    std::vector<PropertyKey> keys;
    std::array<float, 3> sample(double seconds) const;
};
struct PictureKey {
    double seconds = 0;
    std::string texture;
};
struct PictureTrack {
    double start = 0;
    bool looped = false;
    std::vector<PictureKey> keys;
};
class AnimationSet {
  public:
    AnimationSet(const Assets& assets, const std::string& path);
    void apply(ActivityObject& object, double seconds) const;
    double duration() const {
        return duration_;
    }
    bool looped() const {
        return looped_;
    }
    const auto& tracks() const {
        return tracks_;
    }
    const auto& pictures() const {
        return pictures_;
    }

  private:
    std::vector<PropertyTrack> tracks_;
    std::vector<PictureTrack> pictures_;
    double duration_ = 0;
    bool looped_ = false;
};
} // namespace deadtime
