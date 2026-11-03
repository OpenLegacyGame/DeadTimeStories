#include "apothecary.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <set>
#include <stdexcept>

namespace deadtime {
bool Apothecary::skip() {
    if (solved_)
        return false;
    // Native 506050 sends Solve/Reinit, leaving the reward for explicit pickup.
    events_.clear();
    phase_ = Phase::Idle;
    age_ = 0;
    selected_.reset();
    sheetOpen_ = false;
    for (size_t i = 0; i < slots_.size(); ++i)
        slots_[i] = i;
    checkSolved();
    updateVisuals();
    return true;
}
namespace {
std::string numbered(const std::string& prefix, size_t index) {
    return prefix + (index < 9 ? "0" : "") + std::to_string(index + 1);
}
} // namespace
Apothecary::~Apothecary() = default;
Apothecary::Apothecary(const Assets& assets, const std::string& base, uint32_t seed)
    : base_(normalize(base)) {
    pugi::xml_document doc;
    const auto config = assets.read(base_ + "/_config.xml");
    if (!doc.load_buffer(config.data(), config.size()) || !doc.child("sort_apothecary_items"))
        throw std::runtime_error("Invalid apothecary configuration");
    const auto root = doc.child("sort_apothecary_items");
    std::set<std::string> ids, inventory;
    for (auto node : root.child("items").children("item")) {
        ApothecaryItem item{node.attribute("id").value(), node.attribute("tooltip").value(),
                            node.attribute("inventory_item").value(), node.attribute("reward").as_bool()};
        if (item.id.empty() || !ids.insert(item.id).second ||
            (!item.inventory.empty() && !inventory.insert(item.inventory).second) ||
            (item.reward && item.inventory.empty()))
            throw std::runtime_error("Invalid apothecary item");
        slots_.push_back(item.inventory.empty() || item.reward ? std::optional<size_t>(items_.size())
                                                               : std::nullopt);
        items_.push_back(std::move(item));
    }
    if (items_.size() < 2 || items_.size() > 99 ||
        items_.size() != root.child("shelves").attribute("quantity").as_uint())
        throw std::runtime_error("Invalid apothecary shelf/item count");
    std::set<size_t> fixed;
    for (auto node : root.child("locked_items").children("item")) {
        const auto index = node.attribute("index").as_uint();
        if (index >= items_.size() || !slots_[index])
            throw std::runtime_error("Invalid apothecary fixed slot");
        fixed.insert(index);
    }
    const size_t correct = fixed.size() + root.child("num_random_locked_items").attribute("value").as_uint();
    if (correct > items_.size() - missingCount() || correct >= items_.size() ||
        items_.size() - fixed.size() < 2)
        throw std::runtime_error("Invalid apothecary locked count");
    ParticleRandom random(seed);
    auto exchange = [&] {
        for (unsigned tries = 0; tries < 100000; ++tries) {
            const size_t a = random.next() % slots_.size(), b = random.next() % slots_.size();
            if (a != b && !fixed.contains(a) && !fixed.contains(b)) {
                std::swap(slots_[a], slots_[b]);
                return;
            }
        }
        throw std::runtime_error("Cannot shuffle apothecary shelves");
    };
    for (unsigned i = 0; i < 500; ++i)
        exchange();
    for (unsigned i = 0; i < 1000000 && correctCount() != correct; ++i)
        exchange();
    if (correctCount() != correct)
        throw std::runtime_error("Cannot generate apothecary locked count");
    swapSeconds_ = root.child("swap_items_time").attribute("seconds").as_double();
    if (!std::isfinite(swapSeconds_) || swapSeconds_ <= 0)
        throw std::runtime_error("Invalid apothecary swap duration");
    riddleFont_ = resolve(base_, root.child("riddle").attribute("font").value());
    riddleColor_ =
        unsigned(std::stoul(root.child("riddle").attribute("color").as_string("FF000000"), nullptr, 16));
    auto hook = [&](const char* name) {
        const std::string value = root.child(name).attribute("script").value();
        if (value.empty())
            return std::string{};
        const auto path = resolve(base_, value) + ".lua";
        if (!assets.contains(path))
            throw std::runtime_error("Missing apothecary callback: " + path);
        return path;
    };
    onEnter = hook("onEnter");
    onExit = hook("onExit");
    onSolve = hook("onSolve");
    onFinish = hook("onFinish");
    objects_ = loadActivityLayout(assets, base_ + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (const auto* id : {"back", "exit", "half_dark", "sheet", "sheet_zoomed"})
        object(id);
    std::vector<ActivityObject> variants;
    for (size_t i = 0; i < items_.size(); ++i) {
        shelf(i);
        object(numbered("shelf_", i) + "_item_pos");
        const auto& sprite = item(i);
        normalSizes_.push_back({sprite.width, sprite.height});
        ActivityObject variant;
        variant.texture = base_ + "/items/" + items_[i].id + "_inplace";
        variants.push_back(variant);
    }
    measureActivityTextures(assets, variants);
    for (const auto& variant : variants)
        lockedSizes_.push_back({variant.width, variant.height});
    const auto bytes = assets.read(base_ + "/layout.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid apothecary layout");
    for (auto node : doc.child("objects").children("object"))
        if (node.child("particles")) {
            const std::string id = node.attribute("name").value();
            const auto& sprite = object(id);
            auto effect = std::make_unique<Object>();
            effect->id = id;
            effect->x = sprite.x;
            effect->y = sprite.y;
            effect->z = sprite.z;
            effect->alpha = sprite.alpha;
            effect->visible = sprite.visible;
            effect->enabled = false;
            for (auto graph : node.children("particles"))
                effect->particleResources.push_back(
                    {resolve(base_, graph.attribute("src").value()), graph.attribute("presim").as_double()});
            effects_.emplace(id, std::move(effect));
        }
    updateVisuals();
}
const ActivityObject& Apothecary::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing apothecary object: " + id);
    return *found;
}
ActivityObject& Apothecary::mutableObject(const std::string& id) {
    return const_cast<ActivityObject&>(object(id));
}
const ActivityObject& Apothecary::item(size_t index) const {
    return object(numbered("item_", index));
}
const ActivityObject& Apothecary::shelf(size_t index) const {
    return object(numbered("shelf_", index));
}
const Object* Apothecary::effect(const std::string& id) const {
    const auto found = effects_.find(id);
    return found == effects_.end() ? nullptr : found->second.get();
}
bool Apothecary::locked(size_t slot) const {
    return slot < slots_.size() && slots_[slot] && *slots_[slot] == slot;
}
size_t Apothecary::correctCount() const {
    size_t count = 0;
    for (size_t i = 0; i < slots_.size(); ++i)
        count += locked(i);
    return count;
}
size_t Apothecary::missingCount() const {
    return std::count(slots_.begin(), slots_.end(), std::nullopt);
}
void Apothecary::sound(const std::string& path) {
    events_.push_back({ApothecaryEvent::Sound, path});
}
void Apothecary::hint(const std::string& id, bool reactive) {
    events_.push_back({reactive ? ApothecaryEvent::PopupHint : ApothecaryEvent::Hint,
                       "puzzles/sort_apothecary_items/hints/" + id});
}
void Apothecary::enter() {
    phase_ = Phase::Idle;
    age_ = 0;
    sheetOpen_ = false;
    selected_.reset();
    events_.clear();
    checkSolved();
    updateVisuals();
}
void Apothecary::leave() {
    phase_ = Phase::Idle;
    age_ = 0;
    sheetOpen_ = false;
    selected_.reset();
    updateVisuals();
}
void Apothecary::toggleSheet() {
    if (!busy()) {
        sheetOpen_ = !sheetOpen_;
        updateVisuals();
    }
}
void Apothecary::cancel() {
    if (!busy()) {
        selected_.reset();
        updateVisuals();
    }
}
void Apothecary::idleClick() {
    if (!busy()) {
        cancel();
        sound("sound/sfx/sfx_m_click_idle");
    }
}
bool Apothecary::place(size_t slot, const std::string& inventory) {
    if (busy() || sheetOpen_ || solved_ || slot >= slots_.size())
        return false;
    const auto found = std::find_if(items_.begin(), items_.end(), [&](const auto& item) {
        return !item.reward && item.inventory == inventory && !inventory.empty();
    });
    if (found == items_.end()) {
        sound("sound/sfx/sfx_m_click_idle");
        hint(inventory == "baking_soda" || inventory == "aspirin" ? "wrong_drug" : "wrong_item", false);
        return false;
    }
    const size_t index = size_t(found - items_.begin());
    if (slots_[slot] || std::find(slots_.begin(), slots_.end(), std::optional<size_t>(index)) != slots_.end())
        return false;
    slots_[slot] = index;
    selected_.reset();
    if (!missingCount())
        hint("all_items_on_shelves");
    updateVisuals();
    checkSolved();
    return true;
}
bool Apothecary::select(size_t slot) {
    if (busy() || sheetOpen_ || slot >= slots_.size())
        return false;
    sound("sound/sfx/sfx_m_click_idle");
    if (solved_) {
        if (!slots_[slot])
            return false;
        const auto& reward = items_[*slots_[slot]];
        if (!reward.reward) {
            hint("wrong_reward");
            return false;
        }
        events_.push_back({ApothecaryEvent::Award, reward.inventory});
        slots_[slot].reset();
        const bool remaining = std::any_of(slots_.begin(), slots_.end(),
                                           [&](const auto& value) { return value && items_[*value].reward; });
        if (!remaining && !complete_) {
            complete_ = true;
            events_.push_back({ApothecaryEvent::Finish, {}});
            hint("reward_taken");
        }
        updateVisuals();
        return true;
    }
    if (missingCount()) {
        if (missingCount() <= 3)
            hint(missingCount() == 3   ? "three_items_missing"
                 : missingCount() == 2 ? "two_items_missing"
                                       : "one_item_missing");
        return false;
    }
    if (locked(slot)) {
        hint("item_in_place");
        return false;
    }
    if (selected_ == slot) {
        cancel();
        return true;
    }
    if (!selected_) {
        if (!slots_[slot])
            return false;
        selected_ = slot;
        updateVisuals();
        return true;
    }
    first_ = *selected_;
    second_ = slot;
    selected_.reset();
    age_ = 0;
    phase_ = Phase::FadeOut;
    swapCorrect_ = false;
    updateVisuals();
    return true;
}
void Apothecary::checkSolved() {
    if (!solved_ && correctCount() == slots_.size()) {
        solved_ = true;
        sound("sound/sfx/mg/mg_finish");
        events_.push_back({ApothecaryEvent::Solve, {}});
    }
}
void Apothecary::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid apothecary time advance");
    while (busy()) {
        const double half = swapSeconds_ / 2, step = std::min(seconds, std::max(0., half - age_));
        age_ += step;
        seconds -= step;
        if (age_ + 1e-10 < half)
            break;
        age_ = 0;
        if (phase_ == Phase::FadeOut) {
            std::swap(slots_[first_], slots_[second_]);
            swapCorrect_ = locked(first_) || locked(second_);
            phase_ = Phase::FadeIn;
        } else {
            phase_ = Phase::Idle;
            sound(std::string("sound/sfx/mg/sort_apothecary_items/") +
                  (swapCorrect_ ? "mg_correct_place" : "mg_replace2"));
            checkSolved();
        }
        if (seconds <= 0)
            break;
    }
    updateVisuals();
}
void Apothecary::updateVisuals() {
    for (auto& [id, effect] : effects_)
        effect->enabled = false;
    for (size_t i = 0; i < items_.size(); ++i)
        mutableObject(numbered("item_", i)).visible = false;
    for (size_t slot = 0; slot < slots_.size(); ++slot)
        if (slots_[slot]) {
            const size_t index = *slots_[slot];
            auto& sprite = mutableObject(numbered("item_", index));
            const auto& marker = object(numbered("shelf_", slot) + "_item_pos");
            sprite.x = marker.x;
            sprite.y = marker.y;
            sprite.z = marker.z;
            sprite.visible = true;
            sprite.alpha = 1;
            const bool correct = locked(slot);
            const auto size = correct ? lockedSizes_[index] : normalSizes_[index];
            sprite.texture = base_ + "/items/" + items_[index].id + (correct ? "_inplace" : "");
            sprite.width = size.first;
            sprite.height = size.second;
            if (busy() && (slot == first_ || slot == second_)) {
                const float t = std::clamp(float(age_ / (swapSeconds_ / 2)), 0.f, 1.f);
                sprite.alpha = phase_ == Phase::FadeOut ? 1 - t : t;
            }
            const std::string effectID = selected_ == slot                 ? "fx_item_selected"
                                         : solved_ && items_[index].reward ? "fx_item_reward"
                                                                           : "";
            if (auto found = effects_.find(effectID); found != effects_.end()) {
                auto& effect = *found->second;
                effect.enabled = true;
                effect.x = sprite.x + sprite.width / 2 - sprite.pivotX;
                effect.y = sprite.y + sprite.height / 2 - sprite.pivotY;
                effect.z = sprite.z + (effectID == "fx_item_selected" ? -1 : 1);
            }
        }
    mutableObject("sheet_zoomed").visible = sheetOpen_;
    mutableObject("sheet_zoomed").interactive = false;
    mutableObject("half_dark").visible = sheetOpen_;
}
std::vector<ApothecaryEvent> Apothecary::takeEvents() {
    auto result = std::move(events_);
    events_.clear();
    return result;
}
} // namespace deadtime
