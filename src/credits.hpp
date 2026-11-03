#pragma once
#include "gui.hpp"
#include <functional>

namespace deadtime {
struct CreditMetrics {
    float ascent = 0, lineHeight = 0, textHeight = 0;
};
class Credits {
  public:
    using Measure = std::function<CreditMetrics(const std::string&, const std::string&, unsigned)>;
    Credits(const Assets& assets, const Measure& measure);
    void tick(double seconds);
    void dismiss() {
        complete_ = true;
    }
    bool complete() const {
        return complete_;
    }
    int offset() const;
    double duration() const {
        return (start_ - finish_) / speed_;
    }
    double elapsed() const {
        return elapsed_;
    }
    const auto& lines() const {
        return lines_;
    }
    std::vector<GuiElement> elements() const;

  private:
    std::vector<GuiElement> lines_;
    double speed_ = 0, elapsed_ = 0;
    int start_ = 0, finish_ = 0;
    bool complete_ = false;
};
} // namespace deadtime
