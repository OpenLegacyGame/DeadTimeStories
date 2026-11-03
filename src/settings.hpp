#pragma once
#include <array>
#include <filesystem>

namespace deadtime {
enum class AudioBus { Effects, Environment, Music };
class Settings {
  public:
    explicit Settings(std::filesystem::path file = {});
    float volume(AudioBus bus) const {
        return volumes_.at(static_cast<size_t>(bus));
    }
    void volume(AudioBus bus, float value);
    bool windowed() const {
        return windowed_;
    }
    void windowed(bool value) {
        windowed_ = value;
    }
    void save() const;

  private:
    std::filesystem::path file_;
    std::array<float, 3> volumes_{1, 1, 1};
    bool windowed_ = true;
};
} // namespace deadtime
