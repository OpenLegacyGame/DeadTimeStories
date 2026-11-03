#include "player.hpp"
#include <algorithm>
#include <stdexcept>

namespace deadtime {
bool Player::saveSafe() const {
    return saves_ && !worldProfile_.empty() &&
           std::find(profiles_->ids().begin(), profiles_->ids().end(), worldProfile_) !=
               profiles_->ids().end() &&
           dialogs_.empty() && pointerButtons_ == 0 && !draggingInventory_ && pressedInventory_.empty() &&
           !activityPressed_ && !doveRightPressed_ && !windowRubInput_ && !mixStirInput_ && !mixMortarHeld_ &&
           !spiritLockGrip_ && !circleStrokeInput_ && !revealPressed_ && !revealPotPressed_ &&
           (!runtime_.sail() || !runtime_.sail()->gripping()) &&
           (!runtime_.voodooCircle() || !runtime_.voodooCircle()->pressing()) && runtime_.checkpointReady();
}
void Player::saveProgress(bool force) {
    if (!saves_ || restartGame_)
        return;
    if (!force && !saveRequested_ && elapsed_ - lastSave_ < 5)
        return;
    if (!saveSafe())
        return;
    const auto bytes = runtime_.checkpoint();
    if (bytes != lastSaved_) {
        saves_->save(worldProfile_, bytes);
        lastSaved_ = bytes;
    }
    lastSave_ = elapsed_;
    saveRequested_ = false;
}
void Player::cancelPointerInput() {
    pointerButtons_ = 0;
    suppressPointerUp_ = true;
    locationHintPressedButton_.clear();
    carriagePressedButton_.clear();
    hintBottlePressed_ = false;
    skipPressed_ = false;
    confirmationPressed_.clear();
    spellBookPressed_ = nullptr;
    if (auto* window = runtime_.windowCleaning())
        window->cancel();
    if (auto* mixing = runtime_.mixing())
        mixing->releaseStir();
    if (runtime_.revealSpirits())
        runtime_.revealSpirits()->pauseMotion();
    if (spiritLockGrip_ && runtime_.spiritLock())
        runtime_.spiritLock()->release();
    if (runtime_.sail())
        runtime_.sail()->cancelGrip();
    if (runtime_.voodooCircle())
        runtime_.voodooCircle()->release();
    windowRubInput_ = activityPressed_ = doveRightPressed_ = mixStirInput_ = mixMortarHeld_ =
        spiritLockGrip_ = circleStrokeInput_ = false;
    revealPotPressed_ = revealPressed_ = false;
    pressedInventory_.clear();
    draggingInventory_ = false;
    mixHeld_.clear();
    voodooHeld_.clear();
}
bool Player::pauseMenu() {
    if (!saves_ || menuView_ != MenuView::None || bookOpen() || !saveSafe())
        return false;
    saveProgress(true);
    openMenu(MenuView::Pause);
    return true;
}
bool Player::tryHudMenu(float x, float y) {
    if (!saves_ || !dialogs_.empty() || runtime_.cutscene() || bookOpen() || windowRubInput_ ||
        mixStirInput_ || spiritLockGrip_ || circleStrokeInput_ || revealPressed_ ||
        (runtime_.sail() && runtime_.sail()->gripping()))
        return false;
    for (const auto& element : leftHud_)
        if (element.id == "button_menu" && element.visible && element.enabled &&
            hit(element.mask.empty() ? element.normal : element.mask, element.x, element.y, element.width,
                element.height, x, y)) {
            activityPressed_ = false;
            pressedInventory_.clear();
            draggingInventory_ = false;
            pauseMenu();
            return true;
        }
    return false;
}
void Player::resumeGame() {
    if (runtime_.layers().empty() || runtime_.flag("gameCompleted"))
        return;
    if (menuView_ == MenuView::Pause) {
        settings_->save();
        optionsMenu_.reset();
        menuView_ = MenuView::None;
        menuPressed_.clear();
        if (audio_)
            for (auto [channel, chunk] : pausedChannels_)
                if (Mix_GetChunk(channel) == chunk && Mix_Paused(channel))
                    Mix_Resume(channel);
        pausedChannels_.clear();
        sound("sound/sfx/sfx_m_close");
        return;
    }
    menuView_ = MenuView::None;
    menuPressed_.clear();
    currentScene_.clear();
    if (audio_)
        Mix_HaltChannel(-1);
    spiritLockSoundChannel_ = skullsWheelChannel_ = circleChalkChannel_ = activityAmbientChannel_ = -1;
}
void Player::openMenu(MenuView view) {
    if (saves_ && !dialogs_.empty())
        throw std::runtime_error("Cannot replace an unconfirmed game dialog with a menu");
    SDL_StopTextInput();
    if (view == MenuView::Pause) {
        pausedChannels_.clear();
        if (audio_)
            for (int channel = 0; channel < Mix_AllocateChannels(-1); ++channel)
                if (Mix_Playing(channel) && !Mix_Paused(channel)) {
                    pausedChannels_[channel] = Mix_GetChunk(channel);
                    Mix_Pause(channel);
                }
        sound("sound/sfx/sfx_m_open");
    } else if (menuView_ == MenuView::None || menuView_ == MenuView::Pause) {
        pausedChannels_.clear();
        // A stopped SDL channel must not complete pronunciation while paused.
        if (spiritLockSoundChannel_ >= 0 && runtime_.spiritLock())
            runtime_.spiritLock()->useAudioCallback(false);
        spiritLockSoundChannel_ = -1;
        if (audio_)
            Mix_HaltChannel(-1);
        sound("music/01_DTS_Theme", -1);
    }
    menuView_ = view;
    menuPressed_.clear();
    pressedInventory_.clear();
    draggingInventory_ = false;
    if (view != MenuView::Pause) {
        selection_.clear();
        if (inventoryFeedback_)
            inventoryFeedback_->clear();
        inventoryTutorials_.clear();
        if (inventoryPanel_) {
            inventoryPanel_->reset();
            inventoryAllowed_ = false;
            syncInventoryPanel();
        }
        hint_.clear();
        hintBubble_.clear();
        clearTutorial();
    }
    if (view == MenuView::Pause || view == MenuView::Options)
        optionsMenu_ = std::make_unique<OptionsMenu>(assets_, *settings_, view == MenuView::Pause);
    else
        optionsMenu_.reset();
    activityPressed_ = false;
    spellBookOpen_ = plantBookOpen_ = false;
    plantBookPressed_.clear();
    dialogs_.clear();
    if (view == MenuView::Credits) {
        credits_ = std::make_unique<Credits>(
            assets_, [&](const std::string& path, const std::string& text, unsigned width) {
                auto* face = font(path);
                float height = 0;
                if (!text.empty()) {
                    auto* surface =
                        TTF_RenderUTF8_Blended_Wrapped(face, text.c_str(), {255, 255, 255, 255}, width);
                    if (!surface)
                        throw std::runtime_error(TTF_GetError());
                    height = float(surface->h);
                    SDL_FreeSurface(surface);
                }
                return CreditMetrics{float(TTF_FontAscent(face)), float(TTF_FontLineSkip(face)), height};
            });
    } else {
        if (!menuScene_)
            menuScene_ = std::make_unique<MenuScene>(assets_);
        if (mainMenu_.empty()) {
            mainMenu_ = loadGui(assets_, "menu/mainmenu/layout.gui");
            for (auto& element : mainMenu_) {
                if (element.id == "continue")
                    element.visible = false;
                if (element.id == "profiles")
                    element.enabled = profiles_ != nullptr;
            }
            playAgainMenu_ = loadGui(assets_, "menu/play_again/layout.gui");
        }
        const bool continuing = saves_ && !runtime_.layers().empty() && !runtime_.flag("gameCompleted");
        for (auto& element : mainMenu_) {
            if (element.id == "continue")
                element.visible = continuing;
            if (element.id == "start")
                element.visible = !continuing;
        }
        if (view == MenuView::Profiles) {
            if (!profiles_)
                throw std::runtime_error("Profile menu requires a user store");
            if (!profileMenu_)
                profileMenu_ = std::make_unique<ProfileMenu>(assets_, *profiles_);
            else
                profileMenu_->open();
            if (profileMenu_->view() == ProfileMenu::View::Create)
                SDL_StartTextInput();
        }
    }
}
std::vector<GuiElement> Player::menuElements() const {
    if (optionsMenu_)
        return optionsMenu_->elements();
    return menuView_ == MenuView::Profiles    ? profileMenu_->elements()
           : menuView_ == MenuView::PlayAgain ? playAgainMenu_
                                              : mainMenu_;
}
std::string Player::menuButtonAt(float x, float y) {
    const auto elements = menuElements();
    for (auto it = elements.rbegin(); it != elements.rend(); ++it)
        if ((it->kind == "button" || it->kind == "checkbox" || it->kind == "slider") && it->enabled &&
            it->visible) {
            const auto& mask = it->mask.empty() ? it->normal : it->mask;
            if (hit(mask, it->x, it->y, it->width, it->height, x, y))
                return it->id;
        }
    return {};
}
void Player::menuAction(const std::string& button) {
    if (optionsMenu_) {
        if (button == "windowed")
            changeWindowMode();
        else if (button == "return_to_game")
            resumeGame();
        else if (button == "exit" || button == "return_to_mainmenu") {
            settings_->save();
            openMenu(MenuView::Main);
        }
        return;
    }
    if (menuView_ == MenuView::Profiles) {
        const auto previous = profiles_->ids();
        profileMenu_->action(button);
        if (saves_)
            for (const auto& id : previous)
                if (std::find(profiles_->ids().begin(), profiles_->ids().end(), id) == profiles_->ids().end())
                    saves_->discard(id);
        if (profileMenu_->closed() && saves_ && profiles_->current() &&
            profiles_->ids().at(*profiles_->current()) != worldProfile_) {
            reloadProfile_ = true;
            quit_ = true;
            SDL_StopTextInput();
        } else if (profileMenu_->closed())
            openMenu(MenuView::Main);
        else if (profileMenu_->view() == ProfileMenu::View::Create)
            SDL_StartTextInput();
        else
            SDL_StopTextInput();
        return;
    }
    if (menuView_ == MenuView::PlayAgain) {
        if (button == "no")
            openMenu(MenuView::Main);
        else if (button == "yes") {
            restartGame_ = true;
            quit_ = true;
        }
        return;
    }
    if (button == "credits")
        openMenu(MenuView::Credits);
    else if (button == "options")
        openMenu(MenuView::Options);
    else if (button == "profiles")
        openMenu(MenuView::Profiles);
    else if (button == "exit")
        quit_ = true;
    else if (button == "continue")
        resumeGame();
    else if (button == "start") {
        if (runtime_.flag("gameCompleted"))
            openMenu(MenuView::PlayAgain);
        else {
            restartGame_ = true;
            quit_ = true;
        }
    }
}
bool Player::menuInput(const SDL_Event& event) {
    if (menuView_ == MenuView::None)
        return false;
    if (event.type == SDL_QUIT) {
        if (optionsMenu_)
            settings_->save();
        quit_ = true;
        return true;
    }
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        menuPressed_.clear();
        if (optionsMenu_) {
            optionsMenu_->release();
            settings_->save();
        }
    }
    if (menuView_ == MenuView::Credits) {
        if (event.type == SDL_KEYDOWN ||
            (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT))
            openMenu(MenuView::Main);
    } else {
        if (menuView_ == MenuView::Profiles) {
            if (event.type == SDL_TEXTINPUT)
                textInput(event.text.text);
            if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                const auto key = event.key.keysym.sym;
                if (key == SDLK_BACKSPACE)
                    profileMenu_->backspace();
                else if (key == SDLK_UP)
                    menuAction("navigate_up");
                else if (key == SDLK_DOWN)
                    menuAction("navigate_down");
                else if (key == SDLK_DELETE)
                    menuAction("delete_shortcut");
                else if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
                    menuAction(profileMenu_->view() == ProfileMenu::View::Create   ? "create"
                               : profileMenu_->view() == ProfileMenu::View::Delete ? "yes"
                                                                                   : "exit");
                else if (key == SDLK_ESCAPE)
                    menuAction(profileMenu_->view() == ProfileMenu::View::Create   ? "cancel"
                               : profileMenu_->view() == ProfileMenu::View::Delete ? "no"
                                                                                   : "");
            }
        }
        if (optionsMenu_ && event.type == SDL_MOUSEMOTION && optionsMenu_->dragging()) {
            optionsMenu_->move(float(event.motion.x), float(event.motion.y));
            applyVolumes();
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            menuPressed_ = menuButtonAt(float(event.button.x), float(event.button.y));
            if (optionsMenu_ &&
                optionsMenu_->begin(menuPressed_, float(event.button.x), float(event.button.y)))
                menuPressed_.clear();
        }
        if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
            if (optionsMenu_ && optionsMenu_->dragging()) {
                optionsMenu_->move(float(event.button.x), float(event.button.y));
                optionsMenu_->release();
                applyVolumes();
                settings_->save();
                menuPressed_.clear();
                return true;
            }
            const auto button = menuButtonAt(float(event.button.x), float(event.button.y));
            const bool activate = !button.empty() && button == menuPressed_;
            menuPressed_.clear();
            if (activate)
                menuAction(button);
        }
        if (event.type == SDL_KEYDOWN && !event.key.repeat &&
            (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) &&
            menuView_ == MenuView::Options)
            menuAction("exit");
        if (event.type == SDL_KEYDOWN && !event.key.repeat && event.key.keysym.sym == SDLK_ESCAPE) {
            if (menuView_ == MenuView::PlayAgain)
                menuAction("no");
            else if (menuView_ == MenuView::Pause)
                resumeGame();
            else if (menuView_ == MenuView::Options)
                menuAction("exit");
            else if (menuView_ == MenuView::Main && saves_)
                resumeGame();
        }
    }
    return true;
}
void Player::textInput(const std::string& text) {
    if (menuView_ == MenuView::Profiles && profileMenu_->text(text))
        sound("sound/sfx/sfx_typing");
}
void Player::drawMenu() {
    if (menuView_ == MenuView::Credits) {
        drawGui(credits_->elements());
        return;
    }
    int px = 0, py = 0;
    SDL_GetMouseState(&px, &py);
    float x = 0, y = 0;
    SDL_RenderWindowToLogical(renderer_, px, py, &x, &y);
    if (menuView_ == MenuView::Pause)
        renderWorld();
    else {
        menuScene_->hover(menuView_ == MenuView::Main ? menuButtonAt(x, y) : "");
        drawScene(menuScene_->scene());
    }
    if (profiles_ && profiles_->current())
        for (auto& element : mainMenu_)
            if (element.id == "current_profile")
                element.text = profiles_->names().at(*profiles_->current());
    drawGui(menuElements());
}
} // namespace deadtime
