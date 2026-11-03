#include "activity_checkpoints.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
namespace deadtime {
namespace {
void require(bool ok, const std::string& message) {
    if (!ok)
        throw std::runtime_error("Activity checkpoint: " + message);
}
double number(pugi::xml_node node, const char* key) {
    require(bool(node.attribute(key)), std::string("missing ") + key);
    const std::string text = node.attribute(key).value();
    size_t used = 0;
    auto value = std::stod(text, &used);
    require(used == text.size() && std::isfinite(value), "invalid number");
    return value;
}
bool boolean(pugi::xml_node node, const char* key) {
    const std::string value = node.attribute(key).as_string();
    require(value == "true" || value == "false", std::string("invalid boolean ") + key);
    return value == "true";
}
size_t index(pugi::xml_node node, const char* key, size_t maximum) {
    const auto value = number(node, key);
    require(value >= 0 && value == std::trunc(value) && value <= double(maximum),
            std::string("invalid index ") + key);
    return size_t(value);
}
void visuals(const std::vector<ActivityObject>& objects, pugi::xml_node node) {
    for (size_t i = 0; i < objects.size(); ++i) {
        const auto& object = objects[i];
        auto entry = node.append_child("visual");
        entry.append_attribute("id") = object.id.c_str();
        entry.append_attribute("texture") = object.texture.c_str();
        entry.append_attribute("index").set_value(static_cast<unsigned long long>(i));
#define WRITE(field) entry.append_attribute(#field).set_value(object.field)
        WRITE(x);
        WRITE(y);
        WRITE(z);
        WRITE(width);
        WRITE(height);
        WRITE(angle);
        WRITE(alpha);
        WRITE(scale);
        WRITE(pivotX);
        WRITE(pivotY);
        WRITE(visible);
        WRITE(interactive);
        WRITE(enabled);
#undef WRITE
    }
}
void restoreVisuals(std::vector<ActivityObject>& objects, pugi::xml_node node) {
    std::set<size_t> seen;
    for (auto entry : node.children("visual")) {
        const std::string id = entry.attribute("id").as_string();
        require(!objects.empty(), "empty activity visuals");
        const auto i = index(entry, "index", objects.size() - 1);
        require(objects[i].id == id && seen.insert(i).second, "unknown or duplicate visual");
        auto& object = objects[i];
        object.texture = entry.attribute("texture").as_string();
#define FLOAT(field)                                                                                         \
    {                                                                                                        \
        const auto value = number(entry, #field);                                                            \
        require(std::abs(value) <= std::numeric_limits<float>::max(), "visual float overflow");              \
        object.field = float(value);                                                                         \
    }
        FLOAT(x);
        FLOAT(y);
        FLOAT(z);
        FLOAT(width);
        FLOAT(height);
        FLOAT(angle);
        FLOAT(alpha);
        FLOAT(scale);
        FLOAT(pivotX);
        FLOAT(pivotY);
#undef FLOAT
        require(object.width >= 0 && object.height >= 0 && object.alpha >= 0 && object.alpha <= 1 &&
                    object.scale > 0,
                "invalid visual bounds");
        object.visible = boolean(entry, "visible");
        object.interactive = boolean(entry, "interactive");
        object.enabled = boolean(entry, "enabled");
    }
    require(seen.size() == objects.size(), "incomplete visuals");
}
void effect(const Object& object, pugi::xml_node entry) {
    entry.append_attribute("id") = object.id.c_str();
#define WRITE(field) entry.append_attribute(#field).set_value(object.field)
    WRITE(x);
    WRITE(y);
    WRITE(z);
    WRITE(alpha);
    WRITE(width);
    WRITE(height);
    WRITE(treeTime);
    WRITE(particlePresimPending);
    WRITE(visible);
    WRITE(enabled);
    WRITE(interactive);
    WRITE(treePlaying);
    WRITE(treeLooped);
    WRITE(treePlayOption);
    WRITE(treeLoopOverridden);
    WRITE(particleGenerating);
    WRITE(particlesPresimRequested);
#undef WRITE
}
void restoreEffect(Object& object, pugi::xml_node entry) {
    require(object.id == entry.attribute("id").as_string(), "unknown effect object");
#define FLOAT(field)                                                                                         \
    {                                                                                                        \
        const auto value = number(entry, #field);                                                            \
        require(std::abs(value) <= std::numeric_limits<float>::max(), "effect float overflow");              \
        object.field = float(value);                                                                         \
    }
    FLOAT(x);
    FLOAT(y);
    FLOAT(z);
    FLOAT(alpha);
    FLOAT(width);
    FLOAT(height);
#undef FLOAT
    object.treeTime = number(entry, "treeTime");
    object.particlePresimPending = number(entry, "particlePresimPending");
    require(object.width >= 0 && object.height >= 0 && object.alpha >= 0 && object.alpha <= 1 &&
                object.treeTime >= 0 && object.particlePresimPending >= 0,
            "invalid effect bounds/time");
#define BOOL(field) object.field = boolean(entry, #field)
    BOOL(visible);
    BOOL(enabled);
    BOOL(interactive);
    BOOL(treePlaying);
    BOOL(treeLooped);
    BOOL(treePlayOption);
    BOOL(treeLoopOverridden);
    BOOL(particleGenerating);
    BOOL(particlesPresimRequested);
#undef BOOL
}

} // namespace
void ActivityCheckpoints::save(const OuijiBoard& activity, pugi::xml_node node) {
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(opening_);
    WRITE(claimed_);
    WRITE(solved_);
    WRITE(fadePhase_);
    WRITE(fadeAge_);
    WRITE(age_);
    WRITE(duration_);
    WRITE(startX_);
    WRITE(startY_);
    WRITE(startAngle_);
    WRITE(targetX_);
    WRITE(targetY_);
    WRITE(targetAngle_);
    WRITE(moving_);
    WRITE(exitRequested);
    WRITE(rewarded);
#undef WRITE
    node.append_attribute("entered") = activity.entered_.c_str();
    node.append_attribute("selected") = activity.selected_.c_str();
    node.append_attribute("feedback") = activity.feedback.c_str();
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(OuijiBoard& activity, pugi::xml_node node) {
#define BOOL(field) activity.field = boolean(node, #field)
    BOOL(opening_);
    BOOL(claimed_);
    BOOL(solved_);
    BOOL(moving_);
    BOOL(exitRequested);
    BOOL(rewarded);
#undef BOOL
#define NUMBER(field) activity.field = number(node, #field)
    NUMBER(fadeAge_);
    NUMBER(age_);
    NUMBER(duration_);
#undef NUMBER
#define FLOAT(field)                                                                                         \
    {                                                                                                        \
        const auto value = number(node, #field);                                                             \
        require(std::abs(value) <= std::numeric_limits<float>::max(), "pointer float overflow");             \
        activity.field = float(value);                                                                       \
    }
    FLOAT(startX_);
    FLOAT(startY_);
    FLOAT(startAngle_);
    FLOAT(targetX_);
    FLOAT(targetY_);
    FLOAT(targetAngle_);
#undef FLOAT
    activity.fadePhase_ = unsigned(index(node, "fadePhase_", activity.fades_.size()));
    activity.entered_ = node.attribute("entered").as_string();
    activity.selected_ = node.attribute("selected").as_string();
    activity.feedback = node.attribute("feedback").as_string();
    require(activity.phrase_.starts_with(activity.entered_), "invalid entered phrase");
    // Native skip sets solved while retaining any prefix already typed.
    require(activity.solved_ || activity.entered_ != activity.phrase_, "inconsistent solved phrase");
    require(!activity.claimed_ || activity.solved_, "claimed unsolved book");
    require(!activity.rewarded || activity.claimed_, "rewarded unclaimed book");
    require(activity.age_ >= 0 && activity.duration_ >= 0 && activity.fadeAge_ >= 0,
            "negative animation time");
    if (activity.opening_)
        require(activity.solved_ && !activity.claimed_ && !activity.moving_ &&
                    activity.fadePhase_ < activity.fades_.size() &&
                    activity.fadeAge_ < activity.fades_[activity.fadePhase_]->duration(),
                "invalid book opening");
    if (activity.moving_)
        require(!activity.solved_ && !activity.opening_ &&
                    (activity.duration_ == 0 || activity.age_ < activity.duration_),
                "invalid pointer motion");
    if (!activity.selected_.empty())
        activity.object(activity.selected_);
    if (activity.moving_)
        require(!activity.selected_.empty(), "missing pointer target");
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
void ActivityCheckpoints::save(const SpellMixing& activity, pugi::xml_node node) {
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(bottlePlaced_);
    WRITE(mixtureReady_);
    WRITE(stirring_);
    WRITE(bottleFilled_);
    WRITE(complete_);
    WRITE(round_);
    WRITE(gestureIndex_);
    WRITE(mixingAge_);
    WRITE(switchGrace_);
#undef WRITE
    node.append_attribute("feedback") = activity.feedback.c_str();
    std::ostringstream random;
    random << activity.random_;
    node.append_child("random").text().set(random.str().c_str());
    for (const auto& [item, slot] : activity.placement_) {
        auto entry = node.append_child("placed");
        entry.append_attribute("item") = item.c_str();
        entry.append_attribute("slot").set_value(static_cast<unsigned long long>(slot));
    }
    for (const auto& item : activity.mortar_)
        node.append_child("mortar").append_attribute("item") = item.c_str();
    auto gesture = node.append_child("gesture");
#define WRITE(field) gesture.append_attribute(#field).set_value(activity.gesture_.field)
    WRITE(first_);
    WRITE(previous_);
    WRITE(clockwise_);
    WRITE(started_);
#undef WRITE
    gesture.append_attribute("result") = int(activity.gesture_.result_);
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(SpellMixing& activity, pugi::xml_node node) {
#define BOOL(field) activity.field = boolean(node, #field)
    BOOL(bottlePlaced_);
    BOOL(mixtureReady_);
    BOOL(stirring_);
    BOOL(bottleFilled_);
    BOOL(complete_);
#undef BOOL
    activity.round_ = index(node, "round_", activity.rounds_.size() - 1);
    activity.gestureIndex_ = index(node, "gestureIndex_", activity.rounds_[activity.round_].gestures.size());
    activity.mixingAge_ = number(node, "mixingAge_");
    activity.switchGrace_ = number(node, "switchGrace_");
    require(activity.mixingAge_ >= 0 && activity.switchGrace_ >= 0 &&
                activity.switchGrace_ <= activity.switchedFailureSeconds,
            "invalid gesture timing");
    activity.feedback = node.attribute("feedback").as_string();
    std::istringstream random(node.child_value("random"));
    require(bool(random >> activity.random_), "invalid random state");
    random >> std::ws;
    require(random.eof(), "trailing random state");
    std::set<size_t> slots;
    for (auto entry : node.children("placed")) {
        const std::string item = entry.attribute("item").as_string();
        const auto found = activity.ingredients_.find(item);
        const auto slot = index(entry, "slot", activity.slots_.size() - 1);
        require(found != activity.ingredients_.end() && !found->second.bottle && slots.insert(slot).second &&
                    activity.placement_.emplace(item, slot).second,
                "invalid ingredient placement");
    }
    std::set<std::string> allowed;
    for (size_t round = 0; round <= activity.round_; ++round)
        for (const auto& item : activity.rounds_[round].ingredients)
            allowed.insert(item);
    for (auto entry : node.children("mortar")) {
        const std::string item = entry.attribute("item").as_string();
        require(allowed.contains(item) && activity.placement_.contains(item) &&
                    activity.mortar_.insert(item).second,
                "invalid mortar ingredient");
    }
    for (size_t round = 0; round < activity.round_; ++round)
        for (const auto& item : activity.rounds_[round].ingredients)
            require(activity.mortar_.contains(item), "missing earlier-round ingredient");
    bool missing = false;
    for (const auto& item : activity.rounds_[activity.round_].ingredients) {
        if (!activity.mortar_.contains(item))
            missing = true;
        else
            require(!missing, "unordered current ingredients");
    }
    require(!activity.bottleFilled_ || (activity.mixtureReady_ && activity.bottlePlaced_),
            "invalid bottle fill state");
    require(!activity.complete_ || activity.bottleFilled_, "completed empty bottle");
    if (activity.mixtureReady_)
        require(activity.arranged() && activity.round_ + 1 == activity.rounds_.size() &&
                    activity.gestureIndex_ == activity.rounds_.back().gestures.size() &&
                    std::all_of(activity.rounds_.back().ingredients.begin(),
                                activity.rounds_.back().ingredients.end(),
                                [&](const auto& item) { return activity.mortar_.contains(item); }),
                "invalid finished mixture");
    else
        require(activity.gestureIndex_ < activity.rounds_[activity.round_].gestures.size(),
                "invalid gesture progress");
    const auto gesture = node.child("gesture");
    activity.gesture_.first_ = unsigned(index(gesture, "first_", 7));
    activity.gesture_.previous_ = unsigned(index(gesture, "previous_", 7));
    activity.gesture_.clockwise_ = boolean(gesture, "clockwise_");
    activity.gesture_.started_ = boolean(gesture, "started_");
    activity.gesture_.result_ = MixingGesture::Result(index(gesture, "result", 2));
    require(!activity.stirring_ || (activity.readyToStir() && activity.gesture_.started_ &&
                                    activity.gesture_.result_ == MixingGesture::Result::Active &&
                                    activity.gesture_.clockwise_ == (activity.direction() == "cw")),
            "invalid active stirring");
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
void ActivityCheckpoints::save(const HexBones& activity, pugi::xml_node node) {
    node.append_attribute("phase") = int(activity.phase_);
    node.append_attribute("current") = activity.current_.c_str();
    node.append_attribute("feedback") = activity.feedback.c_str();
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(thrown_);
    WRITE(bonesVisible_);
    WRITE(complete_);
    WRITE(rewarded);
    WRITE(age_);
    WRITE(swapFirst_);
    WRITE(swapSecond_);
#undef WRITE
    if (activity.selected_)
        node.append_attribute("selected").set_value(static_cast<unsigned long long>(*activity.selected_));
    for (size_t i = 0; i < activity.angles_.size(); ++i) {
        auto bone = node.append_child("bone");
        bone.append_attribute("angle") = activity.angles_[i];
        bone.append_attribute("depth") = activity.depths_[i];
    }
    for (const auto& sound : activity.sounds_)
        node.append_child("sound").text().set(sound.c_str());
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(HexBones& activity, pugi::xml_node node) {
    activity.phase_ = HexBones::Phase(index(node, "phase", 3));
    activity.current_ = node.attribute("current").as_string();
    activity.feedback = node.attribute("feedback").as_string();
    auto actual = activity.current_, expected = activity.phrase_;
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    require(actual == expected, "invalid bone permutation");
#define BOOL(field) activity.field = boolean(node, #field)
    BOOL(thrown_);
    BOOL(bonesVisible_);
    BOOL(complete_);
    BOOL(rewarded);
#undef BOOL
    activity.age_ = number(node, "age_");
    require(activity.age_ >= 0, "negative bone animation time");
    activity.swapFirst_ = index(node, "swapFirst_", activity.phrase_.size() - 1);
    activity.swapSecond_ = index(node, "swapSecond_", activity.phrase_.size() - 1);
    if (node.attribute("selected"))
        activity.selected_ = index(node, "selected", activity.phrase_.size() - 1);
    for (auto fixed : activity.fixed_)
        require(activity.locked(fixed), "moved fixed bone");
    require(activity.complete_ == (activity.current_ == activity.phrase_) &&
                (!activity.rewarded || activity.complete_),
            "inconsistent bone completion");
    require(activity.thrown_ || (!activity.bonesVisible_ && activity.phase_ == HexBones::Phase::Idle),
            "unthrown bone activity active");
    if (activity.selected_)
        require(!activity.locked(*activity.selected_), "selected locked bone");
    if (activity.phase_ == HexBones::Phase::Swap)
        require(activity.thrown_ && activity.bonesVisible_ && !activity.complete_ &&
                    activity.swapFirst_ != activity.swapSecond_ &&
                    activity.selected_ == activity.swapFirst_ && !activity.locked(activity.swapFirst_) &&
                    !activity.locked(activity.swapSecond_) && activity.age_ < activity.swapSeconds,
                "invalid bone swap");
    if (activity.phase_ == HexBones::Phase::FadeIn)
        require(activity.thrown_ && !activity.bonesVisible_ && activity.age_ < activity.fadeIn_.duration(),
                "invalid bone fade in");
    if (activity.phase_ == HexBones::Phase::FadeOut)
        require(activity.thrown_ && activity.bonesVisible_ && activity.age_ < activity.fadeOut_.duration(),
                "invalid bone fade out");
    activity.angles_.clear();
    activity.depths_.clear();
    for (auto bone : node.children("bone")) {
        const auto angle = number(bone, "angle"), depth = number(bone, "depth");
        require(std::abs(angle) <= std::numeric_limits<float>::max() &&
                    std::abs(depth) <= std::numeric_limits<float>::max(),
                "bone geometry overflow");
        activity.angles_.push_back(float(angle));
        activity.depths_.push_back(float(depth));
    }
    require(activity.angles_.size() == activity.phrase_.size(), "incomplete bone geometry");
    for (auto sound : node.children("sound"))
        activity.sounds_.emplace_back(sound.child_value());
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
    if (activity.phase_ == HexBones::Phase::Swap)
        activity.updateSwap();
}
void ActivityCheckpoints::save(const Piano& activity, pugi::xml_node node) {
    node.append_attribute("phase") = int(activity.phase_);
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(remaining_);
    WRITE(openAge_);
    WRITE(round_);
    WRITE(progress_);
    WRITE(demoRound_);
    WRITE(demoNote_);
    WRITE(opened_);
    WRITE(collected_);
    WRITE(correctNote_);
#undef WRITE
    if (activity.soundingKey_)
        node.append_attribute("sounding_key") = *activity.soundingKey_;
    for (auto key : activity.queuedKeys_)
        node.append_child("queued").append_attribute("key") = key;
    for (const auto& sound : activity.sounds_)
        node.append_child("sound").text().set(sound.c_str());
    for (const auto& [key, duration] : activity.soundDurations_) {
        auto entry = node.append_child("duration");
        entry.append_attribute("key") = key;
        entry.append_attribute("seconds") = duration;
    }
    node.append_attribute("error_seconds") = activity.errorSeconds_;
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(Piano& activity, pugi::xml_node node) {
    activity.phase_ = Piano::Phase(index(node, "phase", 4));
    activity.remaining_ = number(node, "remaining_");
    activity.openAge_ = number(node, "openAge_");
    activity.round_ = index(node, "round_", activity.rounds_.size());
    activity.demoRound_ = index(node, "demoRound_", activity.rounds_.size());
    activity.demoNote_ =
        index(node, "demoNote_",
              activity.rounds_[std::min(activity.demoRound_, activity.rounds_.size() - 1)].size());
    activity.progress_ = index(
        node, "progress_", activity.rounds_[std::min(activity.round_, activity.rounds_.size() - 1)].size());
    activity.opened_ = boolean(node, "opened_");
    activity.collected_ = boolean(node, "collected_");
    activity.correctNote_ = boolean(node, "correctNote_");
    require(activity.remaining_ >= 0 && activity.openAge_ >= 0 && activity.openAge_ <= 1 &&
                (!activity.collected_ || activity.opened_),
            "invalid piano timing/reward");
    auto key = [&](pugi::xml_node entry, const char* name) {
        const auto value = unsigned(index(entry, name, activity.maximumKey_));
        require(value >= activity.minimumKey_, "invalid piano key");
        return value;
    };
    if (node.attribute("sounding_key"))
        activity.soundingKey_ = key(node, "sounding_key");
    for (auto entry : node.children("queued"))
        activity.queuedKeys_.push_back(key(entry, "key"));
    require(activity.queuedKeys_.empty() || activity.phase_ == Piano::Phase::ReproduceKey,
            "queued keys outside reproduction");
    require(!activity.opened_ || (activity.round_ == activity.rounds_.size() &&
                                  activity.phase_ == Piano::Phase::Idle && !activity.soundingKey_),
            "inconsistent piano lid");
    if (activity.phase_ == Piano::Phase::Demo)
        require(!activity.opened_ && activity.demoRound_ < activity.rounds_.size() &&
                    activity.demoNote_ < activity.rounds_[activity.demoRound_].size() &&
                    activity.soundingKey_ == activity.rounds_[activity.demoRound_][activity.demoNote_],
                "invalid piano demo");
    require(activity.demoRound_ < activity.rounds_.size() || activity.opened_,
            "demo end outside opened piano");
    if (activity.phase_ == Piano::Phase::Reproduce || activity.phase_ == Piano::Phase::ReproduceKey)
        require(activity.round_ < activity.rounds_.size() &&
                    activity.progress_ < activity.rounds_[activity.round_].size(),
                "invalid reproduced melody progress");
    if (activity.phase_ == Piano::Phase::ReproduceKey)
        require(bool(activity.soundingKey_), "missing reproduced note");
    if (activity.phase_ == Piano::Phase::Demo || activity.phase_ == Piano::Phase::ReproduceKey)
        require(activity.remaining_ <= (activity.correctNote_
                                            ? activity.soundDurations_.at(*activity.soundingKey_)
                                            : activity.errorSeconds_),
                "invalid remaining note duration");
    if (activity.phase_ == Piano::Phase::Idle)
        require(activity.remaining_ <= activity.autoplaySeconds, "invalid autoplay delay");
    if (activity.phase_ == Piano::Phase::Reproduce)
        require(activity.remaining_ == 0 && !activity.soundingKey_, "invalid idle reproduction");
    if (activity.phase_ == Piano::Phase::NotesFade)
        require(activity.round_ > 0 && activity.remaining_ <= 1 && !activity.soundingKey_,
                "invalid sheet music fade");
    std::set<unsigned> durations;
    for (auto entry : node.children("duration")) {
        const auto note = key(entry, "key");
        require(durations.insert(note).second &&
                    std::abs(number(entry, "seconds") - activity.soundDurations_.at(note)) < 1e-9,
                "piano audio duration changed");
    }
    require(durations.size() == activity.soundDurations_.size() &&
                std::abs(number(node, "error_seconds") - activity.errorSeconds_) < 1e-9,
            "incomplete piano audio timings");
    for (auto sound : node.children("sound"))
        activity.sounds_.emplace_back(sound.child_value());
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
void ActivityCheckpoints::save(const WindowCleaning& activity, pugi::xml_node node) {
    node.append_attribute("phase") = int(activity.phase_);
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(mixturePlaced_);
    WRITE(wet_);
    WRITE(moved_);
    WRITE(inside_);
    WRITE(x_);
    WRITE(y_);
    WRITE(section_);
    WRITE(successRemaining_);
    WRITE(failureRemaining_);
    WRITE(idleAge_);
    WRITE(phaseAge_);
    WRITE(fxRemaining_);
    WRITE(rewarded);
#undef WRITE
    if (activity.hovered_)
        node.append_attribute("hovered").set_value(static_cast<unsigned long long>(*activity.hovered_));
    if (activity.floraAge_)
        node.append_attribute("flora_age") = *activity.floraAge_;
    std::string dirty;
    for (auto value : activity.dirty_)
        dirty += value ? '1' : '0';
    node.append_attribute("dirty") = dirty.c_str();
    std::ostringstream random;
    random << activity.random_;
    node.append_child("random").text().set(random.str().c_str());
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(WindowCleaning& activity, pugi::xml_node node) {
    activity.phase_ = WindowCleaning::Phase(index(node, "phase", 4));
#define BOOL(field) activity.field = boolean(node, #field)
    BOOL(mixturePlaced_);
    BOOL(wet_);
    BOOL(moved_);
    BOOL(inside_);
    BOOL(rewarded);
#undef BOOL
    auto coordinate = [&](const char* name) {
        const auto value = number(node, name);
        require(value == std::trunc(value) && value >= std::numeric_limits<int>::min() &&
                    value <= std::numeric_limits<int>::max(),
                "invalid sponge coordinate");
        return int(value);
    };
    activity.x_ = coordinate("x_");
    activity.y_ = coordinate("y_");
    activity.section_ = index(node, "section_", 9);
#define NUMBER(field) activity.field = number(node, #field)
    NUMBER(successRemaining_);
    NUMBER(failureRemaining_);
    NUMBER(idleAge_);
    NUMBER(phaseAge_);
    NUMBER(fxRemaining_);
#undef NUMBER
    require(activity.successRemaining_ >= 0 && activity.successRemaining_ <= activity.successSeconds &&
                activity.failureRemaining_ >= 0 && activity.failureRemaining_ <= activity.failureSeconds &&
                activity.idleAge_ >= 0 && activity.phaseAge_ >= 0 && activity.fxRemaining_ >= 0 &&
                activity.fxRemaining_ <= activity.removeFxSeconds,
            "invalid cleaning timing");
    if (node.attribute("hovered"))
        activity.hovered_ = index(node, "hovered", 9);
    if (node.attribute("flora_age")) {
        activity.floraAge_ = number(node, "flora_age");
        require(*activity.floraAge_ >= 0 && *activity.floraAge_ < 1, "invalid flora fade");
    }
    const std::string dirty = node.attribute("dirty").as_string();
    require(dirty.size() == 10 && dirty.find_first_not_of("01") == std::string::npos,
            "invalid dirty section set");
    for (size_t i = 0; i < 10; ++i)
        activity.dirty_[i] = dirty[i] == '1';
    require(!activity.rewarded || activity.complete(), "window rewarded before completion");
    const bool clean =
        std::none_of(activity.dirty_.begin(), activity.dirty_.end(), [](bool value) { return value; });
    require((!activity.complete() || clean) &&
                (!clean || activity.complete() || activity.phase_ == WindowCleaning::Phase::FadeDirty),
            "inconsistent clean window");
    if (activity.rubbing() || activity.phase_ == WindowCleaning::Phase::FadeClean)
        require(activity.dirty_[activity.section_], "rubbing an already clean section");
    if (activity.phase_ == WindowCleaning::Phase::FadeClean)
        require(activity.phaseAge_ < activity.disappear_.duration(), "expired cleaning fade");
    if (activity.phase_ == WindowCleaning::Phase::FadeDirty)
        require(!activity.dirty_[activity.section_] && activity.phaseAge_ < activity.reappear_.duration(),
                "invalid recontamination fade");
    std::istringstream random(node.child_value("random"));
    require(bool(random >> activity.random_), "invalid cleaning random state");
    random >> std::ws;
    require(random.eof(), "trailing cleaning random state");
    for (auto entry : node.children("event"))
        activity.events_.push_back({WindowCleaningEvent::Kind(index(entry, "kind", 2)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
}
void ActivityCheckpoints::save(const CellarDoors& activity, pugi::xml_node node) {
    node.append_attribute("phase") = int(activity.phase_);
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(age_);
    WRITE(watchSeconds_);
    WRITE(revealed_);
    WRITE(complete_);
    WRITE(resultCorrect_);
    WRITE(rewarded);
#undef WRITE
    node.append_attribute("correct_sound_seconds") = activity.correctSoundSeconds_;
    if (activity.resultDelay_)
        node.append_attribute("result_delay") = *activity.resultDelay_;
    for (auto value : activity.selected_)
        node.append_child("selected")
            .append_attribute("index")
            .set_value(static_cast<unsigned long long>(value));
    for (auto value : activity.fading_)
        node.append_child("fading").append_attribute("index").set_value(
            static_cast<unsigned long long>(value));
    for (size_t i = 0; i < activity.syllables_.size(); ++i) {
        auto entry = node.append_child("highlight");
        entry.append_attribute("age") = activity.highlightAge_[i];
        entry.append_attribute("start") = activity.fadeStart_[i];
    }
    std::ostringstream random;
    random << activity.random_;
    node.append_child("random").text().set(random.str().c_str());
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(CellarDoors& activity, pugi::xml_node node) {
    activity.phase_ = CellarDoors::Phase(index(node, "phase", 3));
    activity.age_ = number(node, "age_");
    activity.watchSeconds_ = number(node, "watchSeconds_");
#define BOOL(field) activity.field = boolean(node, #field)
    BOOL(revealed_);
    BOOL(complete_);
    BOOL(resultCorrect_);
    BOOL(rewarded);
#undef BOOL
    require(activity.age_ >= 0 && activity.watchSeconds_ >= 0 &&
                std::abs(number(node, "correct_sound_seconds") - activity.correctSoundSeconds_) < 1e-9,
            "invalid cellar timing/audio");
    require((!activity.rewarded || activity.complete_) && (!activity.resultCorrect_ || activity.complete_),
            "invalid cellar completion");
    require(activity.revealed_ || (activity.phase_ == CellarDoors::Phase::Idle && !activity.complete_),
            "unrevealed cellar active");
    auto sequence = [&](const char* name, auto& target) {
        std::set<size_t> seen;
        for (auto entry : node.children(name)) {
            const auto value = index(entry, "index", activity.syllables_.size() - 1);
            require(seen.insert(value).second, "duplicate cellar syllable");
            target.push_back(value);
        }
    };
    sequence("selected", activity.selected_);
    sequence("fading", activity.fading_);
    if (node.attribute("result_delay")) {
        activity.resultDelay_ = number(node, "result_delay");
        require(*activity.resultDelay_ >= 0 && *activity.resultDelay_ <= 1 &&
                    activity.phase_ == CellarDoors::Phase::Idle &&
                    activity.selected_.size() >= activity.answer_.size(),
                "invalid cellar callback delay");
    }
    if (activity.phase_ == CellarDoors::Phase::Reveal || activity.phase_ == CellarDoors::Phase::Fade)
        require(activity.age_ < 1.5, "expired cellar fade");
    if (activity.phase_ == CellarDoors::Phase::Watch)
        require(activity.age_ < activity.watchSeconds_ &&
                    std::abs(activity.watchSeconds_ - (activity.resultCorrect_ ? activity.correctSoundSeconds_
                                                                               : activity.wrongDelay_)) <
                        1e-9 &&
                    (!activity.resultCorrect_ || activity.selected_ == activity.answer_),
                "invalid cellar result watch");
    size_t i = 0;
    for (auto entry : node.children("highlight")) {
        require(i < activity.syllables_.size(), "too many cellar highlights");
        const auto age = number(entry, "age"), start = number(entry, "start");
        require((age == -1 || age >= 0) && start >= 0 && start <= 1, "invalid cellar highlight");
        activity.highlightAge_[i] = age;
        activity.fadeStart_[i] = float(start);
        ++i;
    }
    require(i == activity.syllables_.size(), "missing cellar highlights");
    std::istringstream random(node.child_value("random"));
    require(bool(random >> activity.random_), "invalid cellar RNG");
    random >> std::ws;
    require(random.eof(), "trailing cellar RNG");
    for (auto entry : node.children("event"))
        activity.events_.push_back({CellarEvent::Kind(index(entry, "kind", 2)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
void ActivityCheckpoints::save(const Rats& activity, pugi::xml_node node) {
    node.append_attribute("fade_age") = activity.fadeAge_;
    node.append_attribute("complete") = activity.complete_;
    node.append_attribute("rewarded") = activity.rewarded;
    node.append_attribute("mirror_sound") = activity.mirrorSound_;
    node.append_attribute("rat_sound") = activity.ratSound_;
    if (activity.fading_)
        node.append_attribute("fading").set_value(static_cast<unsigned long long>(*activity.fading_));
    if (activity.retainedRat_)
        node.append_attribute("retained").set_value(static_cast<unsigned long long>(*activity.retainedRat_));
    std::string gone, beams;
    for (bool value : activity.gone_)
        gone += value ? '1' : '0';
    for (bool value : activity.beams_)
        beams += value ? '1' : '0';
    node.append_attribute("gone") = gone.c_str();
    node.append_attribute("beams") = beams.c_str();
    for (const auto& mirror : activity.mirrors_) {
        auto entry = node.append_child("mirror");
        entry.append_attribute("state").set_value(static_cast<unsigned long long>(mirror.state));
        entry.append_attribute("forward") = mirror.forward;
    }
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(Rats& activity, pugi::xml_node node) {
    activity.events_.clear();
    activity.fadeAge_ = number(node, "fade_age");
    activity.complete_ = boolean(node, "complete");
    activity.rewarded = boolean(node, "rewarded");
    activity.mirrorSound_ = unsigned(index(node, "mirror_sound", 1));
    activity.ratSound_ = unsigned(index(node, "rat_sound", 1));
    auto flags = [&](const char* name, auto& values) {
        const std::string saved = node.attribute(name).as_string();
        require(saved.size() == values.size() && saved.find_first_not_of("01") == std::string::npos,
                "invalid rats bitset");
        for (size_t i = 0; i < values.size(); ++i)
            values[i] = saved[i] == '1';
    };
    flags("gone", activity.gone_);
    flags("beams", activity.beams_);
    require(activity.fadeAge_ >= 0 && activity.complete_ == (activity.goneCount() == activity.gone_.size()) &&
                (!activity.rewarded || activity.complete_),
            "invalid rat completion");
    if (node.attribute("fading"))
        activity.fading_ = index(node, "fading", activity.gone_.size() - 1);
    else
        activity.fading_.reset();
    if (node.attribute("retained"))
        activity.retainedRat_ = index(node, "retained", activity.gone_.size() - 1);
    else
        activity.retainedRat_.reset();
    if (activity.fading_)
        require(activity.gone_[*activity.fading_] && activity.retainedRat_ == activity.fading_ &&
                    activity.fadeAge_ < 2.5,
                "invalid rat fade");
    if (activity.retainedRat_)
        require(activity.gone_[*activity.retainedRat_], "retained rat was not removed");
    size_t i = 0;
    for (auto entry : node.children("mirror")) {
        require(i < activity.mirrors_.size(), "too many saved mirrors");
        auto& mirror = activity.mirrors_[i++];
        mirror.state = index(entry, "state", mirror.states.size() - 1);
        mirror.forward = boolean(entry, "forward");
        if (mirror.states.size() > 1)
            require((mirror.state != 0 || mirror.forward) &&
                        (mirror.state + 1 != mirror.states.size() || !mirror.forward),
                    "invalid mirror direction");
    }
    require(i == activity.mirrors_.size(), "missing saved mirrors");
    for (auto entry : node.children("event"))
        activity.events_.push_back({RatsEvent::Kind(index(entry, "kind", 3)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
void ActivityCheckpoints::save(const DoveCote& activity, pugi::xml_node node) {
    node.append_attribute("phase") = int(activity.phase_);
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(flightDoor_);
    WRITE(age_);
    WRITE(inside_);
    WRITE(fed_);
    WRITE(complete_);
    WRITE(helpShown_);
    WRITE(openingWasClosed_);
    WRITE(mouseX_);
    WRITE(mouseY_);
    WRITE(rewarded);
#undef WRITE
    if (activity.excludedDoor_)
        node.append_attribute("excluded").set_value(static_cast<unsigned long long>(*activity.excludedDoor_));
    for (auto value : activity.path_)
        node.append_child("roll").append_attribute("index").set_value(static_cast<unsigned long long>(value));
    std::string closed;
    for (bool value : activity.closed_)
        closed += value ? '1' : '0';
    node.append_attribute("closed") = closed.c_str();
    std::ostringstream random;
    random << activity.random_;
    node.append_child("random").text().set(random.str().c_str());
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    visuals(std::vector<ActivityObject>(activity.objects_.begin(),
                                        activity.objects_.begin() + activity.firstRope_),
            node);
}
void ActivityCheckpoints::load(DoveCote& activity, pugi::xml_node node) {
    activity.phase_ = DoveCote::Phase(index(node, "phase", 5));
    activity.flightDoor_ = index(node, "flightDoor_", activity.locks_.size() - 1);
    activity.age_ = number(node, "age_");
#define BOOL(field) activity.field = boolean(node, #field)
    BOOL(inside_);
    BOOL(fed_);
    BOOL(complete_);
    BOOL(helpShown_);
    BOOL(openingWasClosed_);
    BOOL(rewarded);
#undef BOOL
    const auto x = number(node, "mouseX_"), y = number(node, "mouseY_");
    require(std::abs(x) <= std::numeric_limits<float>::max() &&
                std::abs(y) <= std::numeric_limits<float>::max(),
            "rope coordinate overflow");
    activity.mouseX_ = float(x);
    activity.mouseY_ = float(y);
    if (node.attribute("excluded"))
        activity.excludedDoor_ = index(node, "excluded", activity.locks_.size() - 1);
    const std::string closed = node.attribute("closed").as_string();
    require(closed.size() == activity.closed_.size() && closed.find_first_not_of("01") == std::string::npos,
            "invalid dove door states");
    for (size_t i = 0; i < closed.size(); ++i)
        activity.closed_[i] = closed[i] == '1';
    std::vector<unsigned> visits(activity.count_);
    for (auto entry : node.children("roll")) {
        const auto value = index(entry, "index", activity.count_ - 1);
        require(++visits[value] <= 2, "dove roll used too often");
        if (!activity.path_.empty())
            require(activity.path_.back() != value && value != activity.path_.front() &&
                        (activity.path_.size() < 2 || value != activity.path_[activity.path_.size() - 2]),
                    "invalid saved rope path");
        activity.path_.push_back(value);
    }
    if (!activity.path_.empty())
        require((activity.path_.front() == activity.reference_.front() ||
                 activity.path_.front() == activity.reference_.back()) &&
                    activity.length() <= activity.maxLength_ + 1e-6,
                "invalid rope start/length");
    require(activity.age_ >= 0 && (!activity.rewarded || activity.complete_) &&
                (!activity.complete_ ||
                 (activity.correct() && activity.inside_ && activity.phase_ == DoveCote::Phase::Idle)),
            "invalid dove completion");
    if (activity.busy()) {
        const auto duration = activity.phase_ == DoveCote::Phase::ArrivalWait ? activity.arrivalDelay
                              : activity.phase_ == DoveCote::Phase::Incoming
                                  ? activity.incoming_[activity.flightDoor_]->duration()
                              : activity.phase_ == DoveCote::Phase::Watch ? 1.5
                              : activity.phase_ == DoveCote::Phase::DoorOpening
                                  ? 1.
                                  : activity.outgoing_[activity.flightDoor_]->duration();
        require(activity.age_ < duration || (duration == 0 && activity.age_ == 0), "expired dove phase");
    }
    if (activity.phase_ == DoveCote::Phase::ArrivalWait || activity.phase_ == DoveCote::Phase::Incoming)
        require(activity.fed_ && !activity.inside_, "invalid dove arrival");
    if (activity.phase_ == DoveCote::Phase::Watch || activity.phase_ == DoveCote::Phase::DoorOpening ||
        activity.phase_ == DoveCote::Phase::Outgoing)
        require(activity.inside_, "dove escaped before departure animation");
    std::istringstream random(node.child_value("random"));
    require(bool(random >> activity.random_), "invalid dove RNG");
    random >> std::ws;
    require(random.eof(), "trailing dove RNG");
    for (auto entry : node.children("event"))
        activity.events_.push_back({DoveEvent::Kind(index(entry, "kind", 3)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}

void ActivityCheckpoints::save(const RevealSpirits& activity, pugi::xml_node node) {
    node.append_attribute("brush") = activity.brush.c_str();
    node.append_attribute("container") = activity.container_->id.c_str();
    for (const auto& target : activity.targets_) {
        auto entry = node.append_child("target");
        entry.append_attribute("spirit") = target.spirit->id.c_str();
        entry.append_attribute("mask") = target.mask->id.c_str();
    }
    for (auto value : activity.pending_)
        node.append_child("pending").append_attribute("index").set_value(
            static_cast<unsigned long long>(value));
    auto position = [&](const char* name, const auto& value) {
        if (value) {
            auto entry = node.append_child(name);
            entry.append_attribute("x") = value->first;
            entry.append_attribute("y") = value->second;
        }
    };
    position("position", activity.position_);
    position("previous", activity.previous_);
    if (activity.hit_)
        node.append_attribute("hit").set_value(static_cast<unsigned long long>(*activity.hit_));
    node.append_attribute("stationary_age") = activity.stationaryAge_;
    node.append_attribute("brushing") = activity.brushPlaying_;
    for (const auto& [id, seconds] : activity.vanishCooldowns_) {
        auto entry = node.append_child("cooldown");
        entry.append_attribute("id") = id.c_str();
        entry.append_attribute("seconds") = seconds;
    }
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
}
void ActivityCheckpoints::load(RevealSpirits& activity, pugi::xml_node node) {
    activity.pending_.clear();
    std::set<size_t> seen;
    for (auto entry : node.children("pending")) {
        const auto value = index(entry, "index", activity.targets_.size() - 1);
        require(seen.insert(value).second, "duplicate pending spirit");
        activity.pending_.push_back(value);
    }
    require(std::is_sorted(activity.pending_.begin(), activity.pending_.end()),
            "invalid spirit retirement order");
    for (size_t i = 0; i < activity.targets_.size(); ++i)
        require(seen.contains(i) || activity.targets_[i].spirit->alpha == 1, "retired spirit is incomplete");
    auto position = [&](const char* name, auto& value) {
        if (auto entry = node.child(name)) {
            auto coordinate = [&](const char* key) {
                const auto n = number(entry, key);
                require(n == std::trunc(n) && n >= std::numeric_limits<int>::min() &&
                            n <= std::numeric_limits<int>::max(),
                        "invalid brush coordinate");
                return int(n);
            };
            value = {coordinate("x"), coordinate("y")};
        }
    };
    position("position", activity.position_);
    position("previous", activity.previous_);
    if (node.attribute("hit"))
        activity.hit_ = index(node, "hit", activity.targets_.size() - 1);
    activity.stationaryAge_ = number(node, "stationary_age");
    activity.brushPlaying_ = boolean(node, "brushing");
    require(activity.stationaryAge_ >= 0 &&
                (activity.position_ || (!activity.previous_ && !activity.hit_ && !activity.brushPlaying_)),
            "invalid brush motion state");
    for (auto entry : node.children("cooldown")) {
        const std::string id = entry.attribute("id").as_string();
        const auto seconds = number(entry, "seconds");
        require(std::any_of(activity.targets_.begin(), activity.targets_.end(),
                            [&](const auto& target) { return target.spirit->id == id; }) &&
                    seconds > 0 && seconds <= activity.vanishSoundDelay &&
                    activity.vanishCooldowns_.emplace(id, seconds).second,
                "invalid spirit vanish cooldown");
    }
    for (auto entry : node.children("event"))
        activity.events_.push_back({RevealEvent::Kind(index(entry, "kind", 1)), entry.child_value()});
}
void ActivityCheckpoints::save(const SpiritLock& activity, pugi::xml_node node) {
    node.append_attribute("phase") = int(activity.phase_);
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(angle_);
    WRITE(cursorX_);
    WRITE(cursorY_);
    WRITE(target_);
    WRITE(applied_);
    WRITE(lastKeyAngle_);
    WRITE(finalSeconds_);
    WRITE(key_);
    WRITE(moved_);
    WRITE(entered_);
    WRITE(complete_);
    WRITE(allPiecesFound_);
    WRITE(rewarded);
#undef WRITE
    std::string placed;
    for (bool value : activity.placed_)
        placed += value ? '1' : '0';
    node.append_attribute("placed") = placed.c_str();
    for (const auto& key : activity.sequence_)
        node.append_child("key").append_attribute("duration") = key.duration;
    for (const auto& [id, object] : activity.effects_) {
        auto entry = node.append_child("effect");
        entry.append_attribute("id") = id.c_str();
#define WRITE(field) entry.append_attribute(#field).set_value(object->field)
        WRITE(treeTime);
        WRITE(particlePresimPending);
        WRITE(treePlaying);
        WRITE(treeLooped);
        WRITE(treePlayOption);
        WRITE(treeLoopOverridden);
        WRITE(enabled);
        WRITE(particleGenerating);
        WRITE(particlesPresimRequested);
#undef WRITE
    }
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(SpiritLock& activity, pugi::xml_node node) {
    activity.phase_ = SpiritLock::Phase(index(node, "phase", 2));
    activity.key_ = index(node, "key_", activity.sequence_.size());
#define FLOAT(field)                                                                                         \
    {                                                                                                        \
        const auto value = number(node, #field);                                                             \
        require(std::abs(value) <= std::numeric_limits<float>::max(), "lock float overflow");                \
        activity.field = float(value);                                                                       \
    }
    FLOAT(angle_);
    FLOAT(cursorX_);
    FLOAT(cursorY_);
    FLOAT(target_);
    FLOAT(applied_);
    FLOAT(lastKeyAngle_);
#undef FLOAT
#define BOOL(field) activity.field = boolean(node, #field)
    BOOL(moved_);
    BOOL(entered_);
    BOOL(complete_);
    BOOL(allPiecesFound_);
    BOOL(rewarded);
#undef BOOL
    activity.finalSeconds_ = number(node, "finalSeconds_");
    // The old SDL channel cannot signal this process; finish on the saved timer.
    activity.audioCallback_ = false;
    require(activity.finalSeconds_ >= 0 && activity.finalSeconds_ <= activity.sequence_.back().duration &&
                (!activity.rewarded || activity.complete_) &&
                (!activity.complete_ || activity.phase_ == SpiritLock::Phase::Idle),
            "invalid lock completion/audio time");
    const std::string placed = node.attribute("placed").as_string();
    require(placed.size() == activity.placed_.size() && placed.find_first_not_of("01") == std::string::npos,
            "invalid lock pieces");
    for (size_t i = 0; i < placed.size(); ++i)
        activity.placed_[i] = placed[i] == '1';
    require(activity.assembled() || (!activity.busy() && !activity.complete_ && activity.key_ == 0),
            "unassembled lock is turning");
    require(activity.key_ == 0 ? (!activity.busy() || activity.lastKeyAngle_ == 0)
                               : activity.lastKeyAngle_ == activity.sequence_[activity.key_ - 1].angle,
            "invalid lock pronunciation progress");
    if (activity.phase_ == SpiritLock::Phase::Swinging)
        require(activity.moved_ && std::abs(activity.angle_) < 360, "invalid lock return swing");
    size_t i = 0;
    for (auto entry : node.children("key")) {
        require(i < activity.sequence_.size() &&
                    std::abs(number(entry, "duration") - activity.sequence_[i++].duration) < 1e-9,
                "lock audio duration changed");
    }
    require(i == activity.sequence_.size(), "missing lock audio durations");
    std::set<std::string> seen;
    for (auto entry : node.children("effect")) {
        const std::string id = entry.attribute("id").as_string();
        require(activity.effects_.contains(id) && seen.insert(id).second, "unknown or duplicate lock effect");
        auto& object = *activity.effects_.at(id);
        object.treeTime = number(entry, "treeTime");
        object.particlePresimPending = number(entry, "particlePresimPending");
        require(object.treeTime >= 0 && object.particlePresimPending >= 0, "invalid lock effect clock");
#define BOOL(field) object.field = boolean(entry, #field)
        BOOL(treePlaying);
        BOOL(treeLooped);
        BOOL(treePlayOption);
        BOOL(treeLoopOverridden);
        BOOL(enabled);
        BOOL(particleGenerating);
        BOOL(particlesPresimRequested);
#undef BOOL
    }
    require(seen.size() == activity.effects_.size(), "missing lock effects");
    for (auto entry : node.children("event"))
        activity.events_.push_back({SpiritLockEvent::Kind(index(entry, "kind", 4)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}

void ActivityCheckpoints::save(const Leeches& activity, pugi::xml_node node) {
    node.append_attribute("random").set_value(activity.random_.state_);
    node.append_attribute("orange") = activity.orange_.c_str();
    node.append_attribute("solved") = activity.solved_;
    node.append_attribute("complete") = activity.complete_;
    node.append_attribute("rewarded") = activity.rewarded;
    if (activity.held_)
        node.append_attribute("held").set_value(static_cast<unsigned long long>(*activity.held_));
    for (size_t i = 0; i < activity.states_.size(); ++i) {
        const auto& state = activity.states_[i];
        auto entry = node.append_child("leech");
        entry.append_attribute("index").set_value(static_cast<unsigned long long>(i));
        entry.append_attribute("phase") = int(state.phase);
        entry.append_attribute("delay") = state.delay;
        entry.append_attribute("age") = state.age;
        entry.append_attribute("caught") = state.caught;
        entry.append_attribute("removed") = state.removed;
    }
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(Leeches& activity, pugi::xml_node node) {
    activity.random_.state_ = uint32_t(index(node, "random", std::numeric_limits<uint32_t>::max()));
    activity.orange_ = node.attribute("orange").as_string();
    activity.solved_ = boolean(node, "solved");
    activity.complete_ = boolean(node, "complete");
    activity.rewarded = boolean(node, "rewarded");
    if (node.attribute("held"))
        activity.held_ = index(node, "held", activity.states_.size() - 1);
    std::set<size_t> seen;
    size_t caught = 0;
    for (auto entry : node.children("leech")) {
        const auto i = index(entry, "index", activity.states_.size() - 1);
        require(seen.insert(i).second, "duplicate leech");
        auto& state = activity.states_[i];
        state.phase = LeechState::Phase(index(entry, "phase", 2));
        state.delay = number(entry, "delay");
        state.age = number(entry, "age");
        state.caught = boolean(entry, "caught");
        state.removed = boolean(entry, "removed");
        require(state.delay >= -1e-8 && state.delay <= activity.maxMs_ * .001 && state.age >= 0 &&
                    state.age <= activity.animations_[i].duration() + 1e-8,
                "invalid leech time");
        if (state.phase == LeechState::Animating)
            require(state.age < activity.animations_[i].duration(), "expired leech animation");
        if (state.caught) {
            ++caught;
            require(activity.held_ == i && !state.removed, "invalid held leech");
        }
    }
    require(seen.size() == activity.states_.size() && caught == size_t(activity.held_.has_value()),
            "incomplete leech state/capture");
    require(activity.held_ ? Leeches::isOrange(activity.orange_) : activity.orange_.empty(),
            "invalid captured orange");
    require(activity.solved_ == (activity.removedCount() == activity.states_.size()) &&
                (!activity.complete_ || activity.solved_) && (!activity.rewarded || activity.complete_),
            "invalid leech completion");
    for (auto entry : node.children("event"))
        activity.events_.push_back({LeechesEvent::Kind(index(entry, "kind", 2)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
void ActivityCheckpoints::save(const Sail& activity, pugi::xml_node node) {
    node.append_attribute("phase") = int(activity.phase_);
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(pressX_);
    WRITE(pressY_);
    WRITE(offsetX_);
    WRITE(offsetY_);
    WRITE(sewAge_);
    WRITE(held_);
    WRITE(readyAnnounced_);
    WRITE(complete_);
    WRITE(rewarded);
#undef WRITE
    for (size_t i = 0; i < activity.pieces_.size(); ++i) {
        const auto& piece = activity.pieces_[i];
        auto entry = node.append_child("piece");
        entry.append_attribute("index").set_value(static_cast<unsigned long long>(i));
        entry.append_attribute("id") = piece.id.c_str();
        entry.append_attribute("turns") = piece.turns;
        entry.append_attribute("placed") = piece.placed;
        entry.append_attribute("fixed") = piece.fixedView;
    }
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(Sail& activity, pugi::xml_node node) {
    activity.phase_ = Sail::Phase(index(node, "phase", 3));
    activity.held_ = index(node, "held_", activity.pieces_.size() - 1);
    activity.sewAge_ = number(node, "sewAge_");
#define FLOAT(field)                                                                                         \
    {                                                                                                        \
        const auto value = number(node, #field);                                                             \
        require(std::abs(value) <= std::numeric_limits<float>::max(), "sail float overflow");                \
        activity.field = float(value);                                                                       \
    }
    FLOAT(pressX_);
    FLOAT(pressY_);
    FLOAT(offsetX_);
    FLOAT(offsetY_);
#undef FLOAT
    activity.readyAnnounced_ = boolean(node, "readyAnnounced_");
    activity.complete_ = boolean(node, "complete_");
    activity.rewarded = boolean(node, "rewarded");
    std::set<size_t> seen;
    for (auto entry : node.children("piece")) {
        const auto i = index(entry, "index", activity.pieces_.size() - 1);
        auto& piece = activity.pieces_[i];
        require(seen.insert(i).second && piece.id == entry.attribute("id").as_string(),
                "unknown or duplicate sail piece");
        piece.turns = unsigned(index(entry, "turns", 3));
        piece.placed = boolean(entry, "placed");
        piece.fixedView = boolean(entry, "fixed");
    }
    require(seen.size() == activity.pieces_.size(), "missing sail pieces");
    for (const auto& [id, pieces] : activity.inventory_)
        for (auto i : pieces)
            require(activity.pieces_[i].placed == activity.pieces_[pieces.front()].placed,
                    "partially placed inventory bundle");
    restoreVisuals(activity.objects_, node);
    require(activity.sewAge_ >= 0 && (!activity.rewarded || activity.complete_) &&
                (!activity.complete_ || (activity.ready() && activity.phase_ == Sail::Phase::Idle)),
            "invalid sail completion");
    require(!activity.readyAnnounced_ || activity.ready(), "lost ready sail arrangement");
    if (activity.busy())
        require(activity.ready() && !activity.complete_ && activity.sewAge_ <= 3,
                "invalid sewing timer/arrangement");
    require(activity.complete_ || activity.busy() || activity.sewAge_ == 0, "inactive sewing timer");
    if (activity.gripping())
        require(activity.pieces_[activity.held_].placed && !activity.complete_, "invalid sail grip");
    for (size_t i = 0; i < activity.pieces_.size(); ++i)
        require(!activity.pieces_[i].fixedView || activity.correct(i), "fixed sail piece out of place");
    for (auto entry : node.children("event"))
        activity.events_.push_back({SailEvent::Kind(index(entry, "kind", 3)), entry.child_value()});
    activity.updateVisuals();
}

void ActivityCheckpoints::save(const BlockedWell& activity, pugi::xml_node node) {
    node.append_attribute("phase") = int(activity.phase_);
    node.append_attribute("age") = activity.age_;
    node.append_attribute("word") = activity.word_.c_str();
    node.append_attribute("previous") = activity.previous_.c_str();
    node.append_attribute("entered") = activity.entered_;
    node.append_attribute("complete") = activity.complete_;
    node.append_attribute("rewarded") = activity.rewarded;
    if (activity.hovered_)
        node.append_attribute("hovered").set_value(static_cast<unsigned long long>(*activity.hovered_));
    std::string known;
    for (bool value : activity.known_)
        known += value ? '1' : '0';
    node.append_attribute("known") = known.c_str();
    for (auto value : activity.spelled_)
        node.append_child("spelled").append_attribute("index").set_value(
            static_cast<unsigned long long>(value));
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(BlockedWell& activity, pugi::xml_node node) {
    activity.phase_ = BlockedWell::Phase(index(node, "phase", 3));
    activity.age_ = number(node, "age");
    activity.word_ = node.attribute("word").as_string();
    activity.previous_ = node.attribute("previous").as_string();
    activity.entered_ = boolean(node, "entered");
    activity.complete_ = boolean(node, "complete");
    activity.rewarded = boolean(node, "rewarded");
    if (node.attribute("hovered"))
        activity.hovered_ = index(node, "hovered", activity.alphabet_.size() - 1);
    const std::string known = node.attribute("known").as_string();
    require(known.size() == activity.known_.size() && known.find_first_not_of("01") == std::string::npos,
            "invalid known well names");
    for (size_t i = 0; i < known.size(); ++i)
        activity.known_[i] = known[i] == '1';
    std::set<size_t> seen;
    for (auto entry : node.children("spelled")) {
        const auto i = index(entry, "index", activity.names_.size() - 1);
        require(seen.insert(i).second && activity.known_[i], "duplicate or unknown spelled name");
        activity.spelled_.push_back(i);
    }
    size_t maxLength = 0;
    for (const auto& name : activity.names_)
        maxLength = std::max(maxLength, name.size());
    require(activity.word_.size() <= maxLength + 1 && activity.previous_.size() <= maxLength &&
                activity.word_.find_first_not_of(activity.alphabet_) == std::string::npos &&
                activity.previous_.find_first_not_of(activity.alphabet_) == std::string::npos,
            "invalid well word");
    require(
        activity.complete_ == (activity.spelled_.size() == activity.names_.size()) &&
            (!activity.rewarded || activity.complete_) &&
            (activity.entered_ || (!activity.busy() && activity.spelled_.empty() && activity.word_.empty())),
        "invalid well completion/entry");
    require(activity.age_ >= 0, "negative well fade age");
    if (activity.phase_ == BlockedWell::Phase::Idle)
        require(activity.age_ == 0 && activity.previous_.empty() &&
                    (activity.spelled_.size() < activity.names_.size() || activity.word_.empty()),
                "invalid idle well input");
    else {
        const auto duration = activity.phase_ == BlockedWell::Phase::Letter ? activity.letterSeconds_
                              : activity.phase_ == BlockedWell::Phase::Invalid
                                  ? activity.invalidSeconds_
                                  : std::max(activity.crossSeconds_, activity.progressSeconds_);
        require(activity.age_ < duration && !activity.word_.empty(), "expired/empty well fade");
        if (activity.phase_ == BlockedWell::Phase::Letter)
            require(activity.spelled_.size() < activity.names_.size() &&
                        activity.word_.substr(0, activity.word_.size() - 1) == activity.previous_,
                    "invalid previous letter plaque");
        else
            require(activity.previous_.empty(), "stale previous well plaque");
        if (activity.phase_ == BlockedWell::Phase::Correct)
            require(!activity.spelled_.empty() && activity.word_ == activity.names_[activity.spelled_.back()],
                    "wrong accepted name plaque");
        else
            require(!activity.complete_, "completed well accepts input");
    }
    if (activity.phase_ == BlockedWell::Phase::Idle && !activity.word_.empty()) {
        bool prefix = false;
        for (size_t i = 0; i < activity.names_.size(); ++i)
            prefix |=
                activity.known_[i] && !seen.contains(i) && activity.names_[i].starts_with(activity.word_);
        require(prefix, "invalid unfinished name prefix");
    }
    for (auto entry : node.children("event"))
        activity.events_.push_back({WellEvent::Kind(index(entry, "kind", 2)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
void ActivityCheckpoints::save(const Apothecary& activity, pugi::xml_node node) {
    node.append_attribute("phase") = int(activity.phase_);
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(first_);
    WRITE(second_);
    WRITE(age_);
    WRITE(solved_);
    WRITE(complete_);
    WRITE(sheetOpen_);
    WRITE(swapCorrect_);
#undef WRITE
    if (activity.selected_)
        node.append_attribute("selected").set_value(static_cast<unsigned long long>(*activity.selected_));
    for (size_t i = 0; i < activity.slots_.size(); ++i) {
        auto entry = node.append_child("slot");
        entry.append_attribute("index").set_value(static_cast<unsigned long long>(i));
        if (activity.slots_[i])
            entry.append_attribute("item").set_value(static_cast<unsigned long long>(*activity.slots_[i]));
    }
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(Apothecary& activity, pugi::xml_node node) {
    activity.phase_ = Apothecary::Phase(index(node, "phase", 2));
    activity.first_ = index(node, "first_", activity.slots_.size() - 1);
    activity.second_ = index(node, "second_", activity.slots_.size() - 1);
    activity.age_ = number(node, "age_");
#define BOOL(field) activity.field = boolean(node, #field)
    BOOL(solved_);
    BOOL(complete_);
    BOOL(sheetOpen_);
    BOOL(swapCorrect_);
#undef BOOL
    std::set<size_t> slots, items;
    for (auto entry : node.children("slot")) {
        const auto i = index(entry, "index", activity.slots_.size() - 1);
        require(slots.insert(i).second, "duplicate shelf slot");
        if (entry.attribute("item")) {
            const auto value = index(entry, "item", activity.items_.size() - 1);
            require(items.insert(value).second, "duplicate shelf item");
            activity.slots_[i] = value;
        } else
            activity.slots_[i].reset();
    }
    require(slots.size() == activity.slots_.size(), "missing shelf slots");
    size_t rewards = 0, remaining = 0;
    for (size_t i = 0; i < activity.items_.size(); ++i) {
        const auto& item = activity.items_[i];
        if (item.reward) {
            ++rewards;
            remaining += items.contains(i);
            require(activity.solved_ || items.contains(i), "unclaimed pharmacy reward missing");
        } else
            require(items.contains(i) || (!activity.solved_ && !item.inventory.empty()),
                    "stock medicine missing");
    }
    if (activity.solved_)
        for (size_t i = 0; i < activity.slots_.size(); ++i)
            require(activity.locked(i) || (!activity.slots_[i] && activity.items_[i].reward),
                    "solved shelf arrangement changed");
    require((!activity.complete_ || (activity.solved_ && remaining == 0)) &&
                (!activity.solved_ || remaining > 0 || rewards == 0 || activity.complete_),
            "invalid shelf reward state");
    require(activity.age_ >= 0, "negative shelf swap time");
    if (activity.busy()) {
        require(!activity.solved_ && !activity.sheetOpen_ && activity.missingCount() == 0 &&
                    activity.first_ != activity.second_ && activity.age_ < activity.swapSeconds_ / 2,
                "invalid shelf swap");
        require(activity.phase_ == Apothecary::Phase::FadeOut
                    ? (!activity.locked(activity.first_) && !activity.locked(activity.second_) &&
                       !activity.swapCorrect_)
                    : activity.swapCorrect_ ==
                          (activity.locked(activity.first_) || activity.locked(activity.second_)),
                "invalid shelf swap correctness");
    } else
        require(activity.age_ == 0, "inactive shelf swap timer");
    if (node.attribute("selected")) {
        activity.selected_ = index(node, "selected", activity.slots_.size() - 1);
        require(!activity.busy() && !activity.solved_ && activity.missingCount() == 0 &&
                    !activity.locked(*activity.selected_),
                "invalid selected shelf item");
    }
    for (auto entry : node.children("event"))
        activity.events_.push_back({ApothecaryEvent::Kind(index(entry, "kind", 5)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}

void ActivityCheckpoints::save(const CemeteryGate& activity, pugi::xml_node node) {
    node.append_attribute("age") = activity.age_;
    node.append_attribute("entered") = activity.entered_;
    node.append_attribute("complete") = activity.complete_;
    node.append_attribute("rewarded") = activity.rewarded;
    if (activity.rotating_)
        node.append_attribute("rotating").set_value(static_cast<unsigned long long>(*activity.rotating_));
    for (auto value : activity.current_)
        node.append_child("block").append_attribute("current").set_value(
            static_cast<unsigned long long>(value));
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    effect(*activity.effect_, node.append_child("effect"));
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(CemeteryGate& activity, pugi::xml_node node) {
    activity.age_ = number(node, "age");
    activity.entered_ = boolean(node, "entered");
    activity.complete_ = boolean(node, "complete");
    activity.rewarded = boolean(node, "rewarded");
    if (node.attribute("rotating"))
        activity.rotating_ = index(node, "rotating", activity.blocks_.size() - 1);
    size_t i = 0;
    for (auto entry : node.children("block")) {
        require(i < activity.current_.size(), "too many gate blocks");
        activity.current_[i] = index(entry, "current", activity.blocks_[i].size() - 1);
        ++i;
    }
    require(i == activity.current_.size(), "missing gate blocks");
    bool all = true;
    for (i = 0; i < activity.blocks_.size(); ++i)
        all &= activity.locked(i);
    require(activity.age_ >= 0 &&
                (activity.busy() ? (activity.entered_ && activity.age_ < activity.rotation_.duration())
                                 : activity.age_ == 0) &&
                (!activity.complete_ || (all && !activity.busy())) &&
                (!activity.rewarded || activity.complete_),
            "invalid gate rotation/completion");
    restoreEffect(*activity.effect_, node.child("effect"));
    require(activity.effect_->particleGenerating == activity.busy(), "gate generator/rotation mismatch");
    for (auto entry : node.children("event"))
        activity.events_.push_back({GateEvent::Kind(index(entry, "kind", 2)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
void ActivityCheckpoints::save(const SkullsObey& activity, pugi::xml_node node) {
    node.append_attribute("phase") = int(activity.phase_);
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(selected_);
    WRITE(target_);
    WRITE(angle_);
    WRITE(targetAngle_);
    WRITE(velocity_);
    WRITE(remaining_);
    WRITE(entered_);
    WRITE(opened_);
    WRITE(complete_);
    WRITE(rewarded);
#undef WRITE
    for (auto value : activity.history_)
        node.append_child("history").append_attribute("skull").set_value(
            static_cast<unsigned long long>(value));
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    effect(*activity.eyes_, node.append_child("eyes"));
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(SkullsObey& activity, pugi::xml_node node) {
    activity.phase_ = SkullsObey::Phase(index(node, "phase", 4));
    activity.selected_ = index(node, "selected_", activity.angles_.size() - 1);
    activity.target_ = index(node, "target_", activity.angles_.size() - 1);
#define NUMBER(field) activity.field = number(node, #field)
    NUMBER(angle_);
    NUMBER(targetAngle_);
    NUMBER(velocity_);
    NUMBER(remaining_);
#undef NUMBER
#define BOOL(field) activity.field = boolean(node, #field)
    BOOL(entered_);
    BOOL(opened_);
    BOOL(complete_);
    BOOL(rewarded);
#undef BOOL
    for (auto entry : node.children("history"))
        activity.history_.push_back(index(entry, "skull", activity.angles_.size() - 1));
    require(activity.history_.size() <= activity.sequence_.size() &&
                std::equal(activity.history_.begin(), activity.history_.end(), activity.sequence_.begin()),
            "invalid skull phrase prefix");
    require(activity.remaining_ >= 0 && (!activity.complete_ || (activity.opened_ && !activity.busy())) &&
                (!activity.rewarded || activity.complete_) &&
                (!activity.opened_ || activity.phase_ == SkullsObey::Idle ||
                 activity.phase_ == SkullsObey::LidFade),
            "invalid skull completion");
    if (activity.phase_ == SkullsObey::Turning) {
        double start = activity.angles_[activity.selected_], target = activity.angles_[activity.target_];
        if (std::abs(start - target) > 180) {
            if (target <= start)
                target += 360;
            else
                start += 360;
        }
        const auto velocity = target < start ? -activity.speed_ : activity.speed_;
        require(activity.entered_ && activity.target_ != activity.selected_ &&
                    activity.targetAngle_ == target && activity.velocity_ == velocity &&
                    activity.remaining_ <= std::abs(target - start) / activity.speed_ + 1e-9 &&
                    std::abs(activity.angle_ - (target - velocity * activity.remaining_)) < 1e-7,
                "invalid skull rotation");
    } else if (activity.busy()) {
        const auto duration = activity.phase_ == SkullsObey::BeforePhrase  ? activity.before_
                              : activity.phase_ == SkullsObey::AfterPhrase ? activity.after_
                                                                           : activity.fade_;
        require(activity.entered_ && activity.history_ == activity.sequence_ &&
                    activity.remaining_ <= duration &&
                    (activity.phase_ == SkullsObey::LidFade ? activity.opened_ : !activity.opened_),
                "invalid skull phrase/lid timing");
    }
    if (activity.entered_ && activity.phase_ != SkullsObey::Turning)
        require(activity.angle_ == activity.angles_[activity.selected_], "idle skull angle mismatch");
    restoreEffect(*activity.eyes_, node.child("eyes"));
    for (auto entry : node.children("event"))
        activity.events_.push_back({SkullEvent::Kind(index(entry, "kind", 3)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
void ActivityCheckpoints::save(const VoodooDoll& activity, pugi::xml_node node) {
    node.append_attribute("progress").set_value(static_cast<unsigned long long>(activity.progress_));
    node.append_attribute("rewarded") = activity.rewarded;
    node.append_attribute("feedback") = activity.feedback.c_str();
    for (size_t i = 0; i < activity.placement_.size(); ++i) {
        auto entry = node.append_child("ingredient");
        entry.append_attribute("index").set_value(static_cast<unsigned long long>(i));
        if (activity.placement_[i])
            entry.append_attribute("slot").set_value(
                static_cast<unsigned long long>(*activity.placement_[i]));
    }
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(VoodooDoll& activity, pugi::xml_node node) {
    activity.progress_ = index(node, "progress", activity.ingredients_.size());
    activity.rewarded = boolean(node, "rewarded");
    activity.feedback = node.attribute("feedback").as_string();
    std::set<size_t> seen, slots;
    for (auto entry : node.children("ingredient")) {
        const auto i = index(entry, "index", activity.ingredients_.size() - 1);
        require(seen.insert(i).second, "duplicate doll ingredient");
        if (entry.attribute("slot")) {
            const auto value = index(entry, "slot", activity.slots_.size());
            require(activity.ingredients_[i].central ? value == activity.slots_.size()
                                                     : value < activity.slots_.size(),
                    "wrong doll central/side slot");
            require(slots.insert(value).second, "duplicate doll slot");
            activity.placement_[i] = value;
        }
    }
    require(seen.size() == activity.ingredients_.size() &&
                (activity.placement_[0] ? (activity.progress_ >= 1) : activity.progress_ == 0) &&
                (activity.progress_ <= 1 || activity.arranged()) &&
                (!activity.rewarded || activity.complete()),
            "invalid doll progress");
    for (size_t i = 0; i < activity.progress_; ++i)
        require(activity.placement_[i].has_value(), "used doll ingredient was not placed");
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
void ActivityCheckpoints::save(const VoodooCircle& activity, pugi::xml_node node) {
#define WRITE(field) node.append_attribute(#field).set_value(activity.field)
    WRITE(spellKnown_);
    WRITE(entered_);
    WRITE(moss_);
    WRITE(drawing_);
    WRITE(pressing_);
    WRITE(ending_);
    WRITE(complete_);
    WRITE(chalkSound_);
    WRITE(age_);
    WRITE(soundAge_);
    WRITE(rewarded);
#undef WRITE
    if (activity.active_)
        node.append_attribute("active").set_value(static_cast<unsigned long long>(*activity.active_));
    if (activity.previous_) {
        node.append_attribute("previous_x") = activity.previous_->first;
        node.append_attribute("previous_y") = activity.previous_->second;
    }
    for (auto value : activity.strokePoints_)
        node.append_child("stroke").append_attribute("point").set_value(
            static_cast<unsigned long long>(value));
    std::string garbage;
    for (bool value : activity.garbage_)
        garbage += value ? '1' : '0';
    node.append_attribute("garbage") = garbage.c_str();
    for (const auto& item : activity.items_) {
        auto entry = node.append_child("item");
        entry.append_attribute("id") = item.inventory.c_str();
        entry.append_attribute("state") = item.state;
        if (item.slot)
            entry.append_attribute("slot").set_value(static_cast<unsigned long long>(*item.slot));
    }
    for (size_t i = 0; i < activity.shapes_.size(); ++i) {
        const auto& shape = activity.shapes_[i];
        auto entry = node.append_child("shape");
        entry.append_attribute("index").set_value(static_cast<unsigned long long>(i));
        entry.append_attribute("complete") = shape.complete;
        std::string painted;
        for (bool value : shape.painted)
            painted += value ? '1' : '0';
        entry.append_attribute("painted") = painted.c_str();
        std::ostringstream alpha;
        alpha << std::setprecision(std::numeric_limits<float>::max_digits10);
        for (float value : shape.alpha)
            alpha << value << ' ';
        entry.append_child("alpha").text().set(alpha.str().c_str());
    }
    for (const auto& [id, object] : activity.effects_)
        effect(*object, node.append_child("effect"));
    for (const auto& event : activity.events_) {
        auto entry = node.append_child("event");
        entry.append_attribute("kind") = int(event.kind);
        entry.text().set(event.value.c_str());
    }
    visuals(activity.objects_, node);
}
void ActivityCheckpoints::load(VoodooCircle& activity, pugi::xml_node node) {
#define BOOL(field) activity.field = boolean(node, #field)
    BOOL(spellKnown_);
    BOOL(entered_);
    BOOL(moss_);
    BOOL(drawing_);
    BOOL(pressing_);
    BOOL(ending_);
    BOOL(complete_);
    BOOL(chalkSound_);
    BOOL(rewarded);
#undef BOOL
    activity.age_ = number(node, "age_");
    activity.soundAge_ = number(node, "soundAge_");
    require(activity.age_ >= 0 && activity.soundAge_ >= 0, "negative circle time");
    const std::string garbage = node.attribute("garbage").as_string();
    require(garbage.size() == activity.garbage_.size() &&
                garbage.find_first_not_of("01") == std::string::npos,
            "invalid cleared garbage set");
    for (size_t i = 0; i < garbage.size(); ++i)
        activity.garbage_[i] = garbage[i] == '1';
    size_t i = 0;
    std::set<size_t> slots;
    for (auto entry : node.children("item")) {
        require(i < activity.items_.size(), "too many circle items");
        auto& item = activity.items_[i++];
        require(item.inventory == entry.attribute("id").as_string(), "unknown circle item");
        item.state = unsigned(index(entry, "state", 2));
        require(item.onScene || item.state != 1, "off-scene circle item was picked up");
        if (entry.attribute("slot")) {
            item.slot = index(entry, "slot", activity.items_.size() - 1);
            require(item.state == 2 && slots.insert(*item.slot).second &&
                        (item.conduit == (*item.slot + 1 == activity.items_.size())),
                    "invalid circle item slot");
        }
        require((item.state == 2) == item.slot.has_value(), "placed circle item missing slot");
    }
    require(i == activity.items_.size(), "missing circle items");
    std::set<size_t> shapes;
    bool paintedAny = false;
    for (auto entry : node.children("shape")) {
        const auto id = index(entry, "index", activity.shapes_.size() - 1);
        require(shapes.insert(id).second, "duplicate circle shape");
        auto& shape = activity.shapes_[id];
        shape.complete = boolean(entry, "complete");
        const std::string painted = entry.attribute("painted").as_string();
        require(painted.size() == shape.painted.size() &&
                    painted.find_first_not_of("01") == std::string::npos,
                "invalid circle control points");
        for (size_t point = 0; point < painted.size(); ++point)
            shape.painted[point] = painted[point] == '1';
        paintedAny |= shape.paintedCount() > 0;
        require(shape.complete == (double(shape.paintedCount()) / shape.points.size() >= shape.required),
                "circle shape completion mismatch");
        std::istringstream alpha(entry.child_value("alpha"));
        for (auto& value : shape.alpha)
            require(bool(alpha >> value) && std::isfinite(value) && value >= 0 && value <= 1,
                    "invalid circle vertex alpha");
        alpha >> std::ws;
        require(alpha.eof(), "extra circle vertices");
    }
    require(shapes.size() == activity.shapes_.size(), "missing circle shapes");
    if (node.attribute("active"))
        activity.active_ = index(node, "active", activity.shapes_.size() - 1);
    require(bool(node.attribute("previous_x")) == bool(node.attribute("previous_y")),
            "incomplete circle cursor");
    if (node.attribute("previous_x")) {
        const auto x = number(node, "previous_x"), y = number(node, "previous_y");
        require(std::abs(x) <= std::numeric_limits<float>::max() &&
                    std::abs(y) <= std::numeric_limits<float>::max(),
                "circle cursor overflow");
        activity.previous_ = {float(x), float(y)};
    }
    size_t maxPoints = 0;
    for (const auto& shape : activity.shapes_)
        maxPoints = std::max(maxPoints, shape.points.size());
    const auto points = activity.active_ ? activity.shapes_[*activity.active_].points.size() : maxPoints;
    std::set<size_t> stroke;
    for (auto entry : node.children("stroke")) {
        const auto point = index(entry, "point", points - 1);
        require(stroke.insert(point).second, "duplicate circle stroke point");
        activity.strokePoints_.push_back(point);
    }
    require(!activity.pressing_ || (activity.drawing_ && activity.previous_), "invalid circle press");
    require(activity.pressing_ || (!activity.active_ && activity.strokePoints_.empty()),
            "circle stroke without press");
    if (activity.active_)
        require(activity.pressing_ && !activity.shapes_[*activity.active_].complete,
                "invalid active circle shape");
    require(!activity.chalkSound_ || (activity.drawing_ && activity.pressing_), "chalk sound outside stroke");
    require(!(paintedAny || activity.drawn() || activity.drawing_) || activity.cleared(),
            "circle drawn before clearing");
    require(!activity.drawing_ || (!activity.drawn() && !activity.ending_ && !activity.complete_),
            "invalid circle drawing mode");
    const bool placed = std::all_of(activity.items_.begin(), activity.items_.end(),
                                    [](const auto& item) { return item.state == 2; });
    require(activity.ending_ == placed && (!placed || (activity.cleared() && activity.drawn())) &&
                (!activity.complete_ || activity.ending_) && (!activity.rewarded || activity.complete_),
            "invalid ritual completion/items");
    if (activity.busy())
        require(activity.age_ < activity.doll_.duration() + 3, "expired ritual ending");
    if (activity.complete_)
        require(activity.age_ + 1e-9 >= activity.doll_.duration() + 3, "early ritual completion");
    std::set<std::string> effects;
    for (auto entry : node.children("effect")) {
        const std::string id = entry.attribute("id").as_string();
        require(activity.effects_.contains(id) && effects.insert(id).second,
                "unknown/duplicate circle effect");
        restoreEffect(*activity.effects_.at(id), entry);
    }
    require(effects.size() == activity.effects_.size(), "missing circle effects");
    for (auto entry : node.children("event"))
        activity.events_.push_back({CircleEvent::Kind(index(entry, "kind", 8)), entry.child_value()});
    restoreVisuals(activity.objects_, node);
    activity.updateVisuals();
}
} // namespace deadtime
