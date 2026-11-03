#include "animation_set.hpp"
#include "animation.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <set>
#include <sstream>
#include <stdexcept>

namespace deadtime {
namespace {
double timeValue(const char* text) {
    size_t used = 0;
    const std::string value = text;
    const double result = std::stod(value, &used);
    if (used != value.size() || !std::isfinite(result) || result < 0)
        throw std::runtime_error("Invalid animation time: " + value);
    return result;
}
} // namespace
std::array<float, 3> PropertyTrack::sample(double seconds) const {
    const auto length = keys.back().seconds;
    const auto time = looped && length > 0 ? std::fmod(seconds, length) : seconds;
    auto next = std::upper_bound(keys.begin(), keys.end(), time,
                                 [](double t, const auto& key) { return t < key.seconds; });
    if (next == keys.begin())
        return next->value;
    if (next == keys.end())
        return keys.back().value;
    const auto& before = *std::prev(next);
    const float ratio = float((time - before.seconds) / (next->seconds - before.seconds));
    auto value = before.value;
    for (size_t i = 0; i < value.size(); ++i)
        value[i] += ratio * (next->value[i] - value[i]);
    return value;
}
AnimationSet::AnimationSet(const Assets& assets, const std::string& path) {
    auto bytes = assets.read(path);
    pugi::xml_document doc;
    auto parse = [&] {
        if (!doc.load_buffer(bytes.data(), bytes.size(), pugi::parse_default | pugi::parse_fragment))
            throw std::runtime_error("Invalid animation set: " + path);
    };
    parse();
    const std::string replacement = doc.child("replacement").attribute("src").value();
    if (!replacement.empty()) {
        pugi::xml_document substitutions;
        const auto data = assets.read(resolve(directory(path), replacement));
        if (!substitutions.load_buffer(data.data(), data.size()))
            throw std::runtime_error("Invalid animation replacements: " + path);
        for (auto entry : substitutions.child("replacements").children("replace")) {
            const std::string from = entry.attribute("src").value(), to = entry.attribute("dest").value();
            if (from.empty())
                throw std::runtime_error("Empty animation replacement: " + path);
            size_t at = 0;
            while ((at = bytes.find(from, at)) != std::string::npos) {
                bytes.replace(at, from.size(), to);
                at += to.size();
            }
        }
        parse();
    }
    const auto root = doc.child("animation_set");
    if (!root)
        throw std::runtime_error("Missing animation set: " + path);
    looped_ = root.attribute("looped").as_bool();
    for (auto reference : root.children("animation")) {
        pugi::xml_document external;
        auto node = external.append_copy(reference);
        std::string origin = path;
        std::set<std::string> visited{normalize(path)};
        while (node.attribute("src")) {
            const auto source = resolve(directory(origin), node.attribute("src").value()) + ".anim";
            if (visited.size() >= 16 || !visited.insert(source).second)
                throw std::runtime_error("Animation source cycle: " + source);
            pugi::xml_document loaded;
            const auto data = assets.read(source);
            if (!loaded.load_buffer(data.data(), data.size()) || !loaded.child("animation"))
                throw std::runtime_error("Invalid external animation: " + source);
            auto merged = external.append_copy(loaded.child("animation"));
            for (auto attribute : node.attributes())
                if (std::string(attribute.name()) != "src") {
                    auto dest = merged.attribute(attribute.name());
                    if (!dest)
                        dest = merged.append_attribute(attribute.name());
                    dest.set_value(attribute.value());
                }
            external.remove_child(node);
            node = merged;
            origin = source;
        }
        const std::string type = node.attribute("type").value();
        if ((type != "keys" && type != "const") || node.attribute("weight").as_float(1) != 1)
            throw std::runtime_error("Unsupported animation source or weight: " + path);
        PropertyTrack track;
        const std::string property = node.attribute("applier").value();
        if (property == "picture") {
            const auto keys = node.child("keys");
            if (type != "keys" || std::string(keys.attribute("type").value()) != "picture" ||
                keys.child("cyclic").attribute("value").as_bool())
                throw std::runtime_error("Unsupported picture track: " + path);
            PictureTrack picture;
            picture.start = timeValue(node.attribute("start").as_string("0"));
            picture.looped = node.attribute("looped").as_bool();
            double elapsed = 0;
            for (auto key : keys.child("picture").children("key")) {
                elapsed += timeValue(key.attribute("relative_time").as_string("0"));
                const auto texture = resolve(directory(origin), key.text().as_string());
                if (!std::isfinite(elapsed) || textureResource(assets, texture).empty())
                    throw std::runtime_error("Invalid picture key: " + path);
                picture.keys.push_back({elapsed, texture});
            }
            if (picture.keys.empty())
                throw std::runtime_error("Empty picture track: " + path);
            duration_ = std::max(duration_, picture.start + elapsed);
            pictures_.push_back(std::move(picture));
            continue;
        }
        if (property == "position")
            track.property = AnimatedProperty::Position;
        else if (property == "scale")
            track.property = AnimatedProperty::Scale;
        else if (property == "alpha")
            track.property = AnimatedProperty::Alpha;
        else if (property == "visible")
            track.property = AnimatedProperty::Visible;
        else if (property == "rotationZ")
            track.property = AnimatedProperty::Rotation;
        else
            throw std::runtime_error("Unsupported animation property: " + property);
        track.start = timeValue(node.attribute("start").as_string("0"));
        track.looped = node.attribute("looped").as_bool();
        track.positionOffset = type == "const" && track.property == AnimatedProperty::Position &&
                               node.attribute("priority").as_int() > 0;
        if (type == "const") {
            const auto value = node.child("const");
            const bool vector = track.property == AnimatedProperty::Position;
            if (std::string(value.attribute("type").value()) != (vector ? "vector3" : "float"))
                throw std::runtime_error("Constant animation type mismatch: " + path);
            PropertyKey key;
            std::istringstream fields(value.text().as_string());
            for (int i = 0; i < (vector ? 3 : 1); ++i)
                if (!(fields >> key.value[size_t(i)]) || !std::isfinite(key.value[size_t(i)]))
                    throw std::runtime_error("Invalid constant animation: " + path);
            std::string extra;
            if (fields >> extra)
                throw std::runtime_error("Extra constant animation value: " + path);
            track.keys.push_back(key);
            duration_ = std::max(duration_, track.start);
            tracks_.push_back(std::move(track));
            continue;
        }
        const auto keys = node.child("keys");
        if (keys.child("cyclic").attribute("value").as_bool())
            throw std::runtime_error("Unsupported cyclic key interpolation: " + path);
        const bool vector = track.property == AnimatedProperty::Position;
        if (std::string(keys.attribute("type").value()) != (vector ? "vector3" : "float"))
            throw std::runtime_error("Animation property/key type mismatch: " + path);
        double elapsed = 0;
        for (auto key : keys.child(vector ? "vector3" : "float").children("key")) {
            elapsed += timeValue(key.attribute("relative_time").as_string("0"));
            PropertyKey value;
            value.seconds = elapsed;
            std::istringstream fields(key.text().as_string());
            for (int i = 0; i < (vector ? 3 : 1); ++i)
                if (!(fields >> value.value[size_t(i)]) || !std::isfinite(value.value[size_t(i)]))
                    throw std::runtime_error("Invalid animation key value: " + path);
            std::string extra;
            if ((fields >> extra) || !std::isfinite(elapsed))
                throw std::runtime_error("Invalid animation key: " + path);
            track.keys.push_back(value);
        }
        if (track.keys.empty())
            throw std::runtime_error("Empty animation track: " + path);
        duration_ = std::max(duration_, track.start + elapsed);
        tracks_.push_back(std::move(track));
    }
    if (tracks_.empty() && pictures_.empty())
        throw std::runtime_error("Empty animation set: " + path);
}
void AnimationSet::apply(ActivityObject& object, double seconds) const {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid animation sample time");
    const double time =
        looped_ && duration_ > 0 ? std::fmod(seconds, duration_) : std::min(seconds, duration_);
    std::array<float, 3> position{}, offset{};
    bool hasPosition = false;
    for (const auto& track : tracks_) {
        if (time < track.start)
            continue;
        const auto value = track.sample(time - track.start);
        switch (track.property) {
        case AnimatedProperty::Position:
            if (track.positionOffset)
                for (size_t i = 0; i < 3; ++i)
                    offset[i] += value[i];
            else
                position = value;
            hasPosition = true;
            break;
        case AnimatedProperty::Scale:
            object.scale = value[0];
            break;
        case AnimatedProperty::Alpha:
            object.alpha = value[0];
            break;
        case AnimatedProperty::Visible:
            object.visible = value[0] > 0;
            break;
        case AnimatedProperty::Rotation:
            object.angle = value[0];
            break;
        }
    }
    if (hasPosition) {
        object.x = position[0] + offset[0];
        object.y = position[1] + offset[1];
        object.z = position[2] + offset[2];
    }
    for (const auto& track : pictures_)
        if (time >= track.start) {
            double local = time - track.start;
            if (track.looped && track.keys.back().seconds > 0)
                local = std::fmod(local, track.keys.back().seconds);
            auto next = std::upper_bound(track.keys.begin(), track.keys.end(), local,
                                         [](double t, const auto& key) { return t < key.seconds; });
            const auto& key = next == track.keys.begin() ? *next : *std::prev(next);
            object.texture = key.texture;
            object.width = object.height = 0;
        }
}
} // namespace deadtime
