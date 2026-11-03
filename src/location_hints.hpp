#pragma once
#include "gui.hpp"
#include <map>
namespace deadtime {
struct LocationHintChoice {
    std::string key, group, groupText, title;
    bool operator==(const LocationHintChoice&) const = default;
};
class LocationHintMenu {
  public:
    explicit LocationHintMenu(Assets& assets);
    void update(const std::vector<LocationHintChoice>& choices, bool affordable, bool hasHistory);
    std::vector<GuiElement> elements(const std::string& hovered = {}) const;
    const auto& choices() const {
        return choices_;
    }
    const auto& widgets() const {
        return elements_;
    }

  private:
    Assets& assets_;
    std::vector<LocationHintChoice> lastChoices_;
    bool cached_ = false, affordable_ = false, hasHistory_ = false;
    std::vector<GuiElement> elements_;
    std::map<std::string, std::string> choices_;
    float firstY_ = 0, stepY_ = 0, groupStepY_ = 0, bottom_ = 0, backHeight_ = 0;
};
} // namespace deadtime
