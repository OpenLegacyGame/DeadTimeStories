#include "hint_bottle.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <stdexcept>

namespace deadtime {
void HintBottle::restoreCharge(double charge) {
    if (!std::isfinite(charge) || charge < 0 || charge > 1)
        throw std::runtime_error("Invalid saved hint charge");
    charge_ = charge;
}
void HintBottle::load(const Assets& assets) {
    if (!assets.contains("deadtime.game"))
        return;
    const auto bytes = assets.read("deadtime.game");
    pugi::xml_document doc;
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid game configuration");
    const auto node = doc.child("deadtime").child("hint_bottle");
    if (!node)
        return;
    const auto hints = node.attribute("numHints").as_uint();
    const auto recharge = node.attribute("fullRechargeSeconds").as_double();
    if (!std::isfinite(recharge) || recharge <= 0)
        throw std::runtime_error("Invalid hint recharge time");
    // The native engine uses cost > 1 to disable hints when numHints is zero.
    cost_ = hints ? 1. / hints : 2;
    rechargeSeconds_ = recharge;
}
void HintBottle::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid hint recharge delta");
    charge_ = std::min(1., charge_ + seconds / rechargeSeconds_);
}
bool HintBottle::consume() {
    if (!available())
        return false;
    charge_ = std::max(0., charge_ - cost_);
    return true;
}
} // namespace deadtime
