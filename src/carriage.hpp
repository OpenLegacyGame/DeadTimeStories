#pragma once
#include "gui.hpp"
#include <functional>
#include <optional>

namespace deadtime {
struct CarriageDestination {
    std::string zone, location, button;
    bool available = false;
};
class Carriage {
  public:
    explicit Carriage(Assets& assets);
    void enter(const std::string& currentZone, const std::function<bool(const std::string&)>& allowed);
    std::optional<std::string> destination(const std::string& button) const;
    const auto& destinations() const {
        return destinations_;
    }
    const auto& elements() const {
        return elements_;
    }
    std::vector<GuiElement> elements(const std::string& hovered) const;
    const GuiElement& element(const std::string& id) const;
    std::string shortcut(const std::string& key) const;

  private:
    std::vector<CarriageDestination> destinations_;
    std::vector<GuiElement> source_, elements_;
    float firstX_ = 0, firstY_ = 0, stepX_ = 0, stepY_ = 0;
};
} // namespace deadtime
