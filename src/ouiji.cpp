#include "ouiji.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>

namespace deadtime {
namespace {
constexpr double radians = 3.14159265358979323846 / 180;
std::pair<float, float> rotate(float x, float y, float angle) {
    const auto c = std::cos(angle * radians), s = std::sin(angle * radians);
    return {float(x * c - y * s), float(x * s + y * c)};
}
} // namespace
OuijiBoard::OuijiBoard(const Assets& assets) {
    const std::string base = "minigames/ouiji_board";
    pugi::xml_document doc;
    auto bytes = assets.read(base + "/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid Ouiji configuration");
    const auto config = doc.child("ouiji_board");
    alphabet_ = config.child("alphabet").text().as_string();
    phrase_ = config.child("phrase").text().as_string();
    auto lower = [](unsigned char c) { return char(c >= 'A' && c <= 'Z' ? c + 'a' - 'A' : c); };
    std::transform(alphabet_.begin(), alphabet_.end(), alphabet_.begin(), lower);
    std::transform(phrase_.begin(), phrase_.end(), phrase_.begin(), lower);
    phrase_.erase(std::remove_if(phrase_.begin(), phrase_.end(),
                                 [&](char c) { return alphabet_.find(c) == std::string::npos; }),
                  phrase_.end());
    speed_ = config.child("planchette_move_speed").attribute("pixels_per_second").as_double();
    if (alphabet_.empty() || phrase_.empty() || !std::isfinite(speed_) || speed_ <= 0)
        throw std::runtime_error("Invalid Ouiji alphabet, phrase or speed");
    onFinish = resolve(base, config.child("onFinish").attribute("script").value()) + ".lua";
    objects_ = loadActivityLayout(assets, base + "/layout.xml");
    planchette();
    for (const auto* id : {"planchette", "back_no_board"}) {
        std::vector<ActivityObject> measured{object(id)};
        measureActivityTextures(assets, measured);
        mutableObject(id).width = measured.front().width;
        mutableObject(id).height = measured.front().height;
    }
    for (const auto* id : {"back", "back_no_board", "fx_open", "exit"})
        object(id);
    for (unsigned i = 0; i < fades_.size(); ++i) {
        fades_[i] =
            std::make_unique<AnimationSet>(assets, base + "/fx_open_" + std::to_string(i + 1) + ".animset");
        if (fades_[i]->looped() || fades_[i]->duration() <= 0)
            throw std::runtime_error("Invalid Ouiji opening fade");
    }
    enter();
}
const ActivityObject& OuijiBoard::object(const std::string& id) const {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [&](const auto& o) { return o.id == id; });
    if (found == objects_.end())
        throw std::runtime_error("Missing Ouiji object: " + id);
    return *found;
}
ActivityObject& OuijiBoard::mutableObject(const std::string& id) {
    return const_cast<ActivityObject&>(object(id));
}
void OuijiBoard::enter() {
    exitRequested = false;
    moving_ = false;
    opening_ = false;
    feedback.clear();
    updateVisuals();
}
void OuijiBoard::leave() {
    // Native 4B9BF0 clears the input string (+56) until reward flag +23 is set;
    // the solved flag (+22) remains independent, so an opened book stays open.
    if (!claimed_)
        entered_.clear();
    updateVisuals();
}
void OuijiBoard::updateVisuals() {
    const bool book = solved() && (!opening_ || fadePhase_ == 1);
    for (auto& o : objects_) {
        if (o.id == "back")
            o.visible = !book;
        else if (o.id == "back_no_board")
            o.visible = book;
        else if (o.id == "planchette" || o.id == "yes" || o.id == "no" || o.id == "goodbye" ||
                 o.id.starts_with("smb"))
            o.visible = !book;
        if (o.id.size() == 9 && o.id.starts_with("smb") && o.id.ends_with("_hl")) {
            const auto index = std::stoul(o.id.substr(3, 3));
            o.alpha = index > 0 && index <= alphabet_.size() &&
                              entered_.find(alphabet_[index - 1]) != std::string::npos
                          ? 1.f
                          : 0.f;
        }
    }
    auto& fade = mutableObject("fx_open");
    fade.visible = opening_;
    if (opening_)
        fades_[fadePhase_]->apply(fade, fadeAge_);
}
void OuijiBoard::tickOpening(double seconds) {
    while (opening_) {
        const auto step = std::min(seconds, std::max(0., fades_[fadePhase_]->duration() - fadeAge_));
        fadeAge_ += step;
        seconds -= step;
        if (fadeAge_ < fades_[fadePhase_]->duration())
            break;
        if (++fadePhase_ == fades_.size())
            opening_ = false;
        fadeAge_ = 0;
        if (seconds <= 0)
            break;
    }
    updateVisuals();
}
ActivityObject& OuijiBoard::planchette() {
    const auto found =
        std::find_if(objects_.begin(), objects_.end(), [](const auto& o) { return o.id == "planchette"; });
    if (found == objects_.end())
        throw std::runtime_error("Missing Ouiji planchette");
    return *found;
}
void OuijiBoard::setPlanchetteSize(float width, float height) {
    if (width <= 0 || height <= 0)
        throw std::runtime_error("Invalid planchette dimensions");
    planchette().width = width;
    planchette().height = height;
}
bool OuijiBoard::select(const std::string& id) {
    if (busy())
        return false;
    if (id == "exit") {
        exitRequested = true;
        return true;
    }
    if (solved()) {
        if (id != "back_no_board" || claimed_)
            return false;
        claimed_ = true;
        exitRequested = true;
        return true;
    }
    auto target = std::find_if(objects_.begin(), objects_.end(), [&](const auto& o) { return o.id == id; });
    if (target == objects_.end() || !target->visible || !target->interactive)
        return false;
    if (id != "yes" && id != "no" && id != "goodbye" && !(id.size() == 6 && id.starts_with("smb")))
        return false;
    if (id.starts_with("smb")) {
        if (id.substr(3).find_first_not_of("0123456789") != std::string::npos)
            return false;
        const auto index = std::stoul(id.substr(3));
        if (!index || index > alphabet_.size())
            return false;
    }
    auto& pointer = planchette();
    if (pointer.width <= 0 || pointer.height <= 0)
        throw std::runtime_error("Planchette texture dimensions not loaded");
    const auto delta =
        rotate((target->width - pointer.width) / 2, (target->height - pointer.height) / 2, target->angle);
    startX_ = pointer.x;
    startY_ = pointer.y;
    startAngle_ = pointer.angle;
    targetX_ = target->x + delta.first;
    targetY_ = target->y + delta.second;
    targetAngle_ = target->angle;
    duration_ = std::hypot(targetX_ - startX_, targetY_ - startY_) / speed_;
    selected_ = id;
    age_ = 0;
    moving_ = true;
    feedback.clear();
    return true;
}
bool OuijiBoard::skip() {
    if (solved_)
        return false;
    // Native 4B9EB0/4BA150 sets solved, plays finish and reinitializes directly
    // into the book view. It neither types the answer nor claims the reward.
    solved_ = true;
    moving_ = opening_ = false;
    selected_.clear();
    age_ = duration_ = fadeAge_ = 0;
    fadePhase_ = 0;
    feedback = "sound/sfx/mg/mg_finish";
    updateVisuals();
    return true;
}
void OuijiBoard::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid Ouiji tick");
    if (opening_) {
        tickOpening(seconds);
        return;
    }
    if (!moving_)
        return;
    age_ += seconds;
    const auto fraction = duration_ > 0 ? std::min(1., age_ / duration_) : 1.;
    auto& pointer = planchette();
    pointer.x = startX_ + float(fraction) * (targetX_ - startX_);
    pointer.y = startY_ + float(fraction) * (targetY_ - startY_);
    pointer.angle = startAngle_ + float(fraction) * (targetAngle_ - startAngle_);
    if (fraction < 1)
        return;
    moving_ = false;
    if (selected_ == "goodbye") {
        exitRequested = true;
        return;
    }
    if (!selected_.starts_with("smb"))
        return;
    const auto number = std::stoul(selected_.substr(3));
    if (!number || number > alphabet_.size())
        return;
    const auto attempt = entered_ + alphabet_[number - 1];
    size_t matched = 0;
    while (matched < attempt.size() && matched < phrase_.size() && attempt[matched] == phrase_[matched])
        ++matched;
    entered_ = attempt.substr(0, matched);
    feedback = matched == attempt.size() ? "sound/sfx/mg/mg_high_light" : "sound/sfx/mg/mg_wrong_name";
    solved_ = entered_ == phrase_;
    if (solved()) {
        feedback = "sound/sfx/mg/mg_finish";
        opening_ = true;
        fadePhase_ = 0;
        fadeAge_ = 0;
        tickOpening(std::max(0., age_ - duration_));
    } else
        updateVisuals();
}
} // namespace deadtime
