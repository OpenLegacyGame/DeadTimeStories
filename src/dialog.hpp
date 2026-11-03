#pragma once
#include "runtime.hpp"

namespace deadtime {
struct DialogPage {
    std::string text;
    double seconds = 0;
};
std::vector<DialogPage> dialogPages(std::string text);
struct Dialog {
    explicit Dialog(const Event& event);
    std::string kind, image, voice, after, origin;
    std::vector<DialogPage> pages;
    size_t page = 0;
    double age = 0;
    bool started = false;
    int voiceChannel = -1;
    void tick(double seconds);
    bool next(); // true only when the final page was dismissed
};
} // namespace deadtime
