#include "voodoo_circle.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <pugixml.hpp>
#include <set>
#include <stdexcept>

namespace deadtime {
namespace {
std::string number(size_t i) {
    return (i < 9 ? "0" : "") + std::to_string(i + 1);
}
} // namespace
size_t CircleShape::paintedCount() const {
    return size_t(std::count(painted.begin(), painted.end(), true));
}
VoodooCircle::~VoodooCircle() = default;
VoodooCircle::VoodooCircle(const Assets& assets, const std::string& base)
    : base_(normalize(base)), doll_(assets, base_ + "/doll.animset") {
    pugi::xml_document doc;
    const auto bytes = assets.read(base_ + "/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("voodoo_circle"))
        throw std::runtime_error("Invalid voodoo-circle configuration");
    const auto root = doc.child("voodoo_circle");
    spell_ = root.child("spell").attribute("name").value();
    const auto count = root.child("garbage_objects").attribute("quantity").as_uint();
    if (spell_.empty() || count == 0 || count > 99)
        throw std::runtime_error("Invalid voodoo-circle spell/garbage count");
    garbage_.resize(count);
    std::set<std::string> inventory;
    unsigned conduits = 0;
    for (auto node : root.child("spell_items").children("item")) {
        CircleItem item{node.attribute("inventory_object").value(),
                        node.attribute("on_scene").as_bool(true),
                        node.attribute("conduit").as_bool(),
                        0,
                        {}};
        if (node.attribute("index").as_uint() != items_.size() || item.inventory.empty() ||
            !inventory.insert(item.inventory).second)
            throw std::runtime_error("Invalid voodoo-circle spell item");
        conduits += item.conduit;
        items_.push_back(item);
    }
    if (items_.size() < 2 || items_.size() > 99 ||
        items_.size() != root.child("spell_items").attribute("quantity").as_uint() || conduits != 1)
        throw std::runtime_error("Invalid voodoo-circle item count/conduit");
    const auto drawing = root.child("drawing");
    strokeAlgorithm_ = drawing.attribute("chalk_stroke_algorithm_enabled").as_bool();
    stopSoundSeconds_ = drawing.attribute("stop_sound_time_ms").as_double() / 1000.;
    for (auto node : drawing.children("shape")) {
        CircleShape shape;
        if (node.attribute("index").as_uint() != shapes_.size())
            throw std::runtime_error("Voodoo-circle shape indexes are not consecutive");
        shape.distance = node.attribute("max_distance_from_control_point").as_double();
        shape.stopDistance = node.attribute("stop_drawing_distance_threshold").as_double();
        shape.required = node.attribute("min_percentage_to_finish").as_double() / 100.;
        shape.strokeLength = node.attribute("chalk_stroke_length").as_double();
        shape.strokeThickness = node.attribute("chalk_stroke_thickness").as_double();
        for (auto value :
             {shape.distance, shape.stopDistance, shape.required, shape.strokeLength, shape.strokeThickness})
            if (!std::isfinite(value) || value < 0)
                throw std::runtime_error("Invalid voodoo-circle drawing parameter");
        if (shape.distance <= 0 || shape.required <= 0 || shape.required > 1 || shape.strokeLength <= 0 ||
            shape.strokeThickness <= 0)
            throw std::runtime_error("Invalid voodoo-circle drawing range");
        shapes_.push_back(std::move(shape));
    }
    if (shapes_.empty() || shapes_.size() > 99 ||
        shapes_.size() != drawing.attribute("num_shapes").as_uint() || !std::isfinite(stopSoundSeconds_) ||
        stopSoundSeconds_ < 0 || doll_.duration() <= 0)
        throw std::runtime_error("Invalid voodoo-circle shape count/timing");
    auto hook = [&](const char* name) {
        const std::string script = root.child(name).attribute("script").value();
        if (script.empty())
            return std::string{};
        const auto path = resolve(base_, script) + ".lua";
        if (!assets.contains(path))
            throw std::runtime_error("Missing voodoo-circle callback: " + path);
        return path;
    };
    onEnter = hook("onEnter");
    onExit = hook("onExit");
    onFinish = hook("onFinish");
    objects_ = loadActivityLayout(assets, base_ + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (const auto* id :
         {"back", "exit", "moss", "voodoo_circle_mask", "fx_doll", "fader", "spell_item_slot_central"})
        object(id);
    for (size_t i = 0; i < garbage_.size(); ++i) {
        object("obj_" + number(i));
        object("obj_" + number(i) + "_moved");
    }
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].onScene)
            object("spell_item_" + number(i));
        object("spell_item_" + number(i) + "_placed");
        if (i + 1 < items_.size())
            object("spell_item_slot_" + number(i));
    }
    const auto layout = assets.read(base_ + "/layout.xml");
    doc.load_buffer(layout.data(), layout.size());
    for (auto node : doc.child("objects").children("patch")) {
        const std::string id = node.attribute("name").value();
        if (!id.starts_with("shape_"))
            throw std::runtime_error("Unsupported voodoo-circle patch");
        const size_t index = std::stoul(id.substr(6)) - 1;
        if (index >= shapes_.size() || !shapes_[index].sprite.id.empty())
            throw std::runtime_error("Duplicate/unknown voodoo-circle patch");
        auto& shape = shapes_[index];
        shape.sprite.id = id;
        shape.sprite.texture = resolve(base_, node.child("texture").attribute("src").value());
        const auto position = node.child("position");
        shape.sprite.x = position.attribute("x").as_float();
        shape.sprite.y = position.attribute("y").as_float();
        shape.sprite.z = position.attribute("z").as_float();
        shape.sprite.width = node.child("size").attribute("width").as_float();
        shape.sprite.height = node.child("size").attribute("height").as_float();
        std::vector<ActivityObject> measured{shape.sprite};
        measureActivityTextures(assets, measured);
        shape.sprite = measured.front();
        shape.sprite.interactive = false;
        shape.segmentsX = node.child("discretization").attribute("segments_x").as_uint();
        shape.segmentsY = node.child("discretization").attribute("segments_y").as_uint();
        if (!shape.segmentsX || !shape.segmentsY || shape.segmentsX > 200 || shape.segmentsY > 200)
            throw std::runtime_error("Invalid voodoo-circle patch grid");
        shape.alpha.resize(size_t(shape.segmentsX + 1) * (shape.segmentsY + 1),
                           node.child("vertex_alpha").attribute("default").as_float());
    }
    for (size_t i = 0; i < shapes_.size(); ++i) {
        auto& shape = shapes_[i];
        if (shape.sprite.id.empty())
            throw std::runtime_error("Missing voodoo-circle patch");
        const auto points = assets.read(base_ + "/shapes/" + number(i) + ".xml");
        pugi::xml_document source;
        if (!source.load_buffer(points.data(), points.size()))
            throw std::runtime_error("Invalid voodoo-circle control points");
        for (auto point : source.child("points").children("point")) {
            const float x = point.attribute("x").as_float(), y = point.attribute("y").as_float();
            if (!std::isfinite(x) || !std::isfinite(y) || x < 0 || y < 0 || x >= shape.sprite.width ||
                y >= shape.sprite.height)
                throw std::runtime_error("Voodoo-circle control point outside patch");
            shape.points.push_back({shape.sprite.x + x, shape.sprite.y + y});
        }
        if (shape.points.empty())
            throw std::runtime_error("Empty voodoo-circle control points");
        shape.painted.resize(shape.points.size());
    }
    for (auto node : doc.child("objects").children("object"))
        if (node.child("scene") || node.child("particles")) {
            const std::string id = node.attribute("name").value();
            const auto& sprite = object(id);
            auto effect = std::make_unique<Object>();
            effect->id = id;
            effect->x = sprite.x;
            effect->y = sprite.y;
            effect->z = sprite.z;
            effect->enabled = false;
            effect->visible = sprite.visible;
            effect->alpha = sprite.alpha;
            for (auto scene : node.children("scene")) {
                Object::SceneResource resource{resolve(base_, scene.attribute("src").value()),
                                               scene.attribute("looped").as_bool(true),
                                               {}};
                for (auto graph : scene.children("particles"))
                    resource.particles.push_back({resolve(base_, graph.attribute("src").value()), 0});
                effect->sceneResources.push_back(std::move(resource));
            }
            for (auto graph : node.children("particles"))
                effect->particleResources.push_back({resolve(base_, graph.attribute("src").value()), 0});
            effects_.emplace(id, std::move(effect));
        }
    updateVisuals();
}
const ActivityObject& VoodooCircle::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& o) { return o.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing voodoo-circle object: " + id);
    return *found;
}
ActivityObject& VoodooCircle::mutableObject(const std::string& id) {
    return const_cast<ActivityObject&>(object(id));
}
void VoodooCircle::sound(const std::string& path) {
    events_.push_back({CircleEvent::Sound, path});
}
void VoodooCircle::hint(const std::string& key, bool popup) {
    events_.push_back(
        {popup ? CircleEvent::PopupHint : CircleEvent::Hint, "puzzles/voodoo_circle/hints/" + key});
}
bool VoodooCircle::cleared() const {
    return moss_ && std::all_of(garbage_.begin(), garbage_.end(), [](bool v) { return v; }) &&
           std::all_of(items_.begin(), items_.end(),
                       [](const auto& item) { return !item.onScene || item.state >= 1; });
}
bool VoodooCircle::drawn() const {
    return std::all_of(shapes_.begin(), shapes_.end(), [](const auto& shape) { return shape.complete; });
}
bool VoodooCircle::skip() {
    if (complete_ || ending_ || !spellKnown_)
        return false;
    // Native 50DAD0 prepares the cleared/drawn/offered model and reinitializes;
    // the original doll rise, spirits and final fade still run afterward.
    events_.clear();
    cancelDrawing();
    std::fill(garbage_.begin(), garbage_.end(), true);
    moss_ = true;
    for (size_t i = 0; i < items_.size(); ++i) {
        auto& item = items_[i];
        item.state = 2;
        if (item.conduit)
            item.slot = items_.size() - 1;
        else if (!item.slot)
            for (size_t slot = 0; slot + 1 < items_.size(); ++slot)
                if (std::none_of(items_.begin(), items_.end(),
                                 [&](const auto& other) { return other.slot == slot; })) {
                    item.slot = slot;
                    break;
                }
    }
    for (auto& shape : shapes_) {
        shape.complete = true;
        std::fill(shape.painted.begin(), shape.painted.end(), true);
        for (size_t i = 0; i < shape.points.size(); ++i)
            revealPoint(shape, i, true);
    }
    checkFinished();
    updateVisuals();
    return true;
}
void VoodooCircle::enter(bool known) {
    spellKnown_ = known;
    entered_ = true;
    drawing_ = pressing_ = chalkSound_ = false;
    active_.reset();
    previous_.reset();
    strokePoints_.clear();
    events_.clear();
    for (auto& shape : shapes_)
        for (size_t i = 0; i < shape.points.size(); ++i)
            if (shape.complete || shape.painted[i])
                revealPoint(shape, i, true);
    for (auto& [id, effect] : effects_) {
        effect->enabled = false;
        effect->treeTime = 0;
        effect->particleGenerating = true;
    }
    updateVisuals();
}
void VoodooCircle::leave() {
    cancelDrawing();
    for (auto& [id, effect] : effects_)
        effect->enabled = false;
}
void VoodooCircle::checkCleared() {
    if (cleared()) {
        events_.push_back({CircleEvent::Flash, {}});
        sound("sound/sfx/mg/mg_glow");
        hint("circle_cleared", true);
    }
}
bool VoodooCircle::clearGarbage(size_t index) {
    if (busy() || complete_ || drawing_ || index >= garbage_.size() || garbage_[index])
        return false;
    if (!spellKnown_) {
        hint("spell_unknown", true);
        return false;
    }
    garbage_[index] = true;
    sound("sound/sfx/sfx_m_click_idle");
    checkCleared();
    updateVisuals();
    return true;
}
bool VoodooCircle::takeItem(size_t index) {
    if (busy() || complete_ || drawing_ || index >= items_.size() || !items_[index].onScene ||
        items_[index].state)
        return false;
    if (!spellKnown_) {
        hint("spell_unknown", true);
        return false;
    }
    items_[index].state = 1;
    events_.push_back({CircleEvent::Award, items_[index].inventory});
    sound("sound/sfx/sfx_item_found");
    checkCleared();
    updateVisuals();
    return true;
}
bool VoodooCircle::takeMoss() {
    if (busy() || complete_ || drawing_ || moss_)
        return false;
    if (!spellKnown_) {
        hint("spell_unknown", true);
        return false;
    }
    moss_ = true;
    events_.push_back({CircleEvent::Award, "moss"});
    sound("sound/sfx/sfx_item_found");
    checkCleared();
    updateVisuals();
    return true;
}
bool VoodooCircle::chooseChalk() {
    if (busy() || complete_ || !cleared() || drawn())
        return false;
    drawing_ = true;
    release();
    events_.push_back({CircleEvent::Select, "chalk"});
    return true;
}
bool VoodooCircle::apply(const std::string& inventory, std::optional<size_t> slot) {
    if (busy() || complete_ || drawing_)
        return false;
    if (!spellKnown_) {
        hint("spell_unknown");
        return false;
    }
    if (!cleared()) {
        hint("cannot_put_item_before_clearing_garbage");
        return false;
    }
    const auto found = std::find_if(items_.begin(), items_.end(),
                                    [&](const auto& item) { return item.inventory == inventory; });
    if (!drawn()) {
        if (inventory == "chalk")
            return chooseChalk();
        hint(found == items_.end() ? "wrong_item_for_drawing" : "cannot_put_item_before_drawing");
        return false;
    }
    if (found == items_.end() || found->state == 2) {
        hint("wrong_spell_item");
        return false;
    }
    if (!slot || *slot >= items_.size())
        return false;
    if (found->conduit != (*slot + 1 == items_.size())) {
        hint("doll_in_the_middle", true);
        return false;
    }
    if (std::any_of(items_.begin(), items_.end(), [&](const auto& item) { return item.slot == slot; }))
        return false;
    found->state = 2;
    found->slot = slot;
    events_.push_back({CircleEvent::Consume, inventory});
    events_.push_back({CircleEvent::Select, {}});
    checkFinished();
    updateVisuals();
    return true;
}
void VoodooCircle::idleClick(bool hasChalk) {
    if (busy() || drawing_)
        return;
    sound("sound/sfx/sfx_m_click_idle");
    if (!spellKnown_)
        hint("spell_unknown", true);
    else if (!cleared())
        hint("need_to_clear_garbage", true);
    else if (!drawn())
        hint(hasChalk ? "need_to_draw" : "need_chalk_to_draw", true);
    else if (!complete_)
        hint("need_to_put_spell_items", true);
}
void VoodooCircle::setChalkSound(bool enabled) {
    if (chalkSound_ != enabled) {
        chalkSound_ = enabled;
        events_.push_back({CircleEvent::ChalkSound, enabled ? "sound/sfx/mg/voodoo_circle/mg_chalk" : ""});
    }
}
void VoodooCircle::press(float x, float y) {
    if (!drawing_)
        return;
    pressing_ = true;
    soundAge_ = 0;
    active_.reset();
    strokePoints_.clear();
    drawAt(x, y);
    previous_ = {x, y};
    setChalkSound(pressing_ && drawing_);
}
void VoodooCircle::move(float x, float y) {
    if (drawing_ && pressing_) {
        soundAge_ = 0;
        drawAt(x, y);
    }
    previous_ = {x, y};
}
void VoodooCircle::release() {
    pressing_ = false;
    active_.reset();
    strokePoints_.clear();
    setChalkSound(false);
}
void VoodooCircle::cancelDrawing() {
    release();
    drawing_ = false;
    previous_.reset();
    events_.push_back({CircleEvent::ChalkSound, {}});
    events_.push_back({CircleEvent::Select, {}});
}
void VoodooCircle::revealPoint(CircleShape& shape, size_t index, bool expand) {
    const auto [x, y] = shape.points[index];
    const int gx = int((x - shape.sprite.x) / (shape.sprite.width / shape.segmentsX)),
              gy = int((y - shape.sprite.y) / (shape.sprite.height / shape.segmentsY));
    // Native completion/reentry expands along three diagonal grid vertices;
    // the inner source loop writes each of those vertices three times.
    for (int offset = expand ? -1 : 0; offset <= (expand ? 1 : 0); ++offset) {
        const int u = gx + offset, v = gy + offset;
        if (u >= 0 && v >= 0 && u <= int(shape.segmentsX) && v <= int(shape.segmentsY))
            shape.alpha[size_t(v) * (shape.segmentsX + 1) + size_t(u)] = 1;
    }
}
bool VoodooCircle::paint(size_t index, float x, float y) {
    auto& shape = shapes_[index];
    if (shape.complete)
        return false;
    if (shape.stopDistance > 0 && !strokePoints_.empty() &&
        std::none_of(strokePoints_.begin(), strokePoints_.end(), [&](size_t i) {
            const auto p = shape.points[i];
            return std::hypot(x - p.first, y - p.second) <= shape.stopDistance;
        })) {
        active_.reset();
        return false;
    }
    double fromX = x, fromY = y, ux = 0, uy = 0;
    if (strokeAlgorithm_ && previous_) {
        const auto dx = x - previous_->first, dy = y - previous_->second, distance = std::hypot(dx, dy);
        if (distance > 0) {
            fromX = previous_->first;
            fromY = previous_->second;
            ux = dx / distance;
            uy = dy / distance;
        }
    }
    bool changed = false;
    for (size_t i = 0; i < shape.points.size(); ++i) {
        const auto point = shape.points[i];
        const double dx = point.first - fromX, dy = point.second - fromY;
        const double projection = dx * ux + dy * uy;
        const bool within = strokeAlgorithm_ ? ((ux == 0 && uy == 0) ||
                                                (projection >= 0 && projection <= shape.strokeLength)) &&
                                                   dx * dx + dy * dy - projection * projection <=
                                                       shape.strokeThickness * shape.strokeThickness / 4
                                             : std::hypot(dx, dy) < shape.distance + 1;
        if (!within)
            continue;
        if (std::find(strokePoints_.begin(), strokePoints_.end(), i) == strokePoints_.end())
            strokePoints_.push_back(i);
        if (!shape.painted[i]) {
            shape.painted[i] = true;
            revealPoint(shape, i, false);
            changed = true;
        }
    }
    if (changed && double(shape.paintedCount()) / shape.points.size() >= shape.required) {
        shape.complete = true;
        for (size_t i = 0; i < shape.points.size(); ++i)
            revealPoint(shape, i, true);
        active_.reset();
        strokePoints_.clear();
        if (drawn()) {
            cancelDrawing();
            events_.push_back({CircleEvent::Consume, "chalk"});
            events_.push_back({CircleEvent::Flash, {}});
            sound("sound/sfx/mg/mg_glow");
            hint("drawing_finished", true);
            updateVisuals();
        }
    }
    return changed;
}
void VoodooCircle::drawAt(float x, float y) {
    if (!std::isfinite(x) || !std::isfinite(y))
        return;
    const auto old = active_;
    if (active_ && paint(*active_, x, y))
        return;
    double closest = std::numeric_limits<double>::infinity();
    std::optional<size_t> next;
    for (size_t i = 0; i < shapes_.size(); ++i)
        if (!shapes_[i].complete && old != i)
            for (auto point : shapes_[i].points) {
                const auto distance = std::hypot(x - point.first, y - point.second);
                if (distance < closest) {
                    closest = distance;
                    next = i;
                }
            }
    if (!next)
        return;
    active_ = next;
    strokePoints_.clear();
    paint(*next, x, y);
}
void VoodooCircle::checkFinished() {
    if (ending_ || complete_ ||
        !std::all_of(items_.begin(), items_.end(), [](const auto& item) { return item.state == 2; }))
        return;
    ending_ = true;
    age_ = 0;
    mutableObject("fx_doll").visible = true;
    doll_.apply(mutableObject("fx_doll"), 0);
    if (auto found = effects_.find("fx_spirits"); found != effects_.end()) {
        found->second->enabled = true;
        found->second->particleGenerating = true;
        found->second->treeTime = 0;
    }
    for (const auto* path :
         {"sound/sfx/sfx_m_spirit_whining2", "sound/sfx/sfx_rays_appear", "sound/sfx/sfx_f_spirit_whining1"})
        sound(path);
}
void VoodooCircle::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid voodoo-circle time advance");
    soundAge_ += seconds;
    setChalkSound(drawing_ && pressing_ && soundAge_ < stopSoundSeconds_);
    if (busy()) {
        age_ += seconds;
        doll_.apply(mutableObject("fx_doll"), std::min(age_, doll_.duration()));
        if (age_ >= doll_.duration()) {
            if (auto f = effects_.find("fx_spirits"); f != effects_.end())
                f->second->particleGenerating = false;
            mutableObject("fader").alpha = std::clamp(float((age_ - doll_.duration()) / 3.), 0.f, 1.f);
        }
        if (age_ + 1e-10 >= doll_.duration() + 3) {
            complete_ = true;
            events_.push_back({CircleEvent::Finish, {}});
        }
    }
    updateVisuals();
}
void VoodooCircle::updateVisuals() {
    for (size_t i = 0; i < garbage_.size(); ++i) {
        mutableObject("obj_" + number(i)).visible = !garbage_[i];
        mutableObject("obj_" + number(i) + "_moved").visible = garbage_[i];
    }
    mutableObject("moss").visible = !moss_;
    for (size_t i = 0; i < items_.size(); ++i) {
        const auto& item = items_[i];
        if (item.onScene)
            mutableObject("spell_item_" + number(i)).visible = item.state == 0;
        auto& sprite = mutableObject("spell_item_" + number(i) + "_placed");
        sprite.visible = item.state == 2 && !(item.conduit && ending_);
        if (item.slot && !item.conduit) {
            const auto& slot = object("spell_item_slot_" + number(*item.slot));
            sprite.x = slot.x + slot.width / 2;
            sprite.y = slot.y + slot.height / 2;
            sprite.z = slot.z;
        }
    }
    if (auto f = effects_.find("fx_voodoo_circle_ambient"); f != effects_.end())
        f->second->enabled = drawn() && !complete_;
    for (auto& o : objects_) {
        o.interactive = o.id == "exit" || o.id == "voodoo_circle_mask" || o.id == "moss" ||
                        (o.id.starts_with("obj_") && !o.id.ends_with("_moved")) ||
                        (o.id.starts_with("spell_item_") && !o.id.ends_with("_placed") &&
                         !o.id.starts_with("spell_item_slot_"));
        if (o.id.starts_with("spell_item_slot_"))
            o.interactive = drawn() && !complete_;
    }
}
std::vector<CircleEvent> VoodooCircle::takeEvents() {
    auto result = std::move(events_);
    events_.clear();
    return result;
}
} // namespace deadtime
