#include "sail.hpp"
#include "particles.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <pugixml.hpp>
#include <set>
#include <sstream>
#include <stdexcept>

namespace deadtime {
bool Sail::skip() {
    if (complete_)
        return false;
    // Native 4E88E0 consumes outstanding fabric, retains the sewing kit and
    // emits Finish directly rather than running the timed sewing state.
    phase_ = Phase::Idle;
    sewAge_ = 0;
    events_.clear();
    for (size_t i = 0; i < pieces_.size(); ++i) {
        auto& state = pieces_[i];
        auto& sprite = mutablePiece(i);
        state.placed = true;
        state.turns = 0;
        sprite.x = state.targetX;
        sprite.y = state.targetY;
        sprite.angle = 0;
        fixView(i, false);
    }
    readyAnnounced_ = complete_ = true;
    events_.push_back({SailEvent::Finish, {}});
    updateVisuals();
    return true;
}
Sail::Sail(const Assets& assets, const std::string& base, uint32_t seed) {
    pugi::xml_document doc;
    const auto bytes = assets.read(base + "/config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("sail"))
        throw std::runtime_error("Invalid sail configuration");
    const auto root = doc.child("sail");
    threshold_ = root.child("final_position_threshold").attribute("value").as_float();
    exitY_ = root.child("exit").attribute("y").as_float();
    sewingItem_ = root.child("sew_inventory_object").attribute("name").value();
    if (!std::isfinite(threshold_) || threshold_ <= 0 || !std::isfinite(exitY_) || exitY_ < 0 ||
        exitY_ >= 768 || sewingItem_.empty())
        throw std::runtime_error("Invalid sail threshold, exit or sewing item");
    auto hook = [&](const char* name) {
        const std::string script = root.child(name).attribute("src").value();
        if (script.empty())
            return std::string{};
        const auto path = resolve(base, script) + ".lua";
        if (!assets.contains(path))
            throw std::runtime_error("Missing sail callback: " + path);
        return path;
    };
    onEnter = hook("script_onenter");
    onExit = hook("script_onexit");
    onFinish = hook("script_onfinish");
    ActivityObject background;
    background.id = "back";
    background.texture = resolve(base, root.child("back").attribute("src").value());
    if (background.texture.empty())
        throw std::runtime_error("Missing sail background");
    background.interactive = false;
    objects_.push_back(background);
    ParticleRandom random(seed);
    std::set<std::string> ids;
    std::vector<ActivityObject> fixed;
    for (auto node : root.child("ingame_objects").children()) {
        SailPiece piece;
        piece.id = node.name();
        if (piece.id.empty() || !ids.insert(piece.id).second)
            throw std::runtime_error("Duplicate sail piece");
        piece.targetX = node.child("final_position").attribute("x").as_float();
        piece.targetY = node.child("final_position").attribute("y").as_float();
        if (!std::isfinite(piece.targetX) || !std::isfinite(piece.targetY))
            throw std::runtime_error("Invalid sail target position");
        piece.texture = resolve(base, node.child("texture").attribute("src").value());
        piece.fixedTexture = resolve(base, node.child("texture_fixed").attribute("src").value());
        if (piece.texture.empty() || piece.fixedTexture.empty() || piece.id == "back" ||
            piece.id == "seam_object")
            throw std::runtime_error("Invalid sail piece resources or name");
        piece.turns = random.next() % 4;
        ActivityObject sprite;
        sprite.id = piece.id;
        sprite.texture = piece.texture;
        sprite.z = -1 - float(pieces_.size()) * .01f;
        sprite.pivotX = node.child("pivot").attribute("x").as_float();
        sprite.pivotY = node.child("pivot").attribute("y").as_float();
        sprite.visible = false;
        objects_.push_back(sprite);
        sprite.texture = piece.fixedTexture;
        fixed.push_back(sprite);
        pieces_.push_back(std::move(piece));
    }
    if (pieces_.empty() || pieces_.size() > 100)
        throw std::runtime_error("Invalid sail piece count");
    for (auto node : root.child("inventory_objects").children()) {
        const std::string name = node.name();
        auto& targets = inventory_[name];
        std::istringstream names(node.attribute("transforms_into_ingame").value());
        for (std::string id; names >> id;) {
            const auto found = std::find_if(pieces_.begin(), pieces_.end(),
                                            [&](const auto& piece) { return piece.id == id; });
            if (found == pieces_.end())
                throw std::runtime_error("Unknown sail inventory target: " + id);
            const size_t index = size_t(found - pieces_.begin());
            if (std::find(targets.begin(), targets.end(), index) != targets.end())
                throw std::runtime_error("Duplicate sail inventory target");
            targets.push_back(index);
        }
        if (name.empty() || targets.empty())
            throw std::runtime_error("Empty sail inventory mapping");
    }
    for (size_t i = 0; i < pieces_.size(); ++i) {
        const auto count = std::count_if(inventory_.begin(), inventory_.end(), [&](const auto& pair) {
            return std::find(pair.second.begin(), pair.second.end(), i) != pair.second.end();
        });
        if (count != 1)
            throw std::runtime_error("Sail piece must have one inventory source");
    }
    ActivityObject seam;
    seam.id = "seam_object";
    const auto mask = root.child("sew_mask");
    seam.texture = resolve(base, mask.child("texture").attribute("src").value());
    if (seam.texture.empty())
        throw std::runtime_error("Missing sail seam texture");
    const auto position = mask.child("position");
    seam.x = position.attribute("x").as_float();
    seam.y = position.attribute("y").as_float();
    seam.z = position.attribute("z").as_float();
    seam.visible = seam.interactive = false;
    objects_.push_back(seam);
    measureActivityTextures(assets, objects_);
    measureActivityTextures(assets, fixed);
    for (size_t i = 0; i < pieces_.size(); ++i) {
        normalSizes_.push_back({piece(i).width, piece(i).height});
        fixedSizes_.push_back({fixed[i].width, fixed[i].height});
    }
    updateVisuals();
}
const ActivityObject& Sail::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing sail object: " + id);
    return *found;
}
ActivityObject& Sail::mutableObject(const std::string& id) {
    return const_cast<ActivityObject&>(object(id));
}
const ActivityObject& Sail::piece(size_t index) const {
    return objects_.at(index + 1);
}
ActivityObject& Sail::mutablePiece(size_t index) {
    return objects_.at(index + 1);
}
bool Sail::correct(size_t index) const {
    const auto& state = pieces_.at(index);
    const auto& sprite = piece(index);
    return state.placed && state.turns == 0 &&
           std::hypot(sprite.x - state.targetX, sprite.y - state.targetY) <= threshold_;
}
bool Sail::ready() const {
    for (size_t i = 0; i < pieces_.size(); ++i)
        if (!correct(i))
            return false;
    return true;
}
void Sail::hint(const std::string& id) {
    events_.push_back({SailEvent::Hint, "puzzles/sail/hints/" + id});
}
void Sail::enter() {
    phase_ = Phase::Idle;
    sewAge_ = 0;
    events_.clear();
    for (size_t i = 0; i < pieces_.size(); ++i)
        pieces_[i].fixedView = correct(i);
    if (!complete_) {
        hint("start");
        if (ready())
            hint("ready_to_sew");
    }
    updateVisuals();
}
void Sail::leave() {
    phase_ = Phase::Idle;
    sewAge_ = 0;
    updateVisuals();
}
void Sail::normalizeDepths() {
    std::vector<size_t> sorted(pieces_.size());
    std::iota(sorted.begin(), sorted.end(), 0);
    std::stable_sort(sorted.begin(), sorted.end(), [&](auto a, auto b) { return piece(a).z < piece(b).z; });
    for (size_t i = 0; i < sorted.size(); ++i)
        mutablePiece(sorted[i]).z = -1 - float(sorted.size() - i - 1) * .01f;
}
void Sail::checkReady() {
    if (ready() && !readyAnnounced_) {
        readyAnnounced_ = true;
        hint("ready_to_sew");
    }
}
bool Sail::place(const std::string& inventory, float x, float y) {
    if (busy() || gripping() || complete_ || ready() || !std::isfinite(x) || !std::isfinite(y))
        return false;
    const auto found = inventory_.find(inventory);
    if (found == inventory_.end()) {
        hint("wrong_item");
        return false;
    }
    for (auto index : found->second)
        if (pieces_[index].placed)
            return false;
    for (auto index : found->second) {
        auto& state = pieces_[index];
        auto& sprite = mutablePiece(index);
        state.placed = true;
        sprite.x = x;
        sprite.y = y;
        sprite.z -= float(pieces_.size()) * .1f;
    }
    normalizeDepths();
    checkReady();
    updateVisuals();
    return true;
}
bool Sail::press(size_t index, float x, float y) {
    if (busy() || gripping() || complete_ || ready() || index >= pieces_.size() || !pieces_[index].placed ||
        !std::isfinite(x) || !std::isfinite(y))
        return false;
    if (correct(index)) {
        hint("correct_place");
        return false;
    }
    held_ = index;
    pressX_ = x;
    pressY_ = y;
    auto& sprite = mutablePiece(index);
    offsetX_ = x - sprite.x;
    offsetY_ = y - sprite.y;
    sprite.z -= float(pieces_.size()) * .1f;
    normalizeDepths();
    phase_ = Phase::Pressed;
    return true;
}
void Sail::move(float x, float y) {
    if (!gripping() || !std::isfinite(x) || !std::isfinite(y))
        return;
    if (phase_ == Phase::Pressed) {
        if (std::hypot(x - pressX_, y - pressY_) > 5) {
            phase_ = Phase::Dragging;
            events_.push_back({SailEvent::ClearHint, {}});
        }
    } else {
        auto& sprite = mutablePiece(held_);
        sprite.x = x - offsetX_;
        sprite.y = y - offsetY_;
    }
}
void Sail::snapCorrect() {
    for (size_t i = 0; i < pieces_.size(); ++i)
        if (correct(i)) {
            auto& sprite = mutablePiece(i);
            sprite.x = pieces_[i].targetX;
            sprite.y = pieces_[i].targetY;
        }
}
void Sail::fixView(size_t index, bool feedback) {
    auto& state = pieces_[index];
    if (correct(index)) {
        if (feedback && !state.fixedView) {
            mutablePiece(index).z += float(pieces_.size()) * .1f;
            normalizeDepths();
            hint("correct_place");
        }
        state.fixedView = true;
    } else
        state.fixedView = false;
}
bool Sail::release() {
    if (!gripping())
        return false;
    if (phase_ == Phase::Pressed) {
        auto& state = pieces_[held_];
        auto& sprite = mutablePiece(held_);
        // Preserve the pixel under the original press while turning clockwise.
        const float dx = sprite.x - pressX_, dy = sprite.y - pressY_;
        sprite.x = pressX_ - dy;
        sprite.y = pressY_ + dx;
        state.turns = (state.turns + 1) % 4;
        snapCorrect();
        fixView(held_, true);
        events_.push_back({SailEvent::Sound, "sound/sfx/mg/sail/mg_turn_clothes"});
    } else {
        fixView(held_, true);
        snapCorrect();
    }
    phase_ = Phase::Idle;
    checkReady();
    updateVisuals();
    return true;
}
void Sail::cancelGrip() {
    if (gripping())
        phase_ = Phase::Idle;
}
bool Sail::sew(const std::string& inventory) {
    if (busy() || gripping() || complete_ || !ready())
        return false;
    if (inventory != sewingItem_) {
        hint(inventory == "thread" ? "thread_without_needle" : "wrong_item_for_sew");
        return false;
    }
    sewAge_ = 0;
    phase_ = Phase::Sewing;
    events_.push_back({SailEvent::ClearHint, {}});
    events_.push_back({SailEvent::Sound, "sound/sfx/mg/sail/mg_turn_clothes"});
    updateVisuals();
    return true;
}
void Sail::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid sail time advance");
    if (phase_ != Phase::Sewing)
        return;
    sewAge_ += seconds;
    if (sewAge_ > 3) {
        complete_ = true;
        phase_ = Phase::Idle;
        events_.push_back({SailEvent::Finish, {}});
    }
    updateVisuals();
}
void Sail::updateVisuals() {
    for (size_t i = 0; i < pieces_.size(); ++i) {
        const auto& state = pieces_[i];
        auto& sprite = mutablePiece(i);
        const auto size = state.fixedView ? fixedSizes_[i] : normalSizes_[i];
        sprite.texture = state.fixedView ? state.fixedTexture : state.texture;
        sprite.width = size.first;
        sprite.height = size.second;
        sprite.visible = state.placed;
        sprite.angle = float(state.turns * 90);
    }
    auto& seam = mutableObject("seam_object");
    seam.visible = busy() || complete_;
    seam.alpha = complete_ ? 1 : float(std::clamp(sewAge_ / 2.5, 0., 1.));
}
std::vector<SailEvent> Sail::takeEvents() {
    auto result = std::move(events_);
    events_.clear();
    return result;
}
} // namespace deadtime
