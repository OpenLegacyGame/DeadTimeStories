#pragma once
#include "assets.hpp"

namespace deadtime {
// Resolves logical image names, including legacy PNG/TGA references stored as JPEG.
std::string textureResource(const Assets& assets, const std::string& name);
struct AnimationFrame {
    std::string texture;
    double endSeconds = 0;
};
class ImageSequence {
  public:
    ImageSequence(const Assets& assets, const std::string& path);
    const std::string& frame(double elapsedSeconds) const;
    double duration() const {
        return frames_.back().endSeconds;
    }
    size_t size() const {
        return frames_.size();
    }

  private:
    std::vector<AnimationFrame> frames_;
};
} // namespace deadtime
