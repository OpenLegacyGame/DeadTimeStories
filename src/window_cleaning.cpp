#include "window_cleaning.hpp"
#include "animation.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>
#include <utility>

namespace deadtime {
bool WindowCleaning::skip() {
    if (complete())
        return false;
    // Native 4A5800 finishes without rubbing or cascading dirty-water fades.
    events_.clear();
    dirty_.fill(false);
    phase_ = Phase::Finished;
    phaseAge_ = fxRemaining_ = 0;
    hovered_.reset();
    floraAge_.reset();
    hideSponge();
    for (size_t i = 0; i < sections_.size(); ++i) {
        objects_[sections_[i]].visible = false;
        objects_[sections_[i]].alpha = 0;
    }
    return true;
}
WindowCleaning::WindowCleaning(const Assets& assets, const std::string& path, uint32_t seed)
    : disappear_(assets, normalize(path) + "/grease_disappear.animset"),
      reappear_(assets, normalize(path) + "/grease_reappear.animset"), random_(seed) {
    const auto base = normalize(path);
    const auto data = assets.read(base + "/_config.xml");
    pugi::xml_document doc;
    if (!doc.load_buffer(data.data(), data.size()) || !doc.child("window_cleaning"))
        throw std::runtime_error("Invalid window cleaning configuration");
    const auto root = doc.child("window_cleaning"), cleaning = root.child("cleaning");
    onFinish = resolve(base, root.child("onFinish").attribute("script").value()) + ".lua";
    if (!assets.contains(onFinish))
        throw std::runtime_error("Missing window cleaning finish script");
    successSeconds = cleaning.child("timer_success").attribute("ms").as_double() / 1000.;
    failureSeconds = cleaning.child("timer_failure").attribute("ms").as_double() / 1000.;
    removeFxSeconds = cleaning.child("timer_remove_fx").attribute("ms").as_double() / 1000.;
    movementSeconds = cleaning.child("min_mouse_move_frequency").attribute("ms").as_double() / 1000.;
    speedMinX = cleaning.child("sponge_speed_factor_x").attribute("min").as_float();
    speedMaxX = cleaning.child("sponge_speed_factor_x").attribute("max").as_float();
    speedMinY = cleaning.child("sponge_speed_factor_y").attribute("min").as_float();
    speedMaxY = cleaning.child("sponge_speed_factor_y").attribute("max").as_float();
    fxX = cleaning.child("correct_fx_offset").attribute("x").as_float();
    fxY = cleaning.child("correct_fx_offset").attribute("y").as_float();
    for (double value : {successSeconds, failureSeconds, removeFxSeconds, movementSeconds, double(speedMinX),
                         double(speedMaxX), double(speedMinY), double(speedMaxY), double(fxX), double(fxY)})
        if (!std::isfinite(value))
            throw std::runtime_error("Nonfinite window cleaning configuration");
    if (successSeconds <= 0 || failureSeconds <= 0 || removeFxSeconds < 0 || movementSeconds <= 0 ||
        speedMaxX <= 0 || speedMinX > speedMaxX || speedMinY <= 0 || speedMinY > speedMaxY)
        throw std::runtime_error("Invalid window cleaning timing or speed");
    objects_ = loadActivityLayout(assets, base + "/layout.xml");
    measureActivityTextures(assets, objects_);
    for (size_t i = 0; i < sections_.size(); ++i) {
        auto& section = object("grease_" + std::to_string(i % 2) + '_' + std::to_string(i / 2));
        sections_[i] = size_t(&section - objects_.data());
        dirty_[i] = true;
    }
    for (const auto* name :
         {"sponge_dry", "sponge_wet", "cleaning_mixture", "flora", "correct_fx", "table", "exit"})
        object(name);
}
ActivityObject& WindowCleaning::object(const std::string& name) {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& object) { return object.id == name; });
    if (found == objects_.end())
        throw std::runtime_error("Missing window cleaning object: " + name);
    return *found;
}
void WindowCleaning::emit(WindowCleaningEvent::Kind kind, const std::string& value) {
    events_.push_back({kind, value});
}
std::vector<WindowCleaningEvent> WindowCleaning::takeEvents() {
    return std::exchange(events_, {});
}
bool WindowCleaning::placeMixture() {
    if (mixturePlaced_ || complete() || phase_ != Phase::Idle)
        return false;
    mixturePlaced_ = true;
    object("cleaning_mixture").visible = true;
    floraAge_ = 0;
    emit(WindowCleaningEvent::Kind::Sound, "sound/sfx/mg/window_cleaning/mg_bowl_placed");
    return true;
}
void WindowCleaning::positionSponge() {
    auto& sponge = object(wet_ ? "sponge_wet" : "sponge_dry");
    sponge.x = x_ - sponge.width / 2;
    sponge.y = y_ - sponge.height / 2;
    sponge.z = -10;
    sponge.visible = true;
    auto& fx = object("correct_fx");
    fx.x = sponge.x + fxX;
    fx.y = sponge.y + fxY;
    fx.z = -5;
    const auto& target = sectionObject(section_);
    const bool within =
        x_ >= target.x && x_ <= target.x + target.width && y_ >= target.y && y_ <= target.y + target.height;
    if (inside_ && !within)
        failureRemaining_ = failureSeconds;
    inside_ = within;
}
void WindowCleaning::hideSponge() {
    object("sponge_wet").visible = object("sponge_dry").visible = object("correct_fx").visible = false;
}
bool WindowCleaning::begin(size_t section, bool wet, int x, int y) {
    if (section >= sections_.size() || !dirty_[section] || phase_ != Phase::Idle)
        return false;
    wet_ = wet;
    section_ = section;
    x_ = x;
    y_ = y;
    phase_ = Phase::Rubbing;
    successRemaining_ = successSeconds;
    failureRemaining_ = failureSeconds;
    idleAge_ = phaseAge_ = fxRemaining_ = 0;
    moved_ = true;
    inside_ = false;
    hovered_ = section;
    positionSponge();
    return true;
}
std::pair<int, int> WindowCleaning::move(int x, int y) {
    if (!rubbing())
        return {x, y};
    auto factor = [&](float min, float max) {
        return double(random_() % 100) * 0.01 * (double(max) - min) + min;
    };
    const auto nextX = std::max(0, int((double(x) - x_) * factor(speedMinX, speedMaxX) + x_));
    const auto nextY = std::max(0, int((double(y) - y_) * factor(speedMinY, speedMaxY) + y_));
    moved_ = moved_ || nextX != x_ || nextY != y_;
    x_ = nextX;
    y_ = nextY;
    positionSponge();
    return {x_, y_};
}
void WindowCleaning::cancel(bool hint) {
    if (!rubbing())
        return;
    phase_ = Phase::Idle;
    hideSponge();
    if (hint)
        emit(WindowCleaningEvent::Kind::Hint, "try_harder");
}
void WindowCleaning::leave() {
    if (!complete())
        phase_ = Phase::Idle;
    hideSponge();
    floraAge_.reset();
    object("flora").visible = !mixturePlaced_;
    object("flora").alpha = 1;
    for (size_t i = 0; i < sections_.size(); ++i) {
        auto& sprite = objects_[sections_[i]];
        sprite.visible = dirty_[i];
        sprite.alpha = dirty_[i] ? 1.f : 0.f;
    }
}
void WindowCleaning::finishClean() {
    dirty_[section_] = false;
    objects_[sections_[section_]].visible = false;
    const auto below = section_ + 2;
    if (below < sections_.size() && !dirty_[below]) {
        section_ = below;
        phase_ = Phase::FadeDirty;
        phaseAge_ = 0;
        auto& sprite = objects_[sections_[section_]];
        sprite.visible = true;
        reappear_.apply(sprite, 0);
        emit(WindowCleaningEvent::Kind::Sound, "sound/sfx/mg/window_cleaning/mg_section_dirty");
    } else if (std::none_of(dirty_.begin(), dirty_.end(), [](bool dirty) { return dirty; }))
        phase_ = Phase::Finished;
    else {
        phase_ = Phase::Idle;
        emit(WindowCleaningEvent::Kind::Hint, "sponge_dried_out");
    }
}
void WindowCleaning::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid window cleaning tick");
    if (floraAge_) {
        *floraAge_ += seconds;
        object("flora").alpha = float(std::max(0., 1 - *floraAge_));
        if (*floraAge_ >= 1) {
            object("flora").visible = false;
            floraAge_.reset();
        }
    }
    if (busy()) {
        phaseAge_ += seconds;
        auto& sprite = objects_[sections_[section_]];
        const auto& animation = phase_ == Phase::FadeClean ? disappear_ : reappear_;
        animation.apply(sprite, phaseAge_);
        if (phaseAge_ >= animation.duration()) {
            if (phase_ == Phase::FadeClean)
                finishClean();
            else {
                dirty_[section_] = true;
                phase_ = Phase::Idle;
                emit(WindowCleaningEvent::Kind::Hint, "section_dirtied");
            }
        }
        return;
    }
    if (!rubbing())
        return;
    idleAge_ = moved_ ? 0 : idleAge_ + seconds;
    moved_ = false;
    if (inside_ && idleAge_ < movementSeconds) {
        fxRemaining_ = 0;
        object("correct_fx").visible = wet_;
        successRemaining_ = std::max(0., successRemaining_ - seconds);
        if (successRemaining_ == 0) {
            hideSponge();
            if (wet_) {
                emit(WindowCleaningEvent::Kind::DrySponge);
                emit(WindowCleaningEvent::Kind::Sound, "sound/sfx/mg/mg_section_clean");
                phase_ = Phase::FadeClean;
                phaseAge_ = 0;
                disappear_.apply(objects_[sections_[section_]], 0);
            } else {
                phase_ = Phase::Idle;
                emit(WindowCleaningEvent::Kind::Hint, "need_wet_sponge");
            }
        }
    } else {
        if (fxRemaining_ == 0)
            fxRemaining_ = removeFxSeconds;
        fxRemaining_ = std::max(0., fxRemaining_ - seconds);
        if (fxRemaining_ == 0)
            object("correct_fx").visible = false;
        if (!inside_) {
            failureRemaining_ = std::max(0., failureRemaining_ - seconds);
            if (failureRemaining_ == 0) {
                const auto target = hovered_;
                const bool wet = wet_;
                cancel();
                if (target && *target < sections_.size() && dirty_[*target])
                    begin(*target, wet, x_, y_);
            }
        }
    }
}
} // namespace deadtime
