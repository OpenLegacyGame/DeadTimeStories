#include "spellmixing.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <set>
#include <stdexcept>

namespace deadtime {
void MixingGesture::begin(const std::string& direction, unsigned angle) {
    if ((direction != "cw" && direction != "ccw") || angle >= 360)
        throw std::runtime_error("Invalid mixing gesture start");
    clockwise_ = direction == "cw";
    first_ = previous_ = angle / 45;
    started_ = true;
    result_ = Result::Active;
}
MixingGesture::Result MixingGesture::move(unsigned angle) {
    if (!started_ || angle >= 360)
        throw std::runtime_error("Invalid mixing gesture sample");
    if (result_ != Result::Active)
        return result_;
    const auto sector = angle / 45;
    if (sector == previous_)
        return result_;
    const auto next = clockwise_ ? (previous_ + 1) % 8 : (previous_ + 7) % 8;
    if (sector != next)
        return result_ = Result::Failure;
    previous_ = sector;
    if (sector == first_)
        result_ = Result::Success;
    return result_;
}
SpellMixing::SpellMixing(const Assets& assets, const std::string& path, unsigned seed,
                         std::string finalInventory)
    : finalInventory_(std::move(finalInventory)), random_(seed) {
    const auto base = normalize(path);
    const auto bytes = assets.read(base + "/_config.xml");
    pugi::xml_document doc;
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid spellmixing configuration: " + base);
    const auto root = doc.child("spellmixing");
    if (!root)
        throw std::runtime_error("Unsupported spellmixing schema: " + base);
    auto script = [&](pugi::xml_attribute attribute) {
        const std::string value = attribute.value();
        return value.empty() ? std::string{} : resolve(base, value) + ".lua";
    };
    onFinish = script(root.child("onFinish").attribute("script"));
    layout = base + "/layout.xml";
    if (onFinish.empty() || !assets.contains(onFinish) || !assets.contains(layout))
        throw std::runtime_error("Missing spellmixing layout or finish script: " + base);
    size_t bottles = 0;
    for (auto node : root.child("spell_items").children()) {
        const std::string kind = node.name();
        if (kind != "ingredient" && kind != "bottle")
            throw std::runtime_error("Unknown spellmixing item type: " + kind);
        MixingIngredient item;
        item.inventory = node.attribute("inventory").value();
        item.object = node.attribute("in_loa").value();
        item.bottle = kind == "bottle";
        item.fromTable = node.attribute("from_table").as_bool();
        item.pivotX = node.attribute("loa_pivot_x").as_float();
        item.pivotY = node.attribute("loa_pivot_y").as_float();
        item.addScript = script(node.attribute("add_script"));
        item.mixScript = script(node.attribute("mix_script"));
        item.cursorObject = node.attribute("cursor_object_after_loa").value();
        if (item.inventory.empty() || item.object.empty() ||
            !ingredients_.emplace(item.inventory, item).second)
            throw std::runtime_error("Invalid or duplicate spellmixing ingredient");
        for (const auto& hook : {item.addScript, item.mixScript})
            if (!hook.empty() && !assets.contains(hook))
                throw std::runtime_error("Missing spellmixing ingredient script: " + hook);
        bottles += item.bottle;
    }
    for (auto node : root.child("scene_objects").children()) {
        const std::string kind = node.name(), name = node.attribute("name").value();
        if (name.empty())
            throw std::runtime_error("Empty spellmixing scene object");
        if (kind == "loa_ingredient_slot")
            slots_.push_back(name);
        else if (kind == "loa_bottle_slot")
            bottleSlot = name;
        else
            sceneObjects_[kind] = name;
    }
    if (bottles != 1 || slots_.empty() || slots_.size() + 1 != ingredients_.size() || bottleSlot.empty())
        throw std::runtime_error("Invalid spellmixing ingredient slot count");
    std::set<std::string> uniqueSlots(slots_.begin(), slots_.end());
    if (uniqueSlots.size() != slots_.size())
        throw std::runtime_error("Duplicate spellmixing slot");
    for (auto node : root.child("rounds").children("round")) {
        MixingRound round;
        std::set<std::string> used;
        for (auto ingredient : node.child("ingredients").children()) {
            const std::string id = ingredient.name();
            const auto found = ingredients_.find(id);
            if (found == ingredients_.end() || found->second.bottle || !used.insert(id).second)
                throw std::runtime_error("Invalid spellmixing round ingredient: " + id);
            round.ingredients.push_back(id);
        }
        for (auto gesture : node.child("gestures").children()) {
            const std::string direction = gesture.name();
            if (direction != "cw" && direction != "ccw")
                throw std::runtime_error("Invalid spellmixing gesture: " + direction);
            round.gestures.push_back(direction);
        }
        if (round.ingredients.empty() || round.gestures.empty())
            throw std::runtime_error("Empty spellmixing round");
        rounds_.push_back(std::move(round));
    }
    if (rounds_.empty())
        throw std::runtime_error("Missing spellmixing rounds");
    const auto reset = root.child("need_reset");
    resetWrongOrder = reset.attribute("wrong_ingredient_order").as_bool();
    resetWrongMixing = reset.attribute("wrong_mixing").as_bool();
    const auto mixing = root.child("mixing"), mortar = mixing.child("mortar");
    mortarLeft = mortar.attribute("left").as_float();
    mortarTop = mortar.attribute("top").as_float();
    mortarRight = mortar.attribute("right").as_float();
    mortarBottom = mortar.attribute("bottom").as_float();
    pestleX = mixing.child("pestle_start_pos").attribute("x").as_float();
    pestleY = mixing.child("pestle_start_pos").attribute("y").as_float();
    firstFailureSeconds = mixing.child("min_time_before_first_failure").attribute("ms").as_double() / 1000.;
    switchedFailureSeconds =
        mixing.child("min_failure_time_after_direction_switch").attribute("ms").as_double() / 1000.;
    hintFont = resolve(base, mixing.child("hint_text").attribute("font").value());
    hintColor =
        static_cast<uint32_t>(std::stoul(mixing.child("hint_text").attribute("color").value(), nullptr, 16));
    hintClockwiseX = mixing.child("hint_text_cw_offset").attribute("x").as_float();
    hintClockwiseY = mixing.child("hint_text_cw_offset").attribute("y").as_float();
    hintCounterclockwiseX = mixing.child("hint_text_ccw_offset").attribute("x").as_float();
    hintCounterclockwiseY = mixing.child("hint_text_ccw_offset").attribute("y").as_float();
    if (mortarRight <= mortarLeft || mortarBottom <= mortarTop || !std::isfinite(firstFailureSeconds) ||
        !std::isfinite(switchedFailureSeconds) || firstFailureSeconds < 0 || switchedFailureSeconds < 0)
        throw std::runtime_error("Invalid spellmixing gesture region or timing");
    objects_ = loadActivityLayout(assets, layout);
    const auto layoutBytes = assets.read(layout);
    pugi::xml_document scene;
    if (!scene.load_buffer(layoutBytes.data(), layoutBytes.size()))
        throw std::runtime_error("Invalid spellmixing layout");
    std::set<std::string> names;
    for (auto object : scene.child("objects").children("object"))
        names.insert(object.attribute("name").value());
    for (const auto& slot : slots_)
        if (!names.contains(slot))
            throw std::runtime_error("Missing ingredient slot: " + slot);
    if (!names.contains(bottleSlot))
        throw std::runtime_error("Missing bottle slot");
    for (const auto& [_, item] : ingredients_)
        if (!names.contains(item.object))
            throw std::runtime_error("Missing ingredient scene object: " + item.object);
}
bool SpellMixing::place(const std::string& inventory) {
    const auto found = ingredients_.find(inventory);
    if (found == ingredients_.end())
        return false;
    if (found->second.bottle) {
        if (bottlePlaced_)
            return false;
        bottlePlaced_ = true;
        object(found->second.object).visible = true;
        updateVisuals();
        return true;
    }
    if (placement_.contains(inventory))
        return false;
    std::vector<size_t> available;
    for (size_t slot = 0; slot < slots_.size(); ++slot)
        if (std::none_of(placement_.begin(), placement_.end(),
                         [&](const auto& placed) { return placed.second == slot; }))
            available.push_back(slot);
    if (available.empty())
        return false;
    placement_[inventory] =
        available[std::uniform_int_distribution<size_t>(0, available.size() - 1)(random_)];
    auto& sprite = object(found->second.object);
    const auto& slot = object(slots_[placement_.at(inventory)]);
    sprite.x = slot.x + slot.width / 2 - found->second.pivotX;
    sprite.y = slot.y + slot.height / 2 - found->second.pivotY;
    sprite.z = slot.z - 0.01f;
    sprite.visible = true;
    if (found->second.fromTable) {
        const auto source = inventory + "_big";
        for (auto& object : objects_)
            if (object.id == source)
                object.visible = false;
    }
    updateVisuals();
    return true;
}
ActivityObject& SpellMixing::object(const std::string& name) {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == name; });
    if (found == objects_.end())
        throw std::runtime_error("Missing spellmixing object: " + name);
    return *found;
}
void SpellMixing::updateVisuals() {
    auto visible = [&](const char* role, bool value) {
        const auto found = sceneObjects_.find(role);
        if (found != sceneObjects_.end())
            object(found->second).visible = value;
    };
    visible("mortar", (mortar_.empty() && !mixtureReady_) || bottleFilled_);
    visible("mortar_with_ingredients", !mortar_.empty() && !readyToStir() && !mixtureReady_);
    visible("mortar_full", readyToStir());
    visible("mortar_mixture_ready", mixtureReady_ && !bottleFilled_);
    visible("pestle_outside_mortar", !readyToStir() && !mixtureReady_);
    visible("pestle_in_mortar", readyToStir());
    visible("full_bottle", bottleFilled_ && !complete_);
    for (const auto& [_, ingredient] : ingredients_)
        if (ingredient.bottle)
            object(ingredient.object).visible = bottlePlaced_ && !bottleFilled_;
    visible("hint_back", readyToStir());
    for (const auto* role : {"hint_cw", "hint_ccw"}) {
        const auto found = sceneObjects_.find(role);
        if (found == sceneObjects_.end())
            continue;
        auto& arrow = object(found->second);
        arrow.enabled = arrow.visible =
            readyToStir() && direction() == (std::string(role) == "hint_cw" ? "cw" : "ccw");
    }
    if (readyToStir() && !stirring_) {
        auto& pestle = object(sceneObjects_.at("pestle_in_mortar"));
        pestle.x = pestleX;
        pestle.y = pestleY;
    }
}
bool SpellMixing::putInMortar(const std::string& inventory) {
    if (!arranged() || mixtureReady_ || mortar_.contains(inventory))
        return false;
    const auto& order = rounds_[round_].ingredients;
    const auto expected =
        std::find_if(order.begin(), order.end(), [&](const auto& item) { return !mortar_.contains(item); });
    if (expected == order.end() || *expected != inventory) {
        if (resetWrongOrder) {
            for (const auto& item : order) {
                mortar_.erase(item);
                object(ingredients_.at(item).object).visible = true;
            }
            updateVisuals();
        }
        return false;
    }
    mortar_.insert(inventory);
    object(ingredients_.at(inventory).object).visible = false;
    updateVisuals();
    return true;
}
bool SpellMixing::readyToStir() const {
    return !mixtureReady_ &&
           std::all_of(rounds_[round_].ingredients.begin(), rounds_[round_].ingredients.end(),
                       [&](const auto& item) { return mortar_.contains(item); });
}
void SpellMixing::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid mixing time");
    if (readyToStir())
        mixingAge_ += seconds;
    switchGrace_ = std::max(0., switchGrace_ - seconds);
}
const std::string& SpellMixing::direction() const {
    return rounds_.at(round_).gestures.at(gestureIndex_);
}
size_t SpellMixing::remainingGestures() const {
    if (!readyToStir())
        return 0;
    const auto& gestures = rounds_[round_].gestures;
    size_t end = gestureIndex_;
    while (end < gestures.size() && gestures[end] == direction())
        ++end;
    return end - gestureIndex_;
}
bool SpellMixing::startStir(unsigned angle) {
    if (!readyToStir())
        return false;
    gesture_.begin(direction(), angle);
    stirring_ = true;
    return true;
}
MixingGesture::Result SpellMixing::moveStir(unsigned angle) {
    if (!stirring_)
        return MixingGesture::Result::Active;
    const auto result = gesture_.move(angle);
    if (result == MixingGesture::Result::Failure) {
        if (mixingAge_ >= firstFailureSeconds && switchGrace_ == 0) {
            feedback = "wrong_mixing";
            mixingAge_ = 0;
            if (resetWrongMixing) {
                for (const auto& item : rounds_[round_].ingredients) {
                    mortar_.erase(item);
                    object(ingredients_.at(item).object).visible = true;
                }
                gestureIndex_ = 0;
                stirring_ = false;
                updateVisuals();
                return result;
            }
        }
        gesture_.begin(direction(), angle);
    } else if (result == MixingGesture::Result::Success) {
        const auto previous = direction();
        if (++gestureIndex_ == rounds_[round_].gestures.size()) {
            stirring_ = false;
            if (++round_ == rounds_.size()) {
                --round_;
                mixtureReady_ = true;
            } else
                gestureIndex_ = 0;
            updateVisuals();
        } else {
            if (direction() != previous)
                switchGrace_ = switchedFailureSeconds;
            gesture_.begin(direction(), angle);
            updateVisuals();
        }
    }
    return result;
}
void SpellMixing::releaseStir() {
    if (stirring_ && mixingAge_ >= firstFailureSeconds && switchGrace_ == 0) {
        feedback = "wrong_mixing";
        mixingAge_ = 0;
    }
    stirring_ = false;
}
bool SpellMixing::fillBottle() {
    if (!mixtureReady_ || !bottlePlaced_ || bottleFilled_)
        return false;
    bottleFilled_ = true;
    updateVisuals();
    return true;
}
bool SpellMixing::finish() {
    if (!bottleFilled_ || complete_)
        return false;
    complete_ = true;
    updateVisuals();
    return true;
}
bool SpellMixing::skip(const std::function<void(const MixingIngredient&, bool)>& hook) {
    if (complete_)
        return false;
    // Native 4847A0 arranges and consumes outstanding ingredients, runs add
    // hooks, then consumes the container and runs remaining mix hooks once.
    for (const auto& [id, item] : ingredients_)
        if (!item.bottle && !placement_.contains(id)) {
            place(id);
            hook(item, false);
        }
    for (const auto& [id, item] : ingredients_)
        if (item.bottle && !bottlePlaced_) {
            place(id);
            hook(item, false);
        }
    for (const auto& round : rounds_)
        for (const auto& id : round.ingredients)
            if (mortar_.insert(id).second) {
                object(ingredients_.at(id).object).visible = false;
                hook(ingredients_.at(id), true);
            }
    round_ = rounds_.size() - 1;
    gestureIndex_ = rounds_.back().gestures.size();
    stirring_ = false;
    mixtureReady_ = bottleFilled_ = complete_ = true;
    mixingAge_ = switchGrace_ = 0;
    feedback.clear();
    updateVisuals();
    return true;
}
bool SpellMixing::arranged() const {
    return bottlePlaced_ && placement_.size() == slots_.size();
}
std::optional<size_t> SpellMixing::slot(const std::string& inventory) const {
    const auto found = placement_.find(inventory);
    return found == placement_.end() ? std::nullopt : std::optional<size_t>{found->second};
}
} // namespace deadtime
