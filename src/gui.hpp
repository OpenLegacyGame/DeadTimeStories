#pragma once
#include "assets.hpp"
#include <map>
#include <string>
#include <vector>

namespace deadtime {
struct GuiElement {
    std::string kind, id, normal, hover, clicked, mask, text, font;
    std::string disabled;
    std::string shortcuts;
    std::vector<std::string> ancestors;
    std::map<std::string, unsigned> hoverTextColors;
    std::map<std::string, std::string> hoverTextures;
    float parentX = 0, parentY = 0;
    bool baseline = false;
    bool enabled = true;
    std::string normalChecked, hoverChecked, clickedChecked;
    std::string horizontalAlign = "center", verticalAlign = "center";
    float x = 0, y = 0, width = 0, height = 0;
    float slideStartX = 0, slideStartY = 0, slideEndX = 0, slideEndY = 0;
    bool visible = true, transparent = false;
    float alpha = 1;
    std::map<double, float> alphaKeys;
    unsigned color = 0xFFFFFFFF;
    std::map<float, std::string> progress;
};
// Reads visual hierarchy, includes and inherited visual properties without
// embedding copies of the original game's GUI definitions in the source tree.
std::vector<GuiElement> loadGui(Assets& assets, const std::string& path,
                                const std::map<std::string, bool>& visibility = {});
} // namespace deadtime
