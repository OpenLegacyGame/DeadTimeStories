#include "location_hints.hpp"
#include "activity_layout.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>
namespace deadtime {
LocationHintMenu::LocationHintMenu(Assets& assets) : assets_(assets) {
    pugi::xml_document doc;
    const auto bytes = assets.read("hud/common/_select_hint_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid location hint configuration");
    const auto root = doc.child("constants");
    firstY_ = root.attribute("first_objective_y").as_float();
    stepY_ = root.attribute("hint_step_y").as_float();
    groupStepY_ = root.attribute("next_objective_step_y").as_float();
    bottom_ = root.attribute("bottom_margin").as_float();
    if (!root || !std::isfinite(firstY_ + stepY_ + groupStepY_ + bottom_) || stepY_ <= 0)
        throw std::runtime_error("Invalid location hint spacing");
    const auto widgets = loadGui(assets, "hud/common/hud_select_hint.gui");
    const auto back =
        std::find_if(widgets.begin(), widgets.end(), [](const auto& e) { return e.id == "select_hint_bg"; });
    if (back == widgets.end())
        throw std::runtime_error("Missing location hint background");
    ActivityObject sprite;
    sprite.texture = back->normal;
    std::vector<ActivityObject> measured{sprite};
    measureActivityTextures(assets, measured);
    backHeight_ = measured.front().height;
}
void LocationHintMenu::update(const std::vector<LocationHintChoice>& choices, bool affordable,
                              bool hasHistory) {
    if (cached_ && lastChoices_ == choices && affordable_ == affordable && hasHistory_ == hasHistory)
        return;
    elements_ = loadGui(assets_, "hud/common/hud_select_hint.gui",
                        {{"select_hint", !choices.empty()},
                         {"no_hints_available", choices.empty()},
                         {"cant_afford_hint", false}});
    choices_.clear();
    auto descendants = [&](const std::string& id, auto action) {
        for (auto& e : elements_)
            if (e.id == id || std::find(e.ancestors.begin(), e.ancestors.end(), id) != e.ancestors.end())
                action(e);
    };
    auto find = [&](const std::string& id) -> GuiElement& {
        const auto e =
            std::find_if(elements_.begin(), elements_.end(), [&](const auto& e) { return e.id == id; });
        if (e == elements_.end())
            throw std::runtime_error("Missing location hint widget: " + id);
        return *e;
    };
    std::vector<std::vector<LocationHintChoice>> groups;
    for (const auto& choice : choices) {
        auto group = std::find_if(groups.begin(), groups.end(),
                                  [&](const auto& g) { return g.front().group == choice.group; });
        if (group == groups.end())
            groups.push_back({choice});
        else
            group->push_back(choice);
    }
    float y = firstY_;
    // Native HUD has three objectives and three hint slots per objective.
    for (unsigned g = 0; g < 3; ++g) {
        const auto objective = "label_hint_objective_" + std::to_string(g + 1);
        if (g >= groups.size()) {
            descendants(objective, [](auto& e) { e.visible = false; });
            continue;
        }
        auto& label = find(objective);
        const float dy = label.parentY + y - label.y;
        descendants(objective, [&](auto& e) { e.y += dy; });
        label.text = groups[g].front().groupText;
        for (unsigned h = 0; h < 3; ++h) {
            const auto suffix = std::to_string(g + 1) + '_' + std::to_string(h + 1),
                       button = "button_hint_" + suffix;
            if (h >= groups[g].size()) {
                descendants(button, [](auto& e) { e.visible = false; });
                continue;
            }
            find(button).enabled = affordable;
            find("label_hint_" + suffix).text = groups[g][h].title;
            choices_[button] = groups[g][h].key;
            y += stepY_;
        }
        y += groupStepY_;
    }
    find("label_hint_list_title").visible = !choices.empty() && affordable;
    find("label_hint_list_title_cant_afford").visible = !choices.empty() && !affordable;
    find("label_hint_selection_close").visible = !choices.empty() && affordable;
    find("label_hint_selection_close_cant_afford").visible = !choices.empty() && !affordable;
    for (auto& e : elements_) {
        const bool select =
            std::find(e.ancestors.begin(), e.ancestors.end(), "select_hint") != e.ancestors.end();
        if (e.id == "button_view_activated_hints" ||
            std::any_of(e.ancestors.begin(), e.ancestors.end(),
                        [](const auto& a) { return a == "button_view_activated_hints"; }))
            e.visible = e.visible && hasHistory;
        if (select && e.visible && (e.id == "button_view_activated_hints" || e.id == "button_hints_close")) {
            const auto id = e.id;
            const float dy = y - (e.y - e.parentY);
            // Only move descendants in the same visible group (IDs are reused).
            const float originalY = e.y;
            for (auto& child : elements_)
                if (&child != &e && child.visible &&
                    std::find(child.ancestors.begin(), child.ancestors.end(), id) != child.ancestors.end())
                    child.y += dy;
            e.y = originalY + dy;
        }
    }
    find("select_hint_bg").y = y + bottom_ - backHeight_;
    lastChoices_ = choices;
    affordable_ = affordable;
    hasHistory_ = hasHistory;
    cached_ = true;
}
std::vector<GuiElement> LocationHintMenu::elements(const std::string& hovered) const {
    auto result = elements_;
    for (const auto& button : elements_)
        if (button.id == hovered && button.visible && button.enabled)
            for (auto& e : result)
                if (auto color = button.hoverTextColors.find(e.id); color != button.hoverTextColors.end())
                    e.color = color->second;
    return result;
}
} // namespace deadtime
