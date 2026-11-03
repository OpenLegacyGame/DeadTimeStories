#include "audio_timing.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace deadtime {
namespace {
uint64_t littleEndian(const std::string& bytes, size_t at, size_t count) {
    if (at > bytes.size() || count > bytes.size() - at)
        throw std::runtime_error("Truncated Vorbis audio header");
    uint64_t value = 0;
    for (size_t i = 0; i < count; ++i)
        value |= uint64_t(static_cast<unsigned char>(bytes[at + i])) << (i * 8);
    return value;
}
} // namespace
// Timed sound callbacks also need to work without an audio device.
double vorbisDuration(const Assets& assets, const std::string& path) {
    const auto bytes = assets.read(path);
    const auto identification = bytes.find(std::string("\x01vorbis", 7));
    if (identification == std::string::npos)
        throw std::runtime_error("Unsupported Vorbis audio: " + path);
    const auto rate = littleEndian(bytes, identification + 12, 4);
    uint64_t samples = 0;
    for (size_t at = 0; at < bytes.size();) {
        if (bytes.compare(at, 4, "OggS") != 0 || bytes.size() - at < 27)
            throw std::runtime_error("Invalid Ogg page: " + path);
        const auto granule = littleEndian(bytes, at + 6, 8);
        if (granule != std::numeric_limits<uint64_t>::max())
            samples = std::max(samples, granule);
        const auto segments = littleEndian(bytes, at + 26, 1);
        size_t length = 27 + size_t(segments);
        for (size_t i = 0; i < segments; ++i)
            length += size_t(littleEndian(bytes, at + 27 + i, 1));
        if (length > bytes.size() - at)
            throw std::runtime_error("Truncated Ogg page: " + path);
        at += length;
    }
    if (rate == 0)
        throw std::runtime_error("Invalid Vorbis audio sample rate: " + path);
    return double(samples) / double(rate);
}
} // namespace deadtime
