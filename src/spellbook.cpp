#include "spellbook.hpp"
#include <algorithm>
#include <pugixml.hpp>
#include <stdexcept>

namespace deadtime {
SpellBook::SpellBook(const Assets& assets) {
    pugi::xml_document doc;
    auto bytes = assets.read("spellbook/spellbook.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid spellbook configuration");
    const auto root = doc.child("spellbook");
    background = resolve("spellbook", root.child("background").attribute("src").value());
    checkMark = resolve("spellbook", root.child("checkMark").attribute("src").value());
    x = root.child("position").attribute("x").as_float();
    y = root.child("position").attribute("y").as_float();
    pageX = root.child("pageOffset").attribute("relativeX").as_float();
    pageY = root.child("pageOffset").attribute("relativeY").as_float();
    checkX = root.child("checkMark").attribute("relativeX").as_float();
    checkY = root.child("checkMark").attribute("relativeY").as_float();
    for (bool recipe : {false, true})
        for (auto node : root.child(recipe ? "spells" : "instructions").children()) {
            SpellPage page;
            page.name = node.attribute("name").value();
            page.recipe = recipe;
            page.hidden = node.attribute("invisible").as_bool();
            if (page.name.empty() || page.name.find_first_of("/\\.#") != std::string::npos)
                throw std::runtime_error("Invalid spellbook page name");
            const auto base = "spellbook/" + std::string(recipe ? "sections/" : "instructions/") + page.name;
            page.layout = base + "/layout.gui";
            if (!assets.contains(page.layout))
                throw std::runtime_error("Missing spellbook page: " + page.layout);
            if (recipe) {
                pugi::xml_document ingredients;
                bytes = assets.read(base + "/ingredients.xml");
                if (!ingredients.load_buffer(bytes.data(), bytes.size()))
                    throw std::runtime_error("Invalid spell ingredients: " + page.name);
                for (auto item : ingredients.child("spell").children("ingredient"))
                    page.ingredients.push_back(
                        {item.attribute("item").value(), item.attribute("text").value(),
                         item.attribute("x").as_float(), item.attribute("y").as_float()});
            }
            pages_.push_back(std::move(page));
        }
    rebuild();
}
size_t& SpellBook::index() {
    return recipes_ ? recipeIndex_ : instructionIndex_;
}
size_t SpellBook::index() const {
    return recipes_ ? recipeIndex_ : instructionIndex_;
}
void SpellBook::rebuild() {
    visible_.clear();
    for (size_t i = 0; i < pages_.size(); ++i)
        if (pages_[i].recipe == recipes_ && (!pages_[i].hidden || pages_[i].enabled))
            visible_.push_back(i);
    if (visible_.empty())
        throw std::runtime_error("Empty spellbook section");
    index() = std::min(index(), visible_.size() - 1);
}
void SpellBook::refresh(const std::function<bool(const std::string&)>& enabled) {
    for (auto& page : pages_)
        if (page.recipe)
            page.enabled = enabled(page.name);
    rebuild();
}
void SpellBook::section(bool recipes) {
    recipes_ = recipes;
    rebuild();
}
void SpellBook::open(std::string_view latestSpell) {
    selectTab(true, latestSpell);
    if (!page().enabled)
        selectTab(false, latestSpell);
}
void SpellBook::selectTab(bool recipes, std::string_view latestSpell) {
    section(recipes);
    index() = 0;
    if (!recipes)
        return;
    bool foundLearned = false;
    for (size_t position = visible_.size(); position > 0; --position) {
        const auto& candidate = pages_[visible_[position - 1]];
        if (!candidate.enabled)
            continue;
        // Legacy checkpoints store unlock flags without their acquisition order.
        if (!foundLearned)
            index() = position - 1;
        if (candidate.name == latestSpell) {
            index() = position - 1;
            return;
        }
        foundLearned = true;
    }
}
// The original book treats instructions and recipes as consecutive sections
// for the arrow buttons, while the tabs can still select a section directly.
bool SpellBook::hasPrevious() const {
    return index() > 0 || recipes_;
}
bool SpellBook::hasNext() const {
    return index() + 1 < visible_.size() ||
           (!recipes_ && std::any_of(pages_.begin(), pages_.end(), [](const auto& page) {
               return page.recipe && (!page.hidden || page.enabled);
           }));
}
size_t SpellBook::pageCount() const {
    return visible_.size();
}
bool SpellBook::turn(int direction) {
    if (direction < 0 && hasPrevious()) {
        if (index() > 0) {
            --index();
        } else {
            section(false);
            index() = visible_.size() - 1;
        }
        return true;
    }
    if (direction > 0 && hasNext()) {
        if (index() + 1 < visible_.size()) {
            ++index();
        } else {
            section(true);
            index() = 0;
        }
        return true;
    }
    return false;
}
const SpellPage& SpellBook::page() const {
    return pages_.at(visible_.at(index()));
}
} // namespace deadtime
