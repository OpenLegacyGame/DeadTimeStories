#include "plantbook.hpp"
#include <pugixml.hpp>
#include <stdexcept>
namespace deadtime {
PlantBook::PlantBook(Assets& assets) {
    pugi::xml_document doc;
    const auto bytes = assets.read("plantbook/plantbook.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid plantbook configuration");
    const auto root = doc.child("plantbook");
    if (!root)
        throw std::runtime_error("Missing plantbook configuration");
    background = resolve("plantbook", root.child("background").attribute("src").value());
    x = root.child("position").attribute("x").as_float();
    y = root.child("position").attribute("y").as_float();
    leftX = root.child("left").attribute("relativeX").as_float();
    leftY = root.child("left").attribute("relativeY").as_float();
    rightX = root.child("right").attribute("relativeX").as_float();
    rightY = root.child("right").attribute("relativeY").as_float();
    // This game's HUD opens the plants section. The shipped journal/test page
    // has no production unlock or section-selection control.
    for (auto node : root.child("sections").child("plants").children("page")) {
        PlantPage page;
        page.name = node.text().as_string();
        if (page.name.empty() || page.name.find_first_of("/\\.#") != std::string::npos)
            throw std::runtime_error("Invalid plantbook page name");
        for (const auto& other : pages_)
            if (other.name == page.name)
                throw std::runtime_error("Duplicate plantbook page");
        page.layout = "plantbook/plants/" + page.name + "/page.gui";
        if (!assets.contains(page.layout))
            throw std::runtime_error("Missing plantbook page: " + page.layout);
        pages_.push_back(std::move(page));
    }
    if (pages_.empty())
        throw std::runtime_error("Empty plantbook");
}
void PlantBook::refresh(const std::function<bool(const std::string&)>& enabled) {
    visible_.clear();
    for (size_t i = 0; i < pages_.size(); ++i)
        if (enabled(pages_[i].name))
            visible_.push_back(i);
    if (index_ >= visible_.size())
        index_ = visible_.empty() ? 0 : ((visible_.size() - 1) / 2) * 2;
}
const PlantPage* PlantBook::left() const {
    return index_ < visible_.size() ? &pages_.at(visible_[index_]) : nullptr;
}
const PlantPage* PlantBook::right() const {
    return index_ + 1 < visible_.size() ? &pages_.at(visible_[index_ + 1]) : nullptr;
}
bool PlantBook::turn(int direction) {
    if (direction < 0 && hasPrevious()) {
        index_ -= 2;
        return true;
    }
    if (direction > 0 && hasNext()) {
        index_ += 2;
        return true;
    }
    return false;
}
} // namespace deadtime
