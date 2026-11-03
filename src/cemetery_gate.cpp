#include "cemetery_gate.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>

namespace deadtime {
namespace {
std::string numbered(size_t index) {
    return std::string("block_") + (index < 9 ? "0" : "") + std::to_string(index + 1);
}
std::string lower(std::string value) {
    for (auto& c : value)
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
    return value;
}
} // namespace
CemeteryGate::~CemeteryGate() = default;
CemeteryGate::CemeteryGate(const Assets& assets, const std::string& base, uint32_t seed)
    : base_(normalize(base)), rotation_(assets, base_ + "/block_rotation.animset") {
    pugi::xml_document doc;
    const auto bytes = assets.read(base_ + "/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("cemetery_gate"))
        throw std::runtime_error("Invalid cemetery-gate configuration");
    const auto root = doc.child("cemetery_gate");
    const auto alphabet = lower(root.child_value("alphabet"));
    if (alphabet.empty())
        throw std::runtime_error("Empty cemetery-gate alphabet");
    answer_ = lower(root.child_value("phrase"));
    std::erase(answer_, ' ');
    for (char c : answer_)
        if (alphabet.find(c) == std::string::npos)
            throw std::runtime_error("Invalid cemetery-gate answer letter");
    for (auto node : root.child("letter_blocks").children("block")) {
        const auto letters = lower(node.attribute("letters").value());
        if (letters.empty())
            throw std::runtime_error("Empty cemetery-gate block");
        for (char c : letters)
            if (alphabet.find(c) == std::string::npos)
                throw std::runtime_error("Invalid cemetery-gate block letter");
        blocks_.push_back(letters);
    }
    if (blocks_.empty() || blocks_.size() > 99 || blocks_.size() != answer_.size())
        throw std::runtime_error("Cemetery-gate block/phrase count mismatch");
    for (size_t i = 0; i < blocks_.size(); ++i)
        if (blocks_[i].find(answer_[i]) == std::string::npos)
            throw std::runtime_error("Unsolvable cemetery-gate block");
    const auto initial = root.child("initial_correct_letters");
    const auto minimum = initial.attribute("min").as_uint(), maximum = initial.attribute("max").as_uint();
    if (minimum > maximum || maximum > blocks_.size())
        throw std::runtime_error("Invalid cemetery-gate initial correct range");
    current_.resize(blocks_.size());
    ParticleRandom random(seed);
    bool arranged = false;
    // Native 42F9A0 redraws every block until the total is in the configured
    // inclusive range. Use the same MSVC rand step without sharing global RNG.
    for (unsigned attempt = 0; attempt < 1000000; ++attempt) {
        size_t correct = 0;
        for (size_t i = 0; i < blocks_.size(); ++i) {
            current_[i] = random.next() % blocks_[i].size();
            correct += locked(i);
        }
        if (correct >= minimum && correct <= maximum) {
            arranged = true;
            break;
        }
    }
    if (!arranged)
        throw std::runtime_error("Cannot generate cemetery-gate initial arrangement");
    if (rotation_.looped() || rotation_.pictures().size() != 1 || rotation_.duration() <= 0)
        throw std::runtime_error("Invalid cemetery-gate rotation animation");
    font_ = resolve(base_, root.child("font").attribute("src").value());
    color_ = unsigned(std::stoul(root.child("font").attribute("color").as_string("FFFFFFFF"), nullptr, 16));
    const auto offset = root.child("letter_displace_from_block_center");
    letterX_ = offset.attribute("x").as_float();
    letterY_ = offset.attribute("y").as_float();
    auto hook = [&](const char* name) {
        const std::string script = root.child(name).attribute("script").value();
        if (script.empty())
            return std::string{};
        const auto path = resolve(base_, script) + ".lua";
        if (!assets.contains(path))
            throw std::runtime_error("Missing cemetery-gate callback: " + path);
        return path;
    };
    onEnter = hook("onEnter");
    onExit = hook("onExit");
    onFinish = hook("onFinish");
    objects_ = loadActivityLayout(assets, base_ + "/layout.xml");
    measureActivityTextures(assets, objects_);
    object("back");
    object("exit");
    object("fx_block_rotation");
    for (size_t i = 0; i < blocks_.size(); ++i)
        sizes_.push_back({block(i).width, block(i).height});
    std::vector<ActivityObject> textures;
    for (const auto& path : {base_ + "/block", base_ + "/block_o"}) {
        ActivityObject sprite;
        sprite.texture = path;
        textures.push_back(sprite);
    }
    for (const auto& track : rotation_.pictures())
        for (const auto& key : track.keys) {
            ActivityObject sprite;
            sprite.texture = key.texture;
            textures.push_back(sprite);
        }
    measureActivityTextures(assets, textures);
    for (const auto& sprite : textures)
        textureSizes_[sprite.texture] = {sprite.width, sprite.height};
    const auto layout = assets.read(base_ + "/layout.xml");
    doc.load_buffer(layout.data(), layout.size());
    effect_ = std::make_unique<Object>();
    effect_->id = "fx_block_rotation";
    effect_->particleGenerating = false;
    for (auto node : doc.child("objects").children("object"))
        if (std::string(node.attribute("name").value()) == effect_->id)
            for (auto graph : node.children("particles"))
                effect_->particleResources.push_back({resolve(base_, graph.attribute("src").value()), 0});
    updateVisuals();
}
const ActivityObject& CemeteryGate::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& o) { return o.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing cemetery-gate object: " + id);
    return *found;
}
const ActivityObject& CemeteryGate::block(size_t index) const {
    return object(numbered(index));
}
char CemeteryGate::letter(size_t index) const {
    return blocks_.at(index).at(current_.at(index));
}
bool CemeteryGate::locked(size_t index) const {
    return index < current_.size() && letter(index) == answer_[index];
}
Object& CemeteryGate::effect() {
    return *effect_;
}
void CemeteryGate::enter() {
    entered_ = true;
    rotating_.reset();
    age_ = 0;
    events_.clear();
    effect_->particleGenerating = false;
    updateVisuals();
}
void CemeteryGate::leave() {
    rotating_.reset();
    age_ = 0;
    effect_->particleGenerating = false;
    updateVisuals();
}
bool CemeteryGate::select(size_t index) {
    if (complete_ || busy() || index >= blocks_.size())
        return false;
    if (locked(index)) {
        events_.push_back({GateEvent::Hint, "puzzles/cemetery_gate/hints/block_locked"});
        return false;
    }
    current_[index] = (current_[index] + 1) % blocks_[index].size();
    rotating_ = index;
    age_ = 0;
    const auto& sprite = block(index);
    effect_->x = sprite.x + sprite.width / 2 + letterX_;
    effect_->y = sprite.y + sprite.height / 2 + letterY_;
    effect_->z = sprite.z - 1;
    effect_->particleGenerating = true;
    events_.push_back({GateEvent::Sound, "sound/sfx/mg/cemetery_gate/mg_spin_letter"});
    updateVisuals();
    return true;
}
void CemeteryGate::idleClick() {
    if (!busy())
        events_.push_back({GateEvent::Sound, "sound/sfx/sfx_m_click_idle"});
}
bool CemeteryGate::skip() {
    if (complete_)
        return false;
    // Native 500AD0 arranges the answer before Finish/Reinit; no turn animation.
    rotating_.reset();
    age_ = 0;
    effect_->particleGenerating = false;
    events_.clear();
    for (size_t i = 0; i < blocks_.size(); ++i)
        current_[i] = blocks_[i].find(answer_[i]);
    complete_ = true;
    events_.push_back({GateEvent::Finish, {}});
    updateVisuals();
    return true;
}
void CemeteryGate::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid cemetery-gate time advance");
    if (!busy())
        return;
    age_ += seconds;
    if (age_ + 1e-10 >= rotation_.duration()) {
        rotating_.reset();
        age_ = 0;
        effect_->particleGenerating = false;
        bool solved = true;
        for (size_t i = 0; i < blocks_.size(); ++i)
            solved &= locked(i);
        if (solved && !complete_) {
            complete_ = true;
            events_.push_back({GateEvent::Finish, {}});
        }
    }
    updateVisuals();
}
void CemeteryGate::updateVisuals() {
    for (size_t i = 0; i < blocks_.size(); ++i) {
        auto& sprite = const_cast<ActivityObject&>(block(i));
        sprite.width = sizes_[i].first;
        sprite.height = sizes_[i].second;
        sprite.texture = base_ + (locked(i) ? "/block_o" : "/block");
        if (rotating_ == i)
            rotation_.apply(sprite, age_);
        const auto size = textureSizes_.at(sprite.texture);
        sprite.width = size.first;
        sprite.height = size.second;
    }
}
std::vector<GateEvent> CemeteryGate::takeEvents() {
    auto result = std::move(events_);
    events_.clear();
    return result;
}
} // namespace deadtime
