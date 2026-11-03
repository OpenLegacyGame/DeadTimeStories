#include "carriage.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <set>
#include <sstream>
#include <stdexcept>

namespace deadtime {
Carriage::Carriage(Assets& assets) {
    pugi::xml_document doc;
    const auto bytes = assets.read("carriage/config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("carriage"))
        throw std::runtime_error("Invalid carriage configuration");
    const auto root = doc.child("carriage");
    std::set<std::string> zones, buttons;
    for (auto node : root.child("destinations").children("destination")) {
        CarriageDestination destination{node.attribute("zone").value(), node.attribute("location").value(),
                                        node.attribute("button").value(), false};
        if (destination.zone.empty() || destination.location.empty() || destination.button.empty() ||
            !zones.insert(destination.zone).second || !buttons.insert(destination.button).second)
            throw std::runtime_error("Invalid carriage destination");
        destinations_.push_back(std::move(destination));
    }
    if (destinations_.empty())
        throw std::runtime_error("Empty carriage destinations");
    const auto first = root.child("layout").child("first_button"),
               step = root.child("layout").child("button_step");
    firstX_ = first.attribute("x").as_float();
    firstY_ = first.attribute("y").as_float();
    stepX_ = step.attribute("x").as_float();
    stepY_ = step.attribute("y").as_float();
    if (!std::isfinite(firstX_) || !std::isfinite(firstY_) || !std::isfinite(stepX_) ||
        !std::isfinite(stepY_))
        throw std::runtime_error("Invalid carriage button placement");
    source_ = elements_ = loadGui(assets, "carriage/layout.gui");
    element("button_exit");
    for (const auto& destination : destinations_)
        if (element(destination.button).kind != "button")
            throw std::runtime_error("Invalid carriage destination button");
}
const GuiElement& Carriage::element(const std::string& id) const {
    const auto found =
        std::find_if(elements_.begin(), elements_.end(), [&](const auto& e) { return e.id == id; });
    if (found == elements_.end())
        throw std::runtime_error("Missing carriage widget: " + id);
    return *found;
}
void Carriage::enter(const std::string& currentZone, const std::function<bool(const std::string&)>& allowed) {
    elements_ = source_;
    float x = firstX_, y = firstY_;
    // Native 554960 packs only allowed, non-current zones in config order.
    for (auto& destination : destinations_) {
        destination.available = destination.zone != currentZone && allowed(destination.zone);
        auto& button = const_cast<GuiElement&>(element(destination.button));
        const float dx = button.parentX + x - button.x, dy = button.parentY + y - button.y;
        for (auto& widget : elements_)
            if (widget.id == destination.button || std::find(widget.ancestors.begin(), widget.ancestors.end(),
                                                             destination.button) != widget.ancestors.end()) {
                widget.visible = widget.visible && destination.available;
                if (destination.available) {
                    widget.x += dx;
                    widget.y += dy;
                }
            }
        if (destination.available) {
            x += stepX_;
            y += stepY_;
        }
    }
}
std::optional<std::string> Carriage::destination(const std::string& button) const {
    for (const auto& destination : destinations_)
        if (destination.button == button && destination.available && element(button).visible &&
            element(button).enabled)
            return destination.zone + '.' + destination.location;
    return {};
}
std::vector<GuiElement> Carriage::elements(const std::string& hovered) const {
    auto result = elements_;
    const auto button = std::find_if(elements_.begin(), elements_.end(), [&](const auto& e) {
        return e.id == hovered && e.kind == "button" && e.visible && e.enabled;
    });
    if (button != elements_.end())
        for (auto& widget : result)
            if (const auto color = button->hoverTextColors.find(widget.id);
                color != button->hoverTextColors.end())
                widget.color = color->second;
    return result;
}
std::string Carriage::shortcut(const std::string& key) const {
    for (const auto& element : elements_)
        if (element.kind == "button" && element.visible && element.enabled) {
            std::istringstream shortcuts(element.shortcuts);
            for (std::string token; shortcuts >> token;)
                if (token == key)
                    return element.id;
        }
    return {};
}
} // namespace deadtime
