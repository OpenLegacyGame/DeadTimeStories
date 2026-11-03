#include "scene_particles.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace deadtime {
SceneParticles::SceneParticles(const Assets& assets, uint32_t seed) : assets_(assets), random_(seed) {}
std::shared_ptr<const ParticleLayout> SceneParticles::layout(const std::string& path) {
    auto found = layouts_.find(path);
    if (found == layouts_.end())
        found = layouts_.emplace(path, std::make_shared<ParticleLayout>(assets_, path)).first;
    return found->second;
}
ColladaScene& SceneParticles::model(const std::string& path) {
    auto found = models_.find(path);
    if (found == models_.end())
        found = models_.emplace(path, std::make_unique<ColladaScene>(assets_, path)).first;
    return *found->second;
}
SceneParticles::State& SceneParticles::ensure(const Object& object) {
    auto [found, inserted] = states_.try_emplace(&object);
    if (!inserted)
        return found->second;
    auto& state = found->second;
    try {
        auto append = [&](const Object::ParticleResource& resource, int scene, const std::string& helper) {
            if (!std::isfinite(resource.presim) || resource.presim > 3600)
                throw std::runtime_error("Invalid or oversized particle pre-simulation");
            Instance value;
            value.emitter = std::make_unique<ParticleEmitter>(layout(resource.path), random_);
            value.emitter->setGenerating(object.particleGenerating);
            value.scene = scene;
            value.helper = helper;
            value.presim = resource.presim;
            state.instances.push_back(std::move(value));
        };
        for (const auto& resource : object.particleResources)
            append(resource, -1, {});
        for (size_t i = 0; i < object.sceneResources.size(); ++i) {
            const auto& resource = object.sceneResources[i];
            if (resource.particles.empty())
                continue;
            const auto frame = model(resource.path).frame(0);
            // Native 587450 recursively attaches to every node whose name
            // starts with an underscore, including mesh nodes used by pieces.
            for (const auto& particle : resource.particles)
                for (const auto& name : frame.nodeOrder)
                    if (name.starts_with('_'))
                        append(particle, int(i), name);
        }
    } catch (...) {
        state.failed = true;
        throw;
    }
    return state;
}
void SceneParticles::tick(Object& object, double seconds) {
    if (object.particleResources.empty() &&
        std::all_of(object.sceneResources.begin(), object.sceneResources.end(),
                    [](const auto& scene) { return scene.particles.empty(); })) {
        object.particlePresimPending = 0;
        return;
    }
    if (!std::isfinite(seconds) || seconds < 0 || seconds > 3600 ||
        !std::isfinite(object.particlePresimPending) || object.particlePresimPending > 3600)
        throw std::runtime_error("Invalid or oversized particle time advance");
    auto& state = ensure(object);
    if (state.failed)
        return;
    for (auto& instance : state.instances)
        if (instance.scene < 0)
            instance.presim += object.particlePresimPending;
    object.particlePresimPending = 0;
    double remaining = seconds, advanced = 0;
    do {
        // Interactive updates already use small frame deltas. Offline large
        // advances use the same 0.1s steps as the native presimulation path.
        const double dt = std::min(remaining, .1);
        advanced += dt;
        std::map<int, ModelFrame> frames;
        const double time =
            std::max(0., object.treePlaying ? object.treeTime - seconds + advanced : object.treeTime);
        for (auto& instance : state.instances) {
            auto pose = modelIdentity();
            if (instance.scene >= 0) {
                auto found = frames.find(instance.scene);
                if (found == frames.end()) {
                    const auto& resource = object.sceneResources.at(size_t(instance.scene));
                    found = frames
                                .emplace(instance.scene,
                                         model(resource.path)
                                             .frame(time, object.treeLoopOverridden ? object.treeLooped
                                                                                    : resource.looped))
                                .first;
                }
                pose = found->second.transforms.at(instance.helper);
            }
            pose[3] += object.x;
            pose[7] += object.y;
            pose[11] += object.z;
            instance.transform = pose;
            const ModelVector position{pose[3], pose[7], pose[11]};
            instance.emitter->setGenerating(object.particleGenerating);
            while (instance.presim > 0) {
                const auto step = std::min(.1, instance.presim);
                instance.emitter->tick(step, position);
                instance.presim -= step;
            }
            instance.emitter->tick(dt, position);
        }
        remaining -= dt;
    } while (remaining > 1e-10);
}
std::vector<ParticleBatch> SceneParticles::draw(const Object& object) {
    std::vector<ParticleBatch> result;
    auto found = states_.find(&object);
    if (found == states_.end() || found->second.failed)
        return result;
    for (auto& instance : found->second.instances) {
        const auto& definition = instance.emitter->layout();
        result.push_back({instance.emitter->definition(), instance.emitter->sprites(),
                          definition.worldEmitting ? modelIdentity() : instance.transform});
    }
    return result;
}
void SceneParticles::setGenerating(const Object& object, bool enabled) {
    const auto found = states_.find(&object);
    if (found == states_.end())
        return;
    for (auto& instance : found->second.instances)
        instance.emitter->setGenerating(enabled);
}
size_t SceneParticles::emitterCount(const Object& object) const {
    const auto found = states_.find(&object);
    return found == states_.end() ? 0 : found->second.instances.size();
}
} // namespace deadtime
