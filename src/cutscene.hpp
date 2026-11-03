#pragma once
#include "animation_set.hpp"
#include <memory>

namespace deadtime {
struct CutsceneSound {
    double start = 0;
    std::string path;
    bool music = false;
};
struct CutsceneText {
    double start = 0, end = 0;
    std::string text;
};
class Cutscene {
  public:
    Cutscene(const Assets& assets, const std::string& path);
    void tick(double seconds);
    bool complete() const {
        return time_ >= end_;
    }
    double elapsed() const {
        return time_;
    }
    double duration() const {
        return end_;
    }
    const auto& objects() const {
        return objects_;
    }
    const auto& captions() const {
        return captions_;
    }
    std::vector<CutsceneSound> takeSounds();
    std::string after;
    bool skippable = true;

  private:
    struct Binding {
        size_t object = 0;
        double start = 0;
        std::shared_ptr<AnimationSet> animation;
    };
    std::vector<ActivityObject> initial_, objects_;
    std::vector<Binding> animations_;
    std::vector<CutsceneText> captions_;
    std::vector<CutsceneSound> sounds_, pendingSounds_;
    double time_ = 0, end_ = 0;
    size_t nextSound_ = 0;
};
} // namespace deadtime
