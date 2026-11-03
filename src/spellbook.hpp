#pragma once
#include "assets.hpp"
#include <functional>
#include <string_view>

namespace deadtime {
struct SpellIngredient {
    std::string item, text;
    float x = 0, y = 0;
};
struct SpellPage {
    std::string name, layout;
    bool recipe = false, hidden = false, enabled = false;
    std::vector<SpellIngredient> ingredients;
};
class SpellBook {
  public:
    explicit SpellBook(const Assets& assets);
    void refresh(const std::function<bool(const std::string&)>& enabled);
    void section(bool recipes);
    void open(std::string_view latestSpell);
    void selectTab(bool recipes, std::string_view latestSpell);
    bool turn(int direction);
    const SpellPage& page() const;
    bool hasPrevious() const;
    bool hasNext() const;
    bool recipes() const {
        return recipes_;
    }
    size_t pageCount() const;
    std::string background, checkMark;
    float x = 0, y = 0, pageX = 0, pageY = 0, checkX = 0, checkY = 0;

  private:
    std::vector<SpellPage> pages_;
    std::vector<size_t> visible_;
    size_t recipeIndex_ = 0, instructionIndex_ = 0;
    bool recipes_ = true;
    size_t& index();
    size_t index() const;
    void rebuild();
};
} // namespace deadtime
