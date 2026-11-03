#pragma once
#include "gui.hpp"
#include <functional>
namespace deadtime {
struct PlantPage {
    std::string name, layout;
};
class PlantBook {
  public:
    explicit PlantBook(Assets& assets);
    void refresh(const std::function<bool(const std::string&)>& enabled);
    bool turn(int direction);
    bool hasPrevious() const {
        return index_ > 0;
    }
    bool hasNext() const {
        return index_ + 2 < visible_.size();
    }
    size_t pageCount() const {
        return visible_.size();
    }
    size_t spreadIndex() const {
        return index_ / 2;
    }
    const PlantPage* left() const;
    const PlantPage* right() const;
    std::string background;
    float x = 0, y = 0, leftX = 0, leftY = 0, rightX = 0, rightY = 0;

  private:
    std::vector<PlantPage> pages_;
    std::vector<size_t> visible_;
    size_t index_ = 0;
};
} // namespace deadtime
