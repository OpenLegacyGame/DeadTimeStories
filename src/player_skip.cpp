#include "player.hpp"
#include <algorithm>
#include <cmath>
#include <utility>

namespace deadtime {
bool Player::puzzleSkipInput(const SDL_Event& event) {
    runtime_.setSkipBookOpen(bookOpen());
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        skipPressed_ = false;
    if (menuView_ != MenuView::None || hasDialog() || !runtime_.puzzleSkipVisible()) {
        skipPressed_ = false;
        return false;
    }
    if (event.type == SDL_MOUSEMOTION && skipPressed_)
        return true;
    if ((event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_MOUSEBUTTONUP) ||
        event.button.button != SDL_BUTTON_LEFT)
        return false;
    bool over = false;
    for (const auto& e : skipHud_)
        if (e.id == "button_skip_puzzle_disabled" &&
            hit(e.mask, e.x, e.y, e.width, e.height, float(event.button.x), float(event.button.y))) {
            over = true;
            break;
        }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        skipPressed_ = over;
        return over;
    }
    const bool pressed = skipPressed_;
    skipPressed_ = false;
    if (pressed && over)
        runtime_.requestPuzzleSkip();
    return pressed || over;
}
void Player::drawPuzzleSkip() {
    if (!runtime_.puzzleSkipVisible())
        return;
    const auto progress = float(runtime_.puzzleSkipProgress());
    const auto bar = std::find_if(skipHud_.begin(), skipHud_.end(),
                                  [](const auto& e) { return e.id == "skip_puzzle_progress"; });
    if (bar == skipHud_.end())
        return;
    const float filled = std::floor(bar->width * progress);
    for (auto e : skipHud_) {
        if (e.id == "button_skip_puzzle")
            e.enabled = progress == 1;
        if (e.id == "skip_puzzle_progress_position")
            e.x = bar->x + filled - e.width / 2;
        if (e.id == "skip_puzzle_progress") {
            // Native changes both width and U range, rather than stretching the
            // entire glowing texture into the fraction already filled.
            if (filled <= 0)
                continue;
            SDL_Rect prior;
            SDL_RenderGetClipRect(renderer_, &prior);
            const bool clipped = SDL_RenderIsClipEnabled(renderer_);
            SDL_Rect clip{int(e.x), int(e.y), int(filled), int(e.height)};
            if (clipped)
                SDL_IntersectRect(&clip, &prior, &clip);
            SDL_RenderSetClipRect(renderer_, &clip);
            drawGui({e});
            SDL_RenderSetClipRect(renderer_, clipped ? &prior : nullptr);
        } else
            drawGui({e});
    }
}
bool Player::confirmationInput(const SDL_Event& event) {
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        confirmationPressed_.clear();
    if (!confirmationOpen() || !dialogs_.front().started) {
        confirmationPressed_.clear();
        return false;
    }
    if (event.type == SDL_KEYDOWN) {
        // Source confirmation has only Esc -> No; Enter is not a Yes shortcut.
        if (!event.key.repeat && event.key.keysym.sym == SDLK_ESCAPE) {
            confirmationPressed_.clear();
            dialogInput("button_no");
        }
        return true;
    }
    if (event.type == SDL_MOUSEMOTION)
        return true;
    if (event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_MOUSEBUTTONUP)
        return false;
    if (event.button.button != SDL_BUTTON_LEFT)
        return true;
    std::string button;
    const auto& controls = dialogLayouts_.at("confirmation");
    for (auto it = controls.rbegin(); it != controls.rend(); ++it)
        if (it->kind == "button" && it->enabled && it->visible) {
            const auto& mask = it->mask.empty() ? it->normal : it->mask;
            if (hit(mask, it->x, it->y, it->width, it->height, float(event.button.x),
                    float(event.button.y))) {
                button = it->id;
                break;
            }
        }
    if (event.type == SDL_MOUSEBUTTONDOWN)
        confirmationPressed_ = button;
    else {
        const auto pressed = std::exchange(confirmationPressed_, {});
        if (!button.empty() && button == pressed)
            dialogInput(button);
    }
    return true;
}
} // namespace deadtime
