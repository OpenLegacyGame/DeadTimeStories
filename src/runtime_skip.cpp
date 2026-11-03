#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>

namespace deadtime {
void Runtime::loadPuzzleSkip() {
    if (!assets_.contains("deadtime.game"))
        return;
    const auto bytes = assets_.read("deadtime.game");
    pugi::xml_document doc;
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid game configuration");
    const auto skip = doc.child("deadtime").child("skip_puzzle");
    if (!skip)
        return;
    puzzleSkipSeconds_ = skip.attribute("timerSeconds").as_double();
    if (!std::isfinite(puzzleSkipSeconds_) || puzzleSkipSeconds_ <= 0)
        throw std::runtime_error("Invalid puzzle skip time");
}
std::string Runtime::skippablePuzzle() const {
    if (layers_.empty() || cutsceneActive_ || carriageActive_ || !activeHob_.empty())
        return {};
    const auto found = [&](const auto& values, const auto& item) {
        return std::all_of(values.begin(), values.end(),
                           [&](const auto& value) { return wasFound(item(value)); });
    };
    if (ouijiActive_ && !ouiji_->solved())
        return "ouiji";
    if (hexBonesActive_ && hexBones_->skippable())
        return "bones";
    if (pianoActive_ && !piano_->opened())
        return "piano";
    if (ratsActive_ && !rats_->complete())
        return "rats";
    if (cemeteryGateActive_ && !cemeteryGate_->complete())
        return "gate";
    if (skullsObeyActive_ && !skullsObey_->opened())
        return "skulls";
    if (blockedWellActive_ && !blockedWell_->complete() && blockedWell_->allNamesKnown())
        return "well";
    if (!activeMixing_.empty()) {
        const auto& mix = *mixingGames_.at(activeMixing_);
        if (!mix.complete() && found(mix.skipRequirements, [](const auto& item) { return item; }))
            return "mixing:" + activeMixing_;
    }
    if (voodooActive_ && !voodoo_->complete() &&
        found(voodoo_->ingredients(), [](const auto& item) { return item.inventory; }))
        return "voodoo";
    if (windowCleaningActive_ && !windowCleaning_->complete() && wasFound("sponge") &&
        wasFound("mixture_grease_cleaning"))
        return "window";
    if (doveCoteActive_ && !doveCote_->complete() && wasFound("seeds") && wasFound("rope"))
        return "dove";
    if (cellarDoorsActive_ && !cellarDoors_->complete() && wasFound("ash"))
        return "cellar";
    if (spiritLockActive_ && !spiritLock_->complete() &&
        found(spiritLock_->pieces(), [](const auto& item) { return item; }))
        return "lock";
    if (sailActive_ && !sail_->complete() && wasFound(sail_->sewingItem()) &&
        found(sail_->inventoryPieces(), [](const auto& item) { return item.first; }))
        return "sail";
    if (leechesActive_ && !leeches_->complete() && wasFound("orange_1") && wasFound("orange_2") &&
        wasFound("orange_3"))
        return "leeches";
    if (apothecaryActive_ && !apothecary_->solved() &&
        std::all_of(apothecary_->items().begin(), apothecary_->items().end(), [&](const auto& item) {
            return item.reward || item.inventory.empty() || wasFound(item.inventory);
        }))
        return "apothecary";
    if (voodooCircleActive_ && !voodooCircle_->complete() && !voodooCircle_->busy() &&
        flag("enableSpell:" + voodooCircle_->spell()) &&
        std::all_of(voodooCircle_->items().begin(), voodooCircle_->items().end(),
                    [&](const auto& item) { return item.onScene || wasFound(item.inventory); }))
        return "circle";
    return {};
}
bool Runtime::hasSkipPuzzle(const std::string& id) const {
    return (id == "ouiji" && ouiji_) || (id == "bones" && hexBones_) || (id == "piano" && piano_) ||
           (id == "rats" && rats_) || (id == "gate" && cemeteryGate_) || (id == "skulls" && skullsObey_) ||
           (id == "well" && blockedWell_) ||
           (id.starts_with("mixing:") && mixingGames_.contains(id.substr(7))) ||
           (id == "voodoo" && voodoo_) || (id == "window" && windowCleaning_) ||
           (id == "dove" && doveCote_) || (id == "cellar" && cellarDoors_) || (id == "lock" && spiritLock_) ||
           (id == "sail" && sail_) || (id == "leeches" && leeches_) || (id == "apothecary" && apothecary_) ||
           (id == "circle" && voodooCircle_);
}
double Runtime::puzzleSkipProgress() const {
    const auto found = puzzleSkipProgress_.find(skippablePuzzle());
    return found == puzzleSkipProgress_.end() ? 0 : found->second;
}
bool Runtime::puzzleSkipVisible() const {
    return !locationHintsOpen_ && !skipBookOpen_ && !skippablePuzzle().empty();
}
void Runtime::tickPuzzleSkip(double seconds) {
    const auto key = skippablePuzzle();
    if (key.empty())
        return;
    auto& progress = puzzleSkipProgress_[key];
    if (progress == 1)
        return;
    const double delta = seconds / puzzleSkipSeconds_;
    // Native 53C360 accrues while the HUD is hidden, but postpones ready state
    // and its sound until a visible tick can cross the final fraction.
    if (progress + delta >= 1) {
        if (puzzleSkipVisible()) {
            progress = 1;
            events.push_back({"playSound", {"sound/sfx/sfx_hint_ready"}, "skip_puzzle"});
        } else
            progress = 0.999999;
    } else
        progress += delta;
}
void Runtime::requestPuzzleSkip() {
    if (!puzzleSkipVisible())
        return;
    if (puzzleSkipProgress() < 1) {
        events.push_back({"showHint", {string("dialogs/common/cannot_skip_puzzle_yet")}, "skip_puzzle"});
        events.push_back({"playSound", {"sound/sfx/sfx_reactive_pop_up"}, "skip_puzzle"});
    } else
        events.push_back({"showConfirmationDialog",
                          {string("dialogs/common/skip_puzzle_confirmation"),
                           "if dialogResult == 'yes' then skipMinigame() end"},
                          "skip_puzzle"});
}
bool Runtime::skipMinigame() {
    // Match native Lua 568560: confirmation callbacks cannot bypass eligibility,
    // the hint/book modal gate or the fully charged per-puzzle timer.
    if (!puzzleSkipVisible() || puzzleSkipProgress() != 1)
        return false;
    const auto key = skippablePuzzle();
    bool skipped = false;
    if (key == "ouiji")
        skipped = ouiji_->skip();
    else if (key == "bones")
        skipped = hexBones_->skip();
    else if (key == "piano")
        skipped = piano_->skip();
    else if (key == "rats")
        skipped = rats_->skip();
    else if (key == "gate")
        skipped = cemeteryGate_->skip();
    else if (key == "skulls")
        skipped = skullsObey_->skip();
    else if (key == "well")
        skipped = blockedWell_->skip();
    else if (key.starts_with("mixing:")) {
        auto* mix = mixing();
        skipped = mix->skip([&](const auto& item, bool mixed) {
            if (!mixed)
                consumeInventory(item.inventory);
            runFile(mixed ? item.mixScript : item.addScript);
        });
        if (skipped) {
            consumeInventory(mix->finalInventory());
            closeMixing();
            events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "spellmixing"});
            runFile(mix->onFinish);
        }
    } else if (key == "voodoo") {
        skipped = voodoo_->skip();
        if (skipped) {
            for (const auto& item : voodoo_->ingredients())
                consumeInventory(item.inventory);
            finishVoodoo();
            closeVoodoo();
        }
    } else if (key == "window") {
        skipped = windowCleaning_->skip();
        if (skipped)
            consumeInventory("mixture_grease_cleaning");
    } else if (key == "dove")
        skipped = doveCote_->skip();
    else if (key == "cellar") {
        skipped = cellarDoors_->skip();
        if (skipped)
            consumeInventory("ash");
    } else if (key == "lock") {
        skipped = spiritLock_->skip();
        if (skipped)
            for (const auto& item : spiritLock_->pieces())
                consumeInventory(item);
    } else if (key == "sail") {
        skipped = sail_->skip();
        if (skipped)
            for (const auto& [item, pieces] : sail_->inventoryPieces())
                consumeInventory(item);
    } else if (key == "leeches") {
        skipped = leeches_->skip();
        if (skipped)
            for (const auto* item : {"orange_1", "orange_2", "orange_3"})
                consumeInventory(item);
    } else if (key == "apothecary") {
        skipped = apothecary_->skip();
        if (skipped)
            for (const auto& item : apothecary_->items())
                if (!item.reward)
                    consumeInventory(item.inventory);
    } else if (key == "circle") {
        voodooCircle_->setSpellKnown(true);
        skipped = voodooCircle_->skip();
        if (skipped) {
            for (const auto& item : voodooCircle_->items()) {
                if (!wasFound(item.inventory))
                    grantInventory(item.inventory);
                consumeInventory(item.inventory);
            }
            if (!wasFound("moss"))
                grantInventory("moss");
            consumeInventory("chalk");
        }
    }
    if (skipped) {
        events.push_back({"clearHintBubble", {}, "skip_puzzle"});
        events.push_back({"selectInventoryObject", {""}, "skip_puzzle"});
        tick(0);
        if (key == "window")
            closeWindowCleaning();
    }
    return skipped;
}
} // namespace deadtime
