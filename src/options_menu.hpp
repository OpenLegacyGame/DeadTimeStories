#pragma once
#include "gui.hpp"
#include "settings.hpp"
#include <optional>
namespace deadtime {
// The same source slider/checkbox definitions are shared by title and pause.
class OptionsMenu {
  public:
    OptionsMenu(Assets& assets, Settings& settings, bool paused);
    std::vector<GuiElement> elements() const;
    bool begin(const std::string& id, float x, float y);
    bool move(float x, float y);
    bool dragging() const {
        return !dragged_.empty();
    }
    void release() {
        dragged_.clear();
    }
    void windowAvailable(bool available) {
        windowAvailable_ = available;
    }

  private:
    Settings& settings_;
    std::vector<GuiElement> layout_;
    std::string dragged_;
    float offsetX_ = 0, offsetY_ = 0;
    bool windowAvailable_ = true;
    static std::optional<AudioBus> bus(const std::string& id);
};
} // namespace deadtime
