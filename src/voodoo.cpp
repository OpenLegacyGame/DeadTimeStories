#include "voodoo.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <pugixml.hpp>
#include <set>
#include <sstream>
#include <stdexcept>

namespace deadtime {
namespace {
std::string numbered(const char* prefix, size_t index) {
    return std::string(prefix) + (index < 10 ? "0" : "") + std::to_string(index);
}
std::vector<std::string> names(const char* text) {
    std::istringstream input(text);
    std::vector<std::string> result;
    for (std::string name; input >> name;)
        result.push_back(name);
    return result;
}
void checkEffects(pugi::xml_node node) {
    // This edition's ingredient and finish effects consist of end_fx only.
    // Reject an unsupported timeline rather than silently skipping its work.
    for (auto action : node.child("fx_actions").children())
        if (std::string(action.name()) != "end_fx")
            throw std::runtime_error("Unsupported voodoo effect action: " + std::string(action.name()));
}
} // namespace
VoodooDoll::VoodooDoll(const Assets& assets, const std::string& path) {
    const auto base = normalize(path), bytes = assets.read(base + "/_config.xml");
    pugi::xml_document doc;
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("voodoo"))
        throw std::runtime_error("Invalid voodoo configuration");
    const auto root = doc.child("voodoo");
    checkEffects(root.child("onFinish"));
    onFinish = resolve(base, root.child("onFinish").attribute("script").value()) + ".lua";
    if (!assets.contains(onFinish))
        throw std::runtime_error("Missing voodoo finish script");
    objects_ = loadActivityLayout(assets, base + "/layout.xml");
    measureActivityTextures(assets, objects_);
    std::set<std::string> inventory;
    for (auto node : root.child("ingredients").children("ingredient")) {
        checkEffects(node);
        VoodooIngredient item;
        item.inventory = node.attribute("inventory_item").value();
        item.central = node.attribute("central").as_bool();
        item.object = numbered("ingredient_", ingredients_.size() + 1);
        item.show = names(node.attribute("show_scene_objects").value());
        item.hide = names(node.attribute("hide_scene_objects").value());
        if (item.inventory.empty() || !inventory.insert(item.inventory).second)
            throw std::runtime_error("Invalid or duplicate voodoo ingredient");
        object(item.object);
        for (const auto& name : item.show)
            object(name);
        for (const auto& name : item.hide)
            object(name);
        ingredients_.push_back(std::move(item));
    }
    if (ingredients_.empty() || !ingredients_.front().central ||
        std::count_if(ingredients_.begin(), ingredients_.end(),
                      [](const auto& item) { return item.central; }) != 1)
        throw std::runtime_error("Voodoo recipe must start with one central ingredient");
    for (size_t i = 1; i < ingredients_.size(); ++i) {
        slots_.push_back(numbered("slot_", i));
        object(slots_.back());
    }
    for (const auto* name : {"slot_central", "riddle", "back", "exit"})
        object(name);
    placement_.resize(ingredients_.size());
    updateVisuals();
}
const ActivityObject& VoodooDoll::object(const std::string& name) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == name; });
    if (found == objects_.end())
        throw std::runtime_error("Missing voodoo scene object: " + name);
    return *found;
}
ActivityObject& VoodooDoll::mutableObject(const std::string& name) {
    return const_cast<ActivityObject&>(object(name));
}
std::optional<size_t> VoodooDoll::ingredient(const std::string& inventory) const {
    for (size_t i = 0; i < ingredients_.size(); ++i)
        if (ingredients_[i].inventory == inventory)
            return i;
    return {};
}
bool VoodooDoll::arranged() const {
    return std::all_of(placement_.begin(), placement_.end(), [](auto slot) { return slot.has_value(); });
}
bool VoodooDoll::place(const std::string& inventory, bool centralTarget, float x, float y) {
    feedback.clear();
    const auto index = ingredient(inventory);
    if (!index) {
        feedback = "spellmixing/common/hints/wrong_ingredient";
        return false;
    }
    if (complete() || arranged() || placement_[*index] || !std::isfinite(x) || !std::isfinite(y))
        return false;
    if (ingredients_[*index].central != centralTarget) {
        feedback = "spellmixing/voodoo_doll/hints/sticks_in_the_middle";
        return false;
    }
    if (centralTarget) {
        placement_[*index] = slots_.size();
        advance();
    } else {
        double distance = std::numeric_limits<double>::infinity();
        std::optional<size_t> chosen;
        for (size_t slot = 0; slot < slots_.size(); ++slot) {
            if (std::any_of(placement_.begin(), placement_.end(),
                            [&](auto placed) { return placed == slot; }))
                continue;
            const auto& target = object(slots_[slot]);
            const double dx = target.x + target.width / 2 - x, dy = target.y + target.height / 2 - y;
            if (dx * dx + dy * dy < distance) {
                distance = dx * dx + dy * dy;
                chosen = slot;
            }
        }
        if (!chosen)
            return false;
        placement_[*index] = chosen;
    }
    updateVisuals();
    if (arranged() && !complete())
        feedback = "spellmixing/voodoo_doll/hints/ready_to_combine";
    return true;
}
void VoodooDoll::advance() {
    const auto& item = ingredients_.at(progress_++);
    for (const auto& name : item.show)
        mutableObject(name).visible = true;
    for (const auto& name : item.hide)
        mutableObject(name).visible = false;
}
bool VoodooDoll::apply(const std::string& inventory) {
    feedback.clear();
    const auto index = ingredient(inventory);
    if (!index || !arranged() || complete() || used(*index))
        return false;
    if (*index != progress_) {
        feedback = "spellmixing/voodoo_doll/hints/wrong_order";
        return false;
    }
    advance();
    updateVisuals();
    return true;
}
void VoodooDoll::updateVisuals() {
    const bool ready = arranged();
    mutableObject("riddle").visible = ready;
    for (size_t i = 0; i < ingredients_.size(); ++i) {
        auto& sprite = mutableObject(ingredients_[i].object);
        sprite.visible = placement_[i].has_value() && !used(i);
        sprite.interactive = ready && sprite.visible;
        if (placement_[i] && !ingredients_[i].central) {
            const auto& slot = object(slots_.at(*placement_[i]));
            sprite.x = slot.x + slot.width / 2;
            sprite.y = slot.y + slot.height / 2;
            sprite.z = slot.z;
        }
    }
}
bool VoodooDoll::skip() {
    if (complete())
        return false;
    // Native 49DA90 consumes outstanding ingredients and finishes the recipe.
    // Keep already assigned slots; assign only missing material positions.
    for (size_t i = 0; i < ingredients_.size(); ++i)
        if (!placement_[i]) {
            if (ingredients_[i].central)
                placement_[i] = slots_.size();
            else
                for (size_t slot = 0; slot < slots_.size(); ++slot)
                    if (std::none_of(placement_.begin(), placement_.end(),
                                     [&](auto used) { return used == slot; })) {
                        placement_[i] = slot;
                        break;
                    }
        }
    while (!complete())
        advance();
    feedback.clear();
    updateVisuals();
    return true;
}
} // namespace deadtime
