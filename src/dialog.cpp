#include "dialog.hpp"
#include <cmath>
#include <stdexcept>

namespace deadtime {
std::vector<DialogPage> dialogPages(std::string text) {
    if (text.size() >= 2 && text.front() == '%' && text.back() == '%')
        text = text.substr(1, text.size() - 2);
    std::vector<DialogPage> pages;
    size_t start = 0;
    while (start < text.size()) {
        const auto separator = text.find('|', start);
        if (separator == std::string::npos) {
            pages.push_back({text.substr(start), 0});
            break;
        }
        const auto end = text.find('|', separator + 1);
        const auto timing = text.substr(separator + 1, end == std::string::npos ? end : end - separator - 1);
        if (timing.empty() || timing.find_first_not_of("0123456789") != std::string::npos)
            throw std::runtime_error("Invalid dialogue timing");
        pages.push_back({text.substr(start, separator - start), std::stod(timing) * 0.001});
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    if (pages.empty())
        pages.push_back({"", 0});
    return pages;
}
Dialog::Dialog(const Event& event) : origin(event.origin) {
    auto arg = [&](size_t i) { return i < event.args.size() ? event.args[i] : std::string{}; };
    if (event.type == "showTalkingFaceDialog") {
        kind = "talking_face";
        image = arg(1);
        voice = arg(2);
        after = arg(3);
    } else if (event.type == "showMessageDialogWithImage") {
        kind = "message_with_image";
        image = arg(1);
        after = arg(2);
    } else if (event.type == "showMessageDialogWithSound") {
        kind = "message";
        voice = arg(1);
        after = arg(2);
    } else if (event.type == "showMessageDialog") {
        kind = "message";
        after = arg(1);
    } else if (event.type == "showConfirmationDialog") {
        kind = "confirmation";
        after = arg(1);
    } else if (event.type == "showDialog" && arg(0) == "help_screen") {
        kind = "help_screen";
    } else
        throw std::runtime_error("Unsupported dialog: " + event.type + " " + arg(0));
    pages = dialogPages(kind == "help_screen" ? "" : arg(0));
}
void Dialog::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid dialogue time");
    age += seconds;
    while (page + 1 < pages.size() && pages[page].seconds > 0 && age >= pages[page].seconds) {
        age -= pages[page].seconds;
        ++page;
    }
}
bool Dialog::next() {
    if (page + 1 >= pages.size())
        return true;
    ++page;
    age = 0;
    return false;
}
} // namespace deadtime
