#pragma once
#include <string_view>
namespace deadtime {
enum class TutorialId {
    FirstItem,
    InventoryPicture,
    InventoryHover,
    InventoryPin,
    FirstHob,
    HobIdle,
    PlantTool,
    LocationHint,
    HintSelection,
    Ouiji,
    SpellBook,
    SpellMixing,
    Carriage,
    NewArea,
    SailPiece,
    NewSpell,
    Count
};
struct TutorialSpec {
    std::string_view key;
    float x, y;
    bool repeat = false, picture = false;
};
const TutorialSpec& tutorialSpec(TutorialId id);
} // namespace deadtime
