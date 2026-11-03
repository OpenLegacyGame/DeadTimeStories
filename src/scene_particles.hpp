#pragma once
#include "particles.hpp"
namespace deadtime {
struct Object;
struct ParticleBatch {
    std::shared_ptr<const ParticleLayout> layout;
    std::vector<ParticleSprite> sprites;
    ModelTransform transform;
};
class SceneParticles {
  public:
    explicit SceneParticles(const Assets& assets, uint32_t seed);
    void tick(Object& object, double seconds);
    std::vector<ParticleBatch> draw(const Object& object);
    void setGenerating(const Object& object, bool enabled);
    void reset(const Object& object) {
        states_.erase(&object);
    }
    size_t emitterCount(const Object& object) const;

  private:
    struct Instance {
        std::unique_ptr<ParticleEmitter> emitter;
        std::string helper;
        int scene = -1;
        double presim = 0;
        ModelTransform transform = modelIdentity();
    };
    struct State {
        std::vector<Instance> instances;
        bool failed = false;
    };
    State& ensure(const Object& object);
    std::shared_ptr<const ParticleLayout> layout(const std::string& path);
    ColladaScene& model(const std::string& path);
    const Assets& assets_;
    ParticleRandom random_;
    std::map<const Object*, State> states_;
    std::map<std::string, std::shared_ptr<const ParticleLayout>> layouts_;
    std::map<std::string, std::unique_ptr<ColladaScene>> models_;
};
} // namespace deadtime
