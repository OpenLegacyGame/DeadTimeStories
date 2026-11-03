#include "cellar_doors.hpp"
#include "audio_timing.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>
#include <utility>

namespace deadtime {
bool CellarDoors::skip() {
    if (complete_)
        return false;
    // Finish/Reinit (4D2030) bypasses reveal, pronunciation and letter fades.
    phase_ = Phase::Idle;
    age_ = watchSeconds_ = 0;
    revealed_ = complete_ = true;
    resultCorrect_ = false;
    resultDelay_.reset();
    selected_.clear();
    fading_.clear();
    events_.clear();
    std::fill(highlightAge_.begin(), highlightAge_.end(), -1);
    std::fill(fadeStart_.begin(), fadeStart_.end(), 0);
    events_.push_back({CellarEvent::Finish, {}});
    updateVisuals();
    return true;
}
CellarDoors::CellarDoors(const Assets& assets, const std::string& path, uint32_t seed) : random_(seed) {
    const auto base = normalize(path), bytes = assets.read(base + "/_config.xml");
    pugi::xml_document doc;
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("cellar_doors"))
        throw std::runtime_error("Invalid cellar doors configuration");
    const auto root = doc.child("cellar_doors");
    for (auto node : root.child("syllables").children("syllable")) {
        if (node.attribute("index").as_int(-1) != int(syllables_.size()) || syllables_.size() >= 20)
            throw std::runtime_error("Invalid cellar syllable index");
        const std::string word = node.attribute("name").value();
        const auto sound = resolve(base, node.attribute("sound").value());
        if (word.empty() || std::find(syllables_.begin(), syllables_.end(), word) != syllables_.end() ||
            !assets.contains(sound + ".ogg"))
            throw std::runtime_error("Invalid cellar syllable");
        syllables_.push_back(word);
        sounds_.push_back(sound);
    }
    const std::string word = root.child("word").text().as_string();
    for (size_t start = 0; start < word.size();) {
        const auto end = word.find('-', start);
        const auto part = word.substr(start, end == std::string::npos ? end : end - start);
        const auto found = std::find(syllables_.begin(), syllables_.end(), part);
        if (found == syllables_.end())
            throw std::runtime_error("Unknown cellar answer syllable");
        answer_.push_back(size_t(found - syllables_.begin()));
        if (end == std::string::npos)
            break;
        start = end + 1;
        if (start == word.size())
            throw std::runtime_error("Empty cellar answer syllable");
    }
    if (answer_.empty())
        throw std::runtime_error("Empty cellar answer");
    for (auto node : root.child("incorrect_word_sounds").children("sound")) {
        const auto sound = resolve(base, node.attribute("src").value());
        if (!assets.contains(sound + ".ogg"))
            throw std::runtime_error("Missing cellar wrong-word sound");
        wrongSounds_.push_back(sound);
    }
    if (wrongSounds_.empty())
        throw std::runtime_error("Empty cellar wrong-word sounds");
    wrongDelay_ = root.child("incorrect_word_fade_delay").attribute("ms").as_double() / 1000.;
    if (!std::isfinite(wrongDelay_) || wrongDelay_ < 0)
        throw std::runtime_error("Invalid cellar wrong-word delay");
    correctSoundSeconds_ = vorbisDuration(assets, "sound/sfx/mg/cellar_doors/mg_salvation.ogg");
    if (correctSoundSeconds_ <= 0)
        throw std::runtime_error("Empty cellar completion sound");
    const std::string script = root.child("onFinish").attribute("script").value();
    if (!script.empty()) {
        onFinish = resolve(base, script) + ".lua";
        if (!assets.contains(onFinish))
            throw std::runtime_error("Missing cellar completion script");
    }
    objects_ = loadActivityLayout(assets, base + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (const auto* id : {"back", "exit", "cellar_doors_mask", "letters_covered", "fx_ash"})
        object(id);
    highlightAge_.resize(syllables_.size(), -1);
    fadeStart_.resize(syllables_.size());
    for (size_t i = 0; i < syllables_.size(); ++i) {
        syllable(i);
        object(name(i, true));
    }
    updateVisuals();
}
std::string CellarDoors::name(size_t index, bool highlight) const {
    return "syllable_" + std::string(index < 10 ? "0" : "") + std::to_string(index) +
           (highlight ? "_hl" : "");
}
const ActivityObject& CellarDoors::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing cellar object: " + id);
    return *found;
}
ActivityObject& CellarDoors::mutableObject(const std::string& id) {
    return const_cast<ActivityObject&>(object(id));
}
const ActivityObject& CellarDoors::syllable(size_t index) const {
    if (index >= syllables_.size())
        throw std::runtime_error("Unknown cellar syllable");
    return object(name(index));
}
void CellarDoors::hint(const std::string& key) {
    events_.push_back({CellarEvent::Hint, "puzzles/cellar_doors/hints/" + key});
}
void CellarDoors::idleClick() {
    if (!busy() && !revealed_)
        hint("idle_click_letters_not_revealed");
}
void CellarDoors::wrongItem() {
    if (!busy())
        hint(revealed_ ? "use_wrong_item_after_reveal_letters" : "use_wrong_item_before_reveal_letters");
}
bool CellarDoors::applyAsh() {
    if (busy() || revealed_)
        return false;
    revealed_ = true;
    phase_ = Phase::Reveal;
    age_ = 0;
    events_.push_back({CellarEvent::Sound, "sound/sfx/mg/cellar_doors/mg_ashes_use"});
    updateVisuals();
    return true;
}
bool CellarDoors::select(size_t index) {
    if (busy() || !revealed_ || index >= syllables_.size() ||
        std::find(selected_.begin(), selected_.end(), index) != selected_.end())
        return false;
    selected_.push_back(index);
    highlightAge_[index] = 0;
    events_.push_back({CellarEvent::Sound, sounds_[index]});
    // The native callback belongs to the Nth highlight, not the sound. Extra
    // unique clicks during its one-second fade are still accepted by Idle.
    if (selected_.size() == answer_.size())
        resultDelay_ = 1.;
    updateVisuals();
    return true;
}
void CellarDoors::watch() {
    resultDelay_.reset();
    fading_ = selected_;
    phase_ = Phase::Watch;
    age_ = 0;
    resultCorrect_ = selected_ == answer_;
    if (resultCorrect_) {
        complete_ = true;
        events_.push_back({CellarEvent::Finish, {}});
        events_.push_back({CellarEvent::Sound, "sound/sfx/mg/cellar_doors/mg_salvation"});
        watchSeconds_ = correctSoundSeconds_;
    } else {
        selected_.clear();
        events_.push_back(
            {CellarEvent::Sound,
             wrongSounds_[std::uniform_int_distribution<size_t>(0, wrongSounds_.size() - 1)(random_)]});
        watchSeconds_ = wrongDelay_;
    }
}
void CellarDoors::fade() {
    if (!resultCorrect_)
        hint("incorrect_word");
    phase_ = Phase::Fade;
    age_ = 0;
    for (auto index : fading_)
        fadeStart_[index] = std::clamp(float(highlightAge_[index]), 0.f, 1.f);
}
void CellarDoors::advance(double seconds) {
    age_ += seconds;
    for (auto& age : highlightAge_)
        if (age >= 0)
            age += seconds;
    if (resultDelay_)
        *resultDelay_ = std::max(0., *resultDelay_ - seconds);
}
void CellarDoors::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid cellar tick duration");
    // Split a large tick at callbacks so highlight/fade timings stay ordered.
    for (unsigned guard = 0; guard < 8; ++guard) {
        std::optional<double> until;
        if (phase_ == Phase::Idle)
            until = resultDelay_;
        else
            until = std::max(0., (phase_ == Phase::Watch ? watchSeconds_ : 1.5) - age_);
        if (!until || seconds < *until) {
            advance(seconds);
            break;
        }
        advance(*until);
        seconds -= *until;
        if (phase_ == Phase::Idle)
            watch();
        else if (phase_ == Phase::Reveal) {
            phase_ = Phase::Idle;
            age_ = 0;
            hint("letters_revealed");
        } else if (phase_ == Phase::Watch)
            fade();
        else {
            for (auto index : fading_)
                highlightAge_[index] = -1;
            fading_.clear();
            phase_ = Phase::Idle;
            age_ = 0;
        }
    }
    updateVisuals();
}
void CellarDoors::enter() {
    phase_ = Phase::Idle;
    age_ = 0;
    resultDelay_.reset();
    fading_.clear();
    if (selected_ == answer_)
        selected_.clear();
    std::fill(highlightAge_.begin(), highlightAge_.end(), -1);
    for (auto index : selected_)
        highlightAge_[index] = 1;
    updateVisuals();
}
void CellarDoors::updateVisuals() {
    auto& covered = mutableObject("letters_covered");
    covered.visible = !revealed_ || phase_ == Phase::Reveal;
    covered.alpha = phase_ == Phase::Reveal ? float(std::clamp(1 - age_ / 1.5, 0., 1.)) : 1.f;
    mutableObject("cellar_doors_mask").interactive = !revealed_;
    mutableObject("fx_ash").visible = revealed_;
    for (size_t i = 0; i < syllables_.size(); ++i) {
        mutableObject(name(i)).interactive = revealed_;
        auto& highlight = mutableObject(name(i, true));
        highlight.alpha = highlightAge_[i] < 0 ? 0.f : std::clamp(float(highlightAge_[i]), 0.f, 1.f);
        if (phase_ == Phase::Fade && std::find(fading_.begin(), fading_.end(), i) != fading_.end())
            highlight.alpha = fadeStart_[i] * float(std::clamp(1 - age_ / 1.5, 0., 1.));
    }
}
std::vector<CellarEvent> CellarDoors::takeEvents() {
    return std::exchange(events_, {});
}
} // namespace deadtime
