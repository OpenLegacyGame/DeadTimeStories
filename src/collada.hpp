#pragma once
#include "assets.hpp"
#include <array>

namespace deadtime {
struct ModelVector {
    float x = 0, y = 0, z = 0;
};
struct ModelVertex {
    ModelVector position;
    float u = 0, v = 0;
};
using ModelTransform = std::array<float, 16>;
ModelTransform modelIdentity();
ModelVector modelPoint(const ModelTransform& transform, ModelVector point);
struct ModelMesh {
    std::string texture;
    bool additive = false;
    // Expanded triangles. Positions use the original engine's screen axes.
    std::vector<ModelVertex> vertices;
};
struct ModelFrame {
    std::vector<ModelMesh> meshes;
    std::map<std::string, ModelVector> nodes;
    std::map<std::string, ModelTransform> transforms;
    std::vector<std::string> nodeOrder;
};
// Imports the game's Collada dialect, including its native axis conversion.
// A model contains no SDL resources, so the same geometry can be inspected or
// rendered with a different graphics backend.
class ColladaScene {
  public:
    ColladaScene(const Assets& assets, const std::string& path);
    ModelFrame frame(double seconds, bool looped = true) const;
    double duration() const {
        return duration_;
    }
    size_t nodeCount() const {
        return nodes_.size();
    }

  private:
    struct Track {
        std::vector<double> times;
        std::vector<std::vector<float>> values;
        std::vector<float> sample(double time) const;
    };
    struct Rotation {
        ModelVector axis;
        float degrees = 0;
        std::string sid;
    };
    struct Node {
        std::string id, name, translationSid;
        int parent = -1;
        ModelVector translation;
        std::vector<Rotation> rotations;
        std::vector<ModelMesh> meshes;
        std::map<std::string, Track> tracks;
        double duration = 0;
    };
    std::vector<Node> nodes_;
    double duration_ = 0;
};
} // namespace deadtime
