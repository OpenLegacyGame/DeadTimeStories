#include "piano.hpp"
#include "audio_timing.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>
#include <utility>

namespace deadtime {
namespace {
std::string number(unsigned value) {
    const auto s = std::to_string(value);
    return std::string(3 - s.size(), '0') + s;
}
} // namespace
Piano::Piano(const Assets& assets, const std::string& path) : base_(normalize(path)) {
    pugi::xml_document doc;
    const auto bytes = assets.read(base_ + "/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("piano_puzzle"))
        throw std::runtime_error("Invalid piano configuration");
    const auto root = doc.child("piano_puzzle");
    auto hook = [&](const char* name) {
        const std::string script = root.child(name).attribute("script").value();
        const auto result = script.empty() ? std::string{} : resolve(base_, script) + ".lua";
        if (!result.empty() && !assets.contains(result))
            throw std::runtime_error("Missing piano script: " + result);
        return result;
    };
    onEnter = hook("onEnter");
    onExit = hook("onExit");
    onFinish = hook("onFinish");
    minimumKey_ = root.child("keys").attribute("min").as_uint();
    maximumKey_ = root.child("keys").attribute("max").as_uint();
    if (!minimumKey_ || maximumKey_ <= minimumKey_ || maximumKey_ > 100)
        throw std::runtime_error("Invalid piano key range");
    autoplaySeconds = root.child("auto_play_timeout").attribute("ms").as_double() / 1000;
    if (!std::isfinite(autoplaySeconds) || autoplaySeconds < 0)
        throw std::runtime_error("Invalid piano autoplay time");
    reward = root.child("reward").attribute("inventory_item").value();
    if (reward.empty())
        throw std::runtime_error("Missing piano reward");
    for (auto node : root.child("rounds").children("round")) {
        std::vector<unsigned> notes;
        for (auto key : node.children("key")) {
            const auto index = key.attribute("index").as_uint();
            if (index < minimumKey_ || index > maximumKey_)
                throw std::runtime_error("Piano note outside configured key range");
            notes.push_back(index);
        }
        if (notes.empty())
            throw std::runtime_error("Empty piano round");
        rounds_.push_back(std::move(notes));
    }
    if (rounds_.empty() || rounds_.size() > 10)
        throw std::runtime_error("Invalid piano round count");
    objects_ = loadActivityLayout(assets, base_ + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (const auto* name : {"back", "exit", "piano_open", "fx_piano_open", "reward"})
        object(name);
    for (size_t i = 0; i < rounds_.size(); ++i)
        object("notes_" + std::string(i < 9 ? "0" : "") + std::to_string(i + 1));
    for (unsigned i = minimumKey_; i <= maximumKey_; ++i) {
        key(i);
        soundDurations_[i] = vorbisDuration(assets, soundName(i) + ".ogg");
    }
    errorSeconds_ = vorbisDuration(assets, base_ + "/sounds/error.ogg");
    if (errorSeconds_ <= 0 || std::any_of(soundDurations_.begin(), soundDurations_.end(),
                                          [](const auto& sound) { return sound.second <= 0; }))
        throw std::runtime_error("Empty piano sound");
    enter();
}
const ActivityObject& Piano::object(const std::string& name) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == name; });
    if (found == objects_.end())
        throw std::runtime_error("Missing piano object: " + name);
    return *found;
}
ActivityObject& Piano::mutableObject(const std::string& name) {
    return const_cast<ActivityObject&>(object(name));
}
std::string Piano::keyName(unsigned index) const {
    return "key" + number(index);
}
std::string Piano::soundName(unsigned index) const {
    return base_ + "/sounds/" + number(index);
}
const ActivityObject& Piano::key(unsigned index) const {
    if (index < minimumKey_ || index > maximumKey_)
        throw std::runtime_error("Unknown piano key");
    return object(keyName(index));
}
void Piano::enter() {
    releaseNote();
    queuedKeys_.clear();
    progress_ = 0;
    phase_ = Phase::Idle;
    remaining_ = autoplaySeconds;
    openAge_ = 1;
    updateVisuals();
}
void Piano::updateVisuals() {
    for (unsigned i = minimumKey_; i <= maximumKey_; ++i) {
        auto& sprite = mutableObject(keyName(i));
        sprite.alpha = soundingKey_ == i ? 1 : 0;
        sprite.interactive = !busy() && !opened_;
    }
    for (size_t i = 0; i < rounds_.size(); ++i) {
        auto& sprite = mutableObject("notes_" + std::string(i < 9 ? "0" : "") + std::to_string(i + 1));
        sprite.alpha = i < round_ ? 1 : 0;
        if (phase_ == Phase::NotesFade && i + 1 == round_)
            sprite.alpha = float(std::clamp(1 - remaining_, 0., 1.));
        sprite.interactive = false;
    }
    auto& lid = mutableObject("piano_open");
    lid.visible = opened_;
    lid.alpha = float(openAge_);
    auto& item = mutableObject("reward");
    item.visible = opened_ && !collected_;
    item.enabled = item.visible;
    item.alpha = float(openAge_);
    item.interactive = item.visible;
}
void Piano::releaseNote() {
    soundingKey_.reset();
}
void Piano::startNote(unsigned index, bool correct) {
    soundingKey_ = index;
    correctNote_ = correct;
    remaining_ = correct ? soundDurations_.at(index) : errorSeconds_;
    sounds_.push_back(correct ? soundName(index) : base_ + "/sounds/error");
    updateVisuals();
}
void Piano::demo() {
    phase_ = Phase::Demo;
    queuedKeys_.clear();
    progress_ = 0;
    demoRound_ = round_ < rounds_.size() ? round_ : 0;
    demoNote_ = 0;
    startNote(rounds_[demoRound_][demoNote_], true);
}
bool Piano::press(unsigned index) {
    if (index < minimumKey_ || index > maximumKey_ || busy() || opened_)
        return false;
    if (phase_ == Phase::Idle) {
        demo();
        return true;
    }
    if (phase_ == Phase::ReproduceKey) {
        queuedKeys_.push_back(index);
        return true;
    }
    phase_ = Phase::ReproduceKey;
    startNote(index, index == rounds_[round_][progress_]);
    return true;
}
void Piano::open() {
    opened_ = true;
    phase_ = Phase::Idle;
    remaining_ = 0;
    openAge_ = 0;
    queuedKeys_.clear();
    sounds_.push_back("sound/sfx/mg/skulls_obey/mg_open_lid");
    updateVisuals();
}
void Piano::advance() {
    releaseNote();
    if (phase_ == Phase::Idle) {
        demo();
        return;
    }
    if (phase_ == Phase::Demo) {
        if (++demoNote_ == rounds_[demoRound_].size()) {
            if (round_ < rounds_.size()) {
                phase_ = Phase::Reproduce;
                progress_ = 0;
                remaining_ = 0;
                updateVisuals();
                return;
            }
            if (++demoRound_ == rounds_.size()) {
                open();
                return;
            }
            demoNote_ = 0;
        }
        startNote(rounds_[demoRound_][demoNote_], true);
        return;
    }
    if (phase_ == Phase::NotesFade) {
        demo();
        return;
    }
    if (!correctNote_) {
        enter();
        return;
    }
    if (++progress_ == rounds_[round_].size()) {
        ++round_;
        phase_ = Phase::NotesFade;
        remaining_ = 1;
        queuedKeys_.clear();
        updateVisuals();
        return;
    }
    phase_ = Phase::Reproduce;
    remaining_ = 0;
    if (!queuedKeys_.empty()) {
        const auto key = queuedKeys_.front();
        queuedKeys_.pop_front();
        press(key);
    }
    updateVisuals();
}
void Piano::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid piano time");
    while (seconds > 0) {
        if (opened_) {
            openAge_ = std::min(1., openAge_ + seconds);
            break;
        }
        if (phase_ == Phase::Reproduce || (phase_ == Phase::Idle && autoplaySeconds == 0))
            break;
        const auto step = std::min(remaining_, seconds);
        seconds -= step;
        remaining_ -= step;
        if (remaining_ <= 0)
            advance();
    }
    updateVisuals();
}
bool Piano::takeReward() {
    if (!opened_ || collected_)
        return false;
    collected_ = true;
    updateVisuals();
    return true;
}
bool Piano::skip() {
    if (opened_)
        return false;
    // Native 4F76C0 finishes the round counter, then sends Open/Reinit. The
    // reward remains on the piano and still needs the original pickup callback.
    releaseNote();
    sounds_.clear();
    queuedKeys_.clear();
    round_ = rounds_.size();
    progress_ = demoNote_ = 0;
    demoRound_ = rounds_.size();
    correctNote_ = false;
    open();
    return true;
}
std::vector<std::string> Piano::takeSounds() {
    return std::exchange(sounds_, {});
}
} // namespace deadtime
