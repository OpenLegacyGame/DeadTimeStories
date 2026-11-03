#pragma once
#include "assets.hpp"

namespace deadtime {
class HintBottle {
  public:
    void load(const Assets& assets);
    void tick(double seconds);
    void refill() {
        charge_ = 1;
    }
    bool consume();
    double charge() const {
        return charge_;
    }
    void restoreCharge(double charge);
    bool available() const {
        return charge_ >= cost_;
    }

  private:
    double charge_ = 1, cost_ = 1, rechargeSeconds_ = 120;
};
} // namespace deadtime
