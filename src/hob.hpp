#pragma once
#include "assets.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace deadtime {
struct HobPiece {
    std::string texture;
    std::string inventory, inventoryMessage;
    bool dropInventory = false, removeInventory = false;
    float x = 0, y = 0, z = 0, width = 0, height = 0;
    float alpha = 1;
    bool visible = true, interactive = true;
    bool found = false;
};
struct HobItem {
    std::string name;
    std::vector<HobPiece> pieces;
    bool target = false, environment = true;
    bool found() const;
    size_t remaining() const;
    std::string listText() const;
};
class HiddenObjectRound {
  public:
    HiddenObjectRound(Assets& assets, const std::string& path, const std::string& round, uint32_t seed);
    bool pick(size_t item, size_t piece, const std::string& inventory = {});
    bool complete() const;
    std::vector<HobItem> items;
    std::vector<HobPiece> background;
    std::vector<size_t> targets;
    std::string enterScript, completeScript, congratulations, rewardImage;
    bool rewarded = false;
};
} // namespace deadtime
