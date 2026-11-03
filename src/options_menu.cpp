#include "options_menu.hpp"
#include <algorithm>
namespace deadtime {
std::optional<AudioBus> OptionsMenu::bus(const std::string& id) {
    if (id == "sfx_volume")
        return AudioBus::Effects;
    if (id == "env_volume")
        return AudioBus::Environment;
    if (id == "music_volume")
        return AudioBus::Music;
    return {};
}
OptionsMenu::OptionsMenu(Assets& assets, Settings& settings, bool paused)
    : settings_(settings),
      layout_(loadGui(assets, paused ? "menu/pausemenu/layout.gui" : "menu/options/options.gui")) {}
std::vector<GuiElement> OptionsMenu::elements() const {
    auto result = layout_;
    for (auto& element : result) {
        if (auto channel = bus(element.id)) {
            const auto value = settings_.volume(*channel);
            element.x += element.slideStartX + (element.slideEndX - element.slideStartX) * value;
            element.y += element.slideStartY + (element.slideEndY - element.slideStartY) * value;
            if (element.id == dragged_ && !element.clicked.empty())
                element.normal = element.hover = element.clicked;
        }
        if (element.id == "windowed") {
            element.visible = windowAvailable_;
            if (settings_.windowed()) {
                element.normal = element.normalChecked;
                element.hover = element.hoverChecked;
                element.clicked = element.clickedChecked;
            }
        }
        if (element.id == "no_windowed")
            element.visible = !windowAvailable_;
    }
    return result;
}
bool OptionsMenu::begin(const std::string& id, float x, float y) {
    release();
    if (!bus(id))
        return false;
    for (const auto& element : elements())
        if (element.id == id) {
            dragged_ = id;
            offsetX_ = x - element.x;
            offsetY_ = y - element.y;
            return true;
        }
    return false;
}
bool OptionsMenu::move(float x, float y) {
    const auto channel = bus(dragged_);
    if (!channel)
        return false;
    for (const auto& element : layout_)
        if (element.id == dragged_) {
            const float dx = element.slideEndX - element.slideStartX,
                        dy = element.slideEndY - element.slideStartY, length = dx * dx + dy * dy;
            if (length == 0)
                return false;
            settings_.volume(*channel, std::clamp(((x - offsetX_ - element.x - element.slideStartX) * dx +
                                                   (y - offsetY_ - element.y - element.slideStartY) * dy) /
                                                      length,
                                                  0.f, 1.f));
            return true;
        }
    return false;
}
} // namespace deadtime
