#include "animation.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace deadtime {
std::string textureResource(const Assets& assets, const std::string& name) {
    const auto key = normalize(name);
    if (key.empty())
        return {};
    if (assets.contains(key))
        return key;
    const auto dot = key.find_last_of('.');
    if (dot != std::string::npos) {
        const auto extension = key.substr(dot);
        if (extension == ".png" || extension == ".tga" || extension == ".bmp" || extension == ".jpg") {
            const auto jpeg = key.substr(0, dot) + ".jpg";
            if (assets.contains(jpeg))
                return jpeg;
        }
    }
    for (const auto& extension : {".jpg", ".ifl"})
        if (assets.contains(key + extension))
            return key + extension;
    return {};
}
ImageSequence::ImageSequence(const Assets& assets, const std::string& path) {
    auto source = assets.read(path);
    if (source.starts_with("\xEF\xBB\xBF"))
        source.erase(0, 3);
    std::istringstream lines(source);
    double end = 0;
    for (std::string line; std::getline(lines, line);) {
        line.erase(line.find(';') == std::string::npos ? line.size() : line.find(';'));
        std::istringstream tokens(line);
        std::string name, count, extra;
        if (!(tokens >> name))
            continue;
        double repeats = 1;
        if (tokens >> count) {
            size_t used = 0;
            repeats = std::stod(count, &used);
            if (used != count.size() || (tokens >> extra))
                throw std::runtime_error("Invalid IFL frame: " + path);
        }
        if (!std::isfinite(repeats) || repeats <= 0)
            throw std::runtime_error("Invalid IFL duration: " + path);
        const auto texture = textureResource(assets, resolve(directory(path), name));
        if (texture.empty() || texture.ends_with(".ifl"))
            throw std::runtime_error("Missing or nested IFL frame: " + name);
        const auto nextEnd = end + repeats * 0.04;
        if (!std::isfinite(nextEnd) || nextEnd <= end)
            throw std::runtime_error("Unrepresentable IFL duration: " + path);
        end = nextEnd;
        frames_.push_back({texture, end});
    }
    if (frames_.empty())
        throw std::runtime_error("Empty IFL sequence: " + path);
}
const std::string& ImageSequence::frame(double elapsedSeconds) const {
    if (!std::isfinite(elapsedSeconds))
        throw std::runtime_error("Invalid animation time");
    const auto time = std::fmod(std::max(0., elapsedSeconds), duration());
    const auto found =
        std::upper_bound(frames_.begin(), frames_.end(), time,
                         [](double value, const AnimationFrame& frame) { return value < frame.endSeconds; });
    return (found == frames_.end() ? frames_.front() : *found).texture;
}
} // namespace deadtime
