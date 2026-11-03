#pragma once
#include "collada.hpp"
#include <cstdint>
#include <memory>

namespace deadtime {
class ParticleRandom {
    friend class ActivityCheckpoints;

  public:
    explicit ParticleRandom(uint32_t seed = 1) : state_(seed) {}
    unsigned next() {
        state_ = state_ * 214013u + 2531011u;
        return (state_ >> 16) & 32767u;
    }
    float unit() {
        return float(double(next()) / 32767.);
    }

  private:
    uint32_t state_;
};
struct ParticleCurve {
    struct Key {
        unsigned type = 1;
        float time = 0, in = 0, out = 0, inX = -1, inY = 0, outX = 1, outY = 0;
    };
    std::vector<Key> keys;
    bool loopLeft = false, loopRight = false;
    float sample(float time) const;
};
class ParticleLayout {
  public:
    ParticleLayout(const Assets& assets, const std::string& path);
    size_t nodeCount() const {
        return nodes_.size();
    }
    size_t parameterCount() const {
        return parameters_;
    }
    std::string texture, material;
    bool worldEmitting = true, additive = false;
    unsigned slicesX = 1, slicesY = 1;

  private:
    friend class ParticleEmitter;
    enum class Type {
        Float,
        Vector,
        Spawn,
        Linear,
        Move,
        Gravity,
        Distribution,
        Distribution3,
        Add,
        Sub,
        Mul,
        Div,
        Add3,
        Split,
        Join,
        Track,
        Track3,
        Generator,
        Terminator,
        Constructor
    };
    struct Distribution {
        enum Mode { Fixed, Range, Box } mode = Fixed;
        std::array<double, 3> from{}, to{};
        double scalar(ParticleRandom& random) const;
        ModelVector vector(ParticleRandom& random) const;
    };
    struct Port {
        size_t node = 0;
        unsigned output = 0;
        bool connected = false;
    };
    struct Node {
        Type type;
        std::vector<Port> inputs;
        Distribution distribution;
        std::array<ParticleCurve, 3> curves;
        ModelVector gravity;
        float speed = 0;
        bool collective = false;
        size_t destination = 0;
    };
    std::vector<Node> nodes_;
    size_t parameters_ = 0, modifierBegin_ = 0, functionBegin_ = 0, generator_ = 0, terminator_ = 0,
           constructor_ = 0;
    Distribution phase_, interval_, burst_;
};
struct ParticleSprite {
    ModelVector position, color;
    float size = 0, alpha = 0, rotation = 0;
    unsigned tileX = 0, tileY = 0;
};
class ParticleEmitter {
  public:
    ParticleEmitter(std::shared_ptr<const ParticleLayout> layout, ParticleRandom& random);
    void tick(double seconds, ModelVector spawnPosition);
    void restartGenerator();
    void setGenerating(bool enabled) {
        if (enabled && !generating_)
            restartGenerator();
        generating_ = enabled;
    }
    std::vector<ParticleSprite> sprites();
    size_t size() const {
        return particles_.size();
    }
    const ParticleLayout& layout() const {
        return *layout_;
    }
    const std::shared_ptr<const ParticleLayout>& definition() const {
        return layout_;
    }
    // Exposes actual stored parameters for independent simulation checks.
    ModelVector parameter(size_t particle, size_t index) const;

  private:
    struct Particle {
        std::vector<ModelVector> values;
        unsigned tileX = 0, tileY = 0;
    };
    ModelVector evaluate(const ParticleLayout::Port& port, const Particle& particle, unsigned depth = 0);
    void emit(unsigned count, ModelVector position);
    std::shared_ptr<const ParticleLayout> layout_;
    ParticleRandom* random_;
    std::vector<Particle> particles_;
    double interval_ = 0, phase_ = 0;
    unsigned burst_ = 0;
    bool generating_ = true;
};
} // namespace deadtime
