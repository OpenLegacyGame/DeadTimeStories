#pragma once
#include "assets.hpp"

namespace deadtime {
struct ActivityObject {
    std::string id, texture;
    float x = 0, y = 0, z = 0, width = 0, height = 0, angle = 0, alpha = 1;
    float scale = 1, pivotX = 0, pivotY = 0;
    bool additive = false;
    bool repeatX = false;
    bool visible = true, interactive = true, enabled = true;
    bool contains(float mouseX, float mouseY) const;
};
std::vector<ActivityObject> loadActivityLayout(const Assets& assets, const std::string& path);
// Fill unspecified dimensions from static JPEG texture headers, without
// introducing the presentation layer's pixel decoder into gameplay models.
void measureActivityTextures(const Assets& assets, std::vector<ActivityObject>& objects);
} // namespace deadtime
