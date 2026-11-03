#include "rats.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <set>
#include <stdexcept>
#include <utility>

namespace deadtime {
std::string Rats::number(size_t index) {
    return std::string(index < 10 ? "0" : "") + std::to_string(index);
}
Rats::Rats(const Assets& assets, const std::string& path) {
    const auto base = normalize(path), bytes = assets.read(base + "/_config.xml");
    pugi::xml_document doc;
    if (!doc.load_buffer(bytes.data(), bytes.size()) || !doc.child("rats_puzzle"))
        throw std::runtime_error("Invalid rats configuration");
    const auto root = doc.child("rats_puzzle");
    const auto ratCount = root.child("rats").attribute("quantity").as_uint(),
               beamCount = root.child("beams").attribute("quantity").as_uint();
    const auto mirrorCount = root.child("mirrors").attribute("quantity").as_uint();
    if (!ratCount || ratCount > 10 || !beamCount || beamCount > 100 || !mirrorCount || mirrorCount > 10)
        throw std::runtime_error("Invalid rats/mirror/beam count");
    gone_.resize(ratCount);
    beams_.resize(beamCount);
    mirrors_.resize(mirrorCount);
    windowBeam_ = root.child("window").attribute("beam_global_index").as_uint(100);
    windowTarget_ = root.child("window").attribute("target_mirror").as_uint(10);
    if (windowBeam_ >= beams_.size() || windowTarget_ >= mirrors_.size())
        throw std::runtime_error("Invalid rats window beam");
    size_t index = 0;
    for (auto node : root.child("mirrors").children("mirror")) {
        if (index >= mirrors_.size() || node.attribute("index").as_int(-1) != int(index))
            throw std::runtime_error("Invalid rats mirror index");
        auto& mirror = mirrors_[index++];
        const auto count = node.attribute("num_states").as_uint();
        mirror.state = node.attribute("initial_state").as_uint(5);
        if (!count || count > 5 || mirror.state >= count)
            throw std::runtime_error("Invalid rats mirror state");
        for (auto state : node.children("state")) {
            if (state.attribute("index").as_int(-1) != int(mirror.states.size()))
                throw std::runtime_error("Invalid rats state index");
            std::vector<Rule> rules;
            for (auto beam : state.children("beam")) {
                Rule rule{parseTarget(beam.attribute("from").value()),
                          parseTarget(beam.attribute("to").value()),
                          beam.attribute("global_index").as_uint(100)};
                if ((rule.from.kind != Kind::Window && rule.from.kind != Kind::Mirror) ||
                    rule.to.kind == Kind::Window || rule.beam >= beams_.size())
                    throw std::runtime_error("Invalid rats reflection rule");
                if (std::any_of(rules.begin(), rules.end(),
                                [&](const auto& previous) { return previous.from == rule.from; }))
                    throw std::runtime_error("Duplicate rats reflection source");
                rules.push_back(rule);
            }
            if (rules.empty())
                throw std::runtime_error("Empty rats reflection state");
            mirror.states.push_back(std::move(rules));
        }
        if (mirror.states.size() != count)
            throw std::runtime_error("Rats mirror state count mismatch");
    }
    if (index != mirrors_.size())
        throw std::runtime_error("Rats mirror count mismatch");
    const std::string script = root.child("onFinish").attribute("script").value();
    if (!script.empty()) {
        onFinish = resolve(base, script) + ".lua";
        if (!assets.contains(onFinish))
            throw std::runtime_error("Missing rats completion script");
    }
    objects_ = loadActivityLayout(assets, base + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (const auto* id : {"back", "exit", "seeds"})
        object(id);
    for (size_t i = 0; i < mirrors_.size(); ++i)
        for (size_t j = 0; j < mirrors_[i].states.size(); ++j)
            object("mirror_" + number(i) + "_" + number(j));
    for (size_t i = 0; i < gone_.size(); ++i)
        object("rat_" + number(i));
    for (size_t i = 0; i < beams_.size(); ++i)
        object("beam_" + number(i));
    enter();
}
Rats::Target Rats::parseTarget(const std::string& text) const {
    if (text == "window")
        return {Kind::Window};
    if (text == "wall")
        return {Kind::Wall};
    const auto at = text.find('_');
    if (at == std::string::npos || at + 1 == text.size() ||
        !std::all_of(text.begin() + at + 1, text.end(), [](char c) { return c >= '0' && c <= '9'; }))
        throw std::runtime_error("Invalid rats reflection target: " + text);
    const auto index = std::stoul(text.substr(at + 1));
    if (text.starts_with("mirror_") && index < mirrors_.size())
        return {Kind::Mirror, index};
    if (text.starts_with("rat_") && index < gone_.size())
        return {Kind::Rat, index};
    throw std::runtime_error("Unknown rats reflection target: " + text);
}
const ActivityObject& Rats::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing rats object: " + id);
    return *found;
}
ActivityObject& Rats::mutableObject(const std::string& id) {
    return const_cast<ActivityObject&>(object(id));
}
const ActivityObject& Rats::mirror(size_t index) const {
    return object("mirror_" + number(index) + "_" + number(state(index)));
}
size_t Rats::goneCount() const {
    return size_t(std::count(gone_.begin(), gone_.end(), true));
}
void Rats::reflect() {
    retainedRat_.reset();
    std::fill(beams_.begin(), beams_.end(), false);
    size_t beam = windowBeam_;
    Target from{Kind::Window}, to{Kind::Mirror, windowTarget_};
    std::set<std::pair<size_t, size_t>> visited;
    for (;;) {
        beams_[beam] = true;
        if (to.kind == Kind::Rat) {
            if (!gone_[to.index]) {
                gone_[to.index] = true;
                fading_ = to.index;
                retainedRat_ = to.index;
                fadeAge_ = 0;
                events_.push_back(
                    {RatsEvent::Sound, std::string("sound/sfx/mg/rats/") +
                                           (ratSound_ ? "mg_rat_gone_with_scrape" : "mg_rat_gone")});
                ratSound_ ^= 1;
                if (goneCount() == gone_.size()) {
                    complete_ = true;
                    events_.push_back({RatsEvent::Finish, {}});
                    events_.push_back({RatsEvent::ClearHint, {}});
                } else
                    events_.push_back({RatsEvent::Hint,
                                       "puzzles/rats/hints/" +
                                           std::string(goneCount() == 1 ? "two_rats_left" : "one_rat_left")});
            }
            break;
        }
        if (to.kind != Kind::Mirror)
            break;
        if (!visited.emplace(beam, to.index).second)
            throw std::runtime_error("Cyclic rats reflection path");
        const auto& mirror = mirrors_[to.index];
        const auto& rules = mirror.states.at(mirror.state);
        const auto rule =
            std::find_if(rules.begin(), rules.end(), [&](const auto& rule) { return rule.from == from; });
        if (rule == rules.end())
            break;
        from = to;
        to = rule->to;
        beam = rule->beam;
    }
}
bool Rats::turn(size_t index) {
    if (busy() || index >= mirrors_.size())
        return false;
    auto& mirror = mirrors_[index];
    if (mirror.states.size() > 1) {
        if ((mirror.forward && mirror.state + 1 >= mirror.states.size()) ||
            (!mirror.forward && mirror.state == 0))
            throw std::runtime_error("Rats mirror direction outside configured states");
        if (mirror.forward)
            ++mirror.state;
        else
            --mirror.state;
        if (mirror.state == 0 || mirror.state + 1 == mirror.states.size())
            mirror.forward = !mirror.forward;
        events_.push_back(
            {RatsEvent::Sound, std::string("sound/sfx/mg/rats/mg_mirror_move") + (mirrorSound_ ? "2" : "1")});
        mirrorSound_ ^= 1;
    }
    reflect();
    updateVisuals();
    return true;
}
void Rats::idleClick(bool seeds) {
    if (busy())
        return;
    const auto count = goneCount();
    if (seeds)
        events_.push_back({RatsEvent::Hint, "puzzles/rats/hints/sack_of_grain_clicked"});
    else if (count < 3)
        events_.push_back(
            {RatsEvent::Hint, "puzzles/rats/hints/" + std::string(count == 0   ? "three_rats_left"
                                                                  : count == 1 ? "two_rats_left"
                                                                               : "one_rat_left")});
}
void Rats::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid rats tick duration");
    if (!fading_)
        return;
    fadeAge_ += seconds;
    if (fadeAge_ >= 2.5) {
        fading_.reset();
        if (complete_)
            events_.push_back({RatsEvent::Sound, "sound/sfx/mg/mg_finish"});
    }
    updateVisuals();
}
void Rats::leave() {
    fading_.reset();
    retainedRat_.reset();
    updateVisuals();
}
bool Rats::skip() {
    if (complete_)
        return false;
    // Native 4CC510 sends Finish and Reinit, bypassing the current rat fade.
    events_.clear();
    fading_.reset();
    retainedRat_.reset();
    fadeAge_ = 0;
    std::fill(gone_.begin(), gone_.end(), true);
    complete_ = true;
    events_.push_back({RatsEvent::Finish, {}});
    events_.push_back({RatsEvent::Sound, "sound/sfx/mg/mg_finish"});
    updateVisuals();
    return true;
}
void Rats::enter() {
    fading_.reset();
    reflect();
    updateVisuals();
}
void Rats::updateVisuals() {
    for (size_t i = 0; i < mirrors_.size(); ++i)
        for (size_t j = 0; j < mirrors_[i].states.size(); ++j)
            mutableObject("mirror_" + number(i) + "_" + number(j)).visible = mirrors_[i].state == j;
    for (size_t i = 0; i < gone_.size(); ++i) {
        auto& rat = mutableObject("rat_" + number(i));
        // Native fade only changes alpha; the next reflection initialization
        // hides already-gone rats. Keep that transparent hit node until then.
        rat.visible = !gone_[i] || retainedRat_ == i;
        rat.alpha = fading_ == i ? float(std::clamp(1 - fadeAge_ / 2.5, 0., 1.)) : gone_[i] ? 0.f : 1.f;
    }
    for (size_t i = 0; i < beams_.size(); ++i)
        mutableObject("beam_" + number(i)).visible = beams_[i];
}
std::vector<RatsEvent> Rats::takeEvents() {
    return std::exchange(events_, {});
}
} // namespace deadtime
