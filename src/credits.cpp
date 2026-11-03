#include "credits.hpp"
#include <algorithm>
#include <cmath>
#include <pugixml.hpp>
#include <sstream>
#include <stdexcept>

namespace deadtime {
Credits::Credits(const Assets& assets, const Measure& measure) {
    const auto bytes = assets.read("credits/config.xml");
    pugi::xml_document document;
    if (!document.load_buffer(bytes.data(), bytes.size()) || !document.child("credits"))
        throw std::runtime_error("Invalid credits configuration");
    const auto config = document.child("credits");
    start_ = config.child("text_y").attribute("start_top").as_int();
    const int end = config.child("text_y").attribute("end_bottom").as_int();
    speed_ = config.child("scroll_speed").attribute("px_per_sec").as_double();
    if (!std::isfinite(speed_) || speed_ <= 0)
        throw std::runtime_error("Invalid credits scroll speed");
    auto text = assets.read("credits/credits.txt");
    if (text.starts_with("\xEF\xBB\xBF"))
        text.erase(0, 3);
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    std::istringstream stream(text);
    std::string line;
    float baseline = 0;
    while (std::getline(stream, line)) {
        const char* style = "font_general";
        if (!line.empty() && (line.front() == '!' || line.front() == '#')) {
            style = line.front() == '!' ? "font_heading" : "font_section";
            line.erase(0, 1);
        }
        const auto definition = config.child(style);
        GuiElement element;
        element.id = "credit_" + std::to_string(lines_.size());
        element.kind = "label";
        element.font = resolve("credits", definition.attribute("src").value());
        element.color =
            unsigned(std::stoul(definition.attribute("color").as_string("FFFFFFFF"), nullptr, 16));
        element.text = line;
        element.width = 1024;
        element.baseline = true;
        element.verticalAlign = "top";
        const auto metrics = measure(element.font, line, 1024);
        if (!std::isfinite(metrics.ascent) || !std::isfinite(metrics.lineHeight) ||
            !std::isfinite(metrics.textHeight) || metrics.lineHeight <= 0 || metrics.textHeight < 0)
            throw std::runtime_error("Invalid credits font metrics");
        // Native 55FAA0 uses the first font's ascender for the first baseline,
        // then advances by max(laid-out text height, face line height).
        if (lines_.empty())
            baseline = std::floor(metrics.ascent + .5f);
        element.y = baseline;
        element.height = std::max(metrics.textHeight, metrics.lineHeight);
        baseline = std::floor(baseline + element.height + .5f);
        lines_.push_back(std::move(element));
    }
    if (lines_.empty())
        throw std::runtime_error("Empty credits text");
    finish_ = int(end - baseline);
}
int Credits::offset() const {
    return int(start_ - elapsed_ * speed_);
}
void Credits::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid credits time");
    if (complete_)
        return;
    elapsed_ += seconds;
    // Native 560950 uses an integer coordinate and a strict bottom boundary.
    if (offset() < finish_)
        complete_ = true;
}
std::vector<GuiElement> Credits::elements() const {
    GuiElement back;
    back.kind = "image";
    back.normal = "credits/back";
    std::vector<GuiElement> result{back};
    for (auto line : lines_) {
        line.y += float(offset());
        if (line.y < 768 && line.y + line.height > 0)
            result.push_back(std::move(line));
    }
    GuiElement overlay;
    overlay.kind = "image";
    overlay.normal = "credits/overlay";
    result.push_back(std::move(overlay));
    return result;
}
} // namespace deadtime
