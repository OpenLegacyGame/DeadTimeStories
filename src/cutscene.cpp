#include "cutscene.hpp"
#include "animation.hpp"
#include "audio_timing.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <pugixml.hpp>
#include <stdexcept>
#include <utility>

namespace deadtime {
namespace {
double milliseconds(const char* value) {
    size_t used = 0;
    const std::string text = value;
    const double result = std::stod(text, &used) / 1000;
    if (used != text.size() || !std::isfinite(result) || result < 0)
        throw std::runtime_error("Invalid cutscene time: " + text);
    return result;
}

} // namespace
Cutscene::Cutscene(const Assets& assets, const std::string& path) {
    initial_ = loadActivityLayout(assets, path);
    std::map<std::string, size_t> objects;
    for (size_t i = 0; i < initial_.size(); ++i) {
        if (initial_[i].id.empty())
            throw std::runtime_error("Unnamed cutscene object: " + path);
        // SceneNode lookup walks children in creation order. Repeated names
        // remain separate drawables; an unindexed reference finds the first.
        objects.emplace(initial_[i].id, i);
        if (textureResource(assets, initial_[i].texture).empty())
            throw std::runtime_error("Missing cutscene texture: " + initial_[i].texture);
    }
    const auto base = directory(path), data = assets.read(path);
    pugi::xml_document doc;
    if (!doc.load_buffer(data.data(), data.size()))
        throw std::runtime_error("Invalid cutscene: " + path);
    const auto actions = doc.child("cutscene").child("actions");
    if (!actions)
        throw std::runtime_error("Missing cutscene actions: " + path);
    std::map<std::string, std::shared_ptr<AnimationSet>> sets;
    std::map<std::string, double> audioDurations;
    end_ = std::numeric_limits<double>::infinity();
    std::function<double(pugi::xml_node, double)> compile = [&](auto node, double start) -> double {
        const std::string kind = node.name();
        double finish = start;
        if (kind == "group") {
            for (auto child : node.children())
                if (std::string(child.name()) != "onFinish")
                    finish = std::max(finish, compile(child, start));
        } else if (kind == "timer") {
            finish += milliseconds(node.attribute("ms").value());
        } else if (kind == "anim") {
            const std::string name = node.attribute("object").value();
            if (!objects.contains(name))
                throw std::runtime_error("Unknown animated cutscene object: " + name);
            const auto resource = resolve(base, node.attribute("src").value()) + ".animset";
            if (!sets.contains(resource))
                sets[resource] = std::make_shared<AnimationSet>(assets, resource);
            const auto& animation = sets.at(resource);
            finish += animation->looped() ? std::numeric_limits<double>::infinity() : animation->duration();
            animations_.push_back({objects.at(name), start, animation});
        } else if (kind == "text") {
            finish += milliseconds(node.attribute("time").value());
            captions_.push_back({start, finish, node.attribute("msg").value()});
        } else if (kind == "sound" || kind == "streamsound" || kind == "streammusic") {
            auto resource = resolve(base, node.attribute("src").value());
            if (!resource.ends_with(".ogg"))
                resource += ".ogg";
            if (!audioDurations.contains(resource))
                audioDurations[resource] = vorbisDuration(assets, resource);
            finish += audioDurations.at(resource);
            sounds_.push_back({start, resource, kind == "streammusic"});
        } else if (kind == "end") {
            end_ = std::min(end_, start);
        } else
            throw std::runtime_error("Unsupported cutscene action: " + kind);
        // A group's ordinary children are parallel members. Its onFinish list,
        // and other actions' children, begin when that action itself completes.
        const auto continuation = kind == "group" ? node.child("onFinish") : node;
        for (auto child : continuation.children())
            compile(child, finish);
        return finish;
    };
    for (auto node : actions.children())
        compile(node, 0);
    if (!std::isfinite(end_))
        throw std::runtime_error("Cutscene has no reachable end: " + path);
    std::stable_sort(animations_.begin(), animations_.end(),
                     [](const auto& a, const auto& b) { return a.start < b.start; });
    std::stable_sort(sounds_.begin(), sounds_.end(),
                     [](const auto& a, const auto& b) { return a.start < b.start; });
    tick(0);
}
void Cutscene::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid cutscene tick");
    time_ = std::min(time_ + seconds, end_);
    objects_ = initial_;
    for (const auto& binding : animations_)
        if (binding.start <= time_)
            binding.animation->apply(objects_[binding.object], time_ - binding.start);
    while (nextSound_ < sounds_.size() && sounds_[nextSound_].start <= time_) {
        if (sounds_[nextSound_].start < end_)
            pendingSounds_.push_back(sounds_[nextSound_]);
        ++nextSound_;
    }
}
std::vector<CutsceneSound> Cutscene::takeSounds() {
    return std::exchange(pendingSounds_, {});
}
} // namespace deadtime
