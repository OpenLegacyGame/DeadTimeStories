#include "spirit_lock.hpp"
#include "audio_timing.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>

namespace deadtime {
bool SpiritLock::skip() {
    if (complete_)
        return false;
    // Native 4D9920 consumes unplaced fragments and emits Finish/Reinit.
    std::fill(placed_.begin(), placed_.end(), true);
    phase_ = Phase::Idle;
    complete_ = allPiecesFound_ = true;
    key_ = 0;
    angle_ = target_ = applied_ = lastKeyAngle_ = 0;
    finalSeconds_ = 0;
    audioCallback_ = false;
    events_.clear();
    for (auto& [id, effect] : effects_) {
        effect->enabled = false;
        effect->particleGenerating = false;
    }
    events_.push_back({SpiritLockEvent::Finish, {}});
    updateVisuals();
    return true;
}
namespace {
constexpr double pi = 3.14159265358979323846;
std::string pieceName(size_t index) {
    return "piece_" + std::string(index < 9 ? "0" : "") + std::to_string(index + 1);
}
} // namespace
SpiritLock::~SpiritLock() = default;
SpiritLock::SpiritLock(const Assets& assets, const std::string& base) {
    pugi::xml_document doc;
    const auto bytes = assets.read(base + "/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid spirit lock configuration");
    const auto root = doc.child("spirit_lock"), turn = root.child("turn_lock");
    for (auto piece : root.child("pieces").children())
        pieces_.push_back(piece.name());
    if (pieces_.empty())
        throw std::runtime_error("Empty spirit lock pieces");
    placed_.resize(pieces_.size());
    velocity_ = turn.child("velocity").attribute("degrees_per_second").as_float();
    swingVelocity_ = turn.child("swing_back_velocity").attribute("degrees_per_second").as_float();
    minRadius_ = turn.child("limit_radius_vector").attribute("min_ratio").as_float();
    maxRadius_ = turn.child("limit_radius_vector").attribute("max_ratio").as_float();
    minAngle_ = turn.child("limit_angle_difference").attribute("min_degrees").as_float();
    maxAngle_ = turn.child("limit_angle_difference").attribute("max_degrees").as_float();
    for (float value : {velocity_, swingVelocity_, minRadius_, maxRadius_, minAngle_, maxAngle_})
        if (!std::isfinite(value))
            throw std::runtime_error("Non-finite spirit lock limits");
    if (velocity_ <= 0 || swingVelocity_ <= 0 || minRadius_ < 0 || minRadius_ >= 1 ||
        minRadius_ >= maxRadius_ || minAngle_ < 0 || minAngle_ >= maxAngle_)
        throw std::runtime_error("Invalid spirit lock limits");
    for (auto key : turn.child("sequence").children("key")) {
        const auto sound = resolve(base, key.attribute("sound").value());
        const float angle = key.attribute("angle_degrees").as_float();
        if (!std::isfinite(angle) || sound.empty())
            throw std::runtime_error("Invalid spirit lock sequence");
        sequence_.push_back({angle, sound, vorbisDuration(assets, sound + ".ogg")});
    }
    if (sequence_.empty())
        throw std::runtime_error("Empty spirit lock sequence");
    auto hook = [&](const char* name) {
        const std::string path = root.child(name).attribute("script").value();
        if (path.empty())
            return std::string{};
        const auto result = resolve(base, path) + ".lua";
        if (!assets.contains(result))
            throw std::runtime_error("Missing spirit lock callback: " + result);
        return result;
    };
    onEnter = hook("onEnter");
    onExit = hook("onExit");
    onFinish = hook("onFinish");
    objects_ = loadActivityLayout(assets, base + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (const auto* id : {"back", "exit", "lock", "all_pieces", "overlay_star", "fx_put_piece"})
        object(id);
    for (size_t i = 0; i < pieces_.size(); ++i)
        piece(i);
    if (object("lock").pivotX + object("lock").pivotY <= 0)
        throw std::runtime_error("Missing spirit lock radius");
    // These native scene objects retain the original graphs and Collada helper
    // nodes. No generated substitute is used for the puzzle's visual effects.
    const auto layout = assets.read(base + "/layout.xml");
    if (!doc.load_buffer(layout.data(), layout.size()))
        throw std::runtime_error("Invalid spirit lock layout");
    for (auto node : doc.child("objects").children("object")) {
        if (!node.child("particles") && !node.child("scene"))
            continue;
        const std::string id = node.attribute("name").value();
        const auto& source = object(id);
        auto effect = std::make_unique<Object>();
        effect->id = id;
        effect->x = source.x;
        effect->y = source.y;
        effect->z = source.z;
        effect->alpha = source.alpha;
        effect->visible = source.visible;
        effect->enabled = source.enabled;
        for (auto particle : node.children("particles"))
            effect->particleResources.push_back(
                {resolve(base, particle.attribute("src").value()), particle.attribute("presim").as_double()});
        for (auto scene : node.children("scene")) {
            Object::SceneResource resource{resolve(base, scene.attribute("src").value()),
                                           scene.attribute("animation_looped").as_bool(true),
                                           {}};
            for (auto particle : scene.children("particles"))
                resource.particles.push_back({resolve(base, particle.attribute("src").value()),
                                              particle.attribute("presim").as_double()});
            if (id == "fx_put_piece")
                putDuration_ = std::max(putDuration_, ColladaScene(assets, resource.path).duration());
            effect->sceneResources.push_back(std::move(resource));
        }
        effects_.emplace(id, std::move(effect));
    }
    updateVisuals();
}
const ActivityObject& SpiritLock::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing spirit lock object: " + id);
    return *found;
}
ActivityObject& SpiritLock::mutableObject(const std::string& id) {
    return const_cast<ActivityObject&>(object(id));
}
const ActivityObject& SpiritLock::piece(size_t index) const {
    return object(pieceName(index));
}
const Object* SpiritLock::effect(const std::string& id) const {
    const auto found = effects_.find(id);
    return found == effects_.end() ? nullptr : found->second.get();
}
bool SpiritLock::assembled() const {
    return std::all_of(placed_.begin(), placed_.end(), [](bool value) { return value; });
}
void SpiritLock::hint(const std::string& name, bool reactive) {
    events_.push_back(
        {reactive ? SpiritLockEvent::PopupHint : SpiritLockEvent::Hint, "puzzles/spirit_lock/hints/" + name});
}
void SpiritLock::sound(const std::string& path) {
    events_.push_back({SpiritLockEvent::Sound, path});
}
void SpiritLock::enter(bool allPiecesFound) {
    phase_ = Phase::Idle;
    angle_ = 0;
    target_ = applied_ = 0;
    key_ = 0;
    finalSeconds_ = 0;
    audioCallback_ = false;
    events_.clear();
    allPiecesFound_ = allPiecesFound;
    for (auto& [id, effect] : effects_) {
        effect->treeTime = 0;
        effect->treePlaying = true;
        effect->treeLoopOverridden = id == "fx_put_piece";
        effect->treeLooped = false;
        effect->enabled = id != "fx_put_piece";
        effect->particleGenerating = true;
    }
    if (!entered_ && !complete_)
        hint(allPiecesFound_ ? "all_pieces_found" : "general", false);
    entered_ = true;
    updateVisuals();
}
bool SpiritLock::place(size_t slot, const std::string& item) {
    if (busy() || complete_ || slot >= placed_.size() || placed_[slot])
        return false;
    if (std::find(pieces_.begin(), pieces_.end(), item) == pieces_.end()) {
        wrongItem();
        return false;
    }
    if (pieces_[slot] != item) {
        sound("sound/sfx/sfx_m_click_idle");
        hint("wrong_piece");
        return false;
    }
    placed_[slot] = true;
    sound("sound/sfx/mg/spirit_lock/mg_piece_added");
    if (auto found = effects_.find("fx_put_piece"); found != effects_.end()) {
        auto& effect = *found->second;
        effect.enabled = true;
        effect.particleGenerating = true;
        effect.treeTime = 0;
    }
    if (assembled())
        hint("need_to_turn_the_lock");
    updateVisuals();
    return true;
}
void SpiritLock::idleClick() {
    if (busy())
        return;
    sound("sound/sfx/sfx_m_click_idle");
    if (!complete_)
        hint(allPiecesFound_ ? "all_pieces_found" : "general");
}
void SpiritLock::wrongItem() {
    if (!busy() && !complete_)
        hint("wrong_item", false);
}
bool SpiritLock::press(float x, float y) {
    if (busy() || complete_ || !assembled())
        return false;
    cursorX_ = x;
    cursorY_ = y;
    target_ = applied_ = lastKeyAngle_ = 0;
    key_ = 0;
    moved_ = false;
    finalSeconds_ = 0;
    phase_ = Phase::Turning;
    return true;
}
void SpiritLock::move(float x, float y) {
    if (!turning() || key_ == sequence_.size())
        return;
    const auto& lock = object("lock");
    const double dx = x - lock.x, dy = y - lock.y, length = std::hypot(dx, dy);
    const double radius = (lock.pivotX + lock.pivotY) / 2;
    if (length < minRadius_ * radius || length > maxRadius_ * radius || length == 0)
        return;
    const double oldX = cursorX_ - lock.x, oldY = cursorY_ - lock.y, oldLength = std::hypot(oldX, oldY);
    if (oldLength == 0)
        return;
    const double cosine = (dx * oldX + dy * oldY) / (length * oldLength);
    // Oversized jumps leave the accepted grip point unchanged, as in native.
    if (!(std::cos(maxAngle_ * pi / 180) < cosine))
        return;
    const double delta = std::acos(std::clamp(cosine, -1., 1.)) * 180 / pi;
    target_ += float((oldX * dy - oldY * dx > 0 ? 1 : -1) * delta);
    cursorX_ = x;
    cursorY_ = y;
}
void SpiritLock::release() {
    if (!turning() || key_ == sequence_.size())
        return;
    if (!moved_) {
        phase_ = Phase::Idle;
        hint("need_to_turn_the_lock");
        return;
    }
    phase_ = Phase::Swinging;
    angle_ = std::fmod(angle_, 360.f);
}
void SpiritLock::finishPronunciation() {
    if (!turning() || key_ != sequence_.size() || complete_)
        return;
    complete_ = true;
    phase_ = Phase::Idle;
    events_.push_back({SpiritLockEvent::Finish, {}});
}
void SpiritLock::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid spirit lock time advance");
    for (auto& [id, effect] : effects_)
        if (effect->enabled) {
            effect->treeTime += seconds;
            if (id == "fx_put_piece" && effect->treeTime >= putDuration_)
                effect->particleGenerating = false;
        }
    if (phase_ == Phase::Swinging) {
        const auto delta = std::min(std::abs(angle_), float(swingVelocity_ * seconds));
        angle_ += angle_ > 0 ? -delta : delta;
        if (std::abs(angle_) < .1 * 180 / pi) {
            angle_ = 0;
            phase_ = Phase::Idle;
            hint("wrong_turning");
        }
    } else if (phase_ == Phase::Turning) {
        if (key_ == sequence_.size()) {
            finalSeconds_ = std::max(0., finalSeconds_ - seconds);
            if (finalSeconds_ == 0 && !audioCallback_)
                finishPronunciation();
        }
        const float difference = target_ - applied_;
        if (!complete_ && std::abs(difference) >= minAngle_) {
            const auto delta =
                std::clamp(difference, float(-velocity_ * seconds), float(velocity_ * seconds));
            angle_ += delta;
            applied_ += delta;
            moved_ = true;
            if (key_ < sequence_.size()) {
                const auto& next = sequence_[key_];
                if ((lastKeyAngle_ < next.angle && next.angle <= angle_) ||
                    (lastKeyAngle_ > next.angle && next.angle >= angle_)) {
                    lastKeyAngle_ = next.angle;
                    ++key_;
                    if (key_ == sequence_.size()) {
                        finalSeconds_ = next.duration;
                        events_.push_back({SpiritLockEvent::FinalSound, next.sound});
                    } else
                        sound(next.sound);
                }
            }
        }
    }
    updateVisuals();
}
void SpiritLock::updateVisuals() {
    const bool all = assembled();
    for (size_t i = 0; i < pieces_.size(); ++i) {
        auto& object = mutableObject(pieceName(i));
        object.alpha = placed_[i] ? 1 : 0;
        object.visible = !all;
    }
    auto& combined = mutableObject("all_pieces");
    combined.visible = all;
    combined.angle = angle_;
    mutableObject("lock").angle = angle_;
    mutableObject("overlay_star").angle = angle_;
}
std::vector<SpiritLockEvent> SpiritLock::takeEvents() {
    auto result = std::move(events_);
    events_.clear();
    return result;
}
} // namespace deadtime
