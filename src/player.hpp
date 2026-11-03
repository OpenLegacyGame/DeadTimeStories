#pragma once
#include "animation.hpp"
#include "collada.hpp"
#include "credits.hpp"
#include "dialog.hpp"
#include "game_saves.hpp"
#include "gui.hpp"
#include "inventory_feedback.hpp"
#include "inventory_panel.hpp"
#include "menu_scene.hpp"
#include "options_menu.hpp"
#include "plantbook.hpp"
#include "profiles.hpp"
#include "runtime.hpp"
#include "spellbook.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <deque>
#include <memory>

namespace deadtime {
struct PlayerOptions {
    std::string snapshot;
    int frames = 0;
    bool audio = true;
    bool menu = false, credits = false, profileMenu = false;
    std::function<void()> frameInput;
};
class Player {
  public:
    Player(Assets& assets, Runtime& runtime, Profiles* profiles = nullptr, GameSaves* saves = nullptr,
           Settings* settings = nullptr);
    ~Player();
    int run(const PlayerOptions& options);
    bool menuInput(const SDL_Event& event);
    bool pauseMenu();
    bool plantBookOpen() const {
        return plantBookOpen_;
    }
    bool spellBookOpen() const {
        return spellBookOpen_;
    }
    const PlantBook* plantBook() const {
        return plantBook_.get();
    }
    std::optional<TutorialId> tutorial() const {
        return tutorialId_;
    }
    bool confirmationOpen() const {
        return hasDialog() && dialogs_.front().kind == "confirmation";
    }
    const InventoryFeedback* inventoryFeedback() const {
        return inventoryFeedback_.get();
    }
    const std::string& selectedInventory() const {
        return selection_;
    }
    bool inventoryExpanded() const {
        return inventoryOpen_;
    }
    const InventoryPanel* inventoryPanel() const {
        return inventoryPanel_.get();
    }
    enum class MenuView { None, Main, Credits, PlayAgain, Profiles, Options, Pause };
    MenuView menuView() const {
        return menuView_;
    }
    const Credits* credits() const {
        return credits_.get();
    }
    static constexpr int RestartGame = 3;
    static constexpr int ReloadProfile = 4;

  private:
    friend class CampaignPresentation;
    Settings inspectionSettings_;
    Settings* settings_;
    std::unique_ptr<OptionsMenu> optionsMenu_;
    std::map<int, AudioBus> channelBuses_;
    std::map<int, Mix_Chunk*> pausedChannels_;
    void applyVolumes();
    void changeWindowMode();
    std::vector<GuiElement> menuElements() const;
    void renderWorld();
    MenuView menuView_ = MenuView::None;
    bool restartGame_ = false, reloadProfile_ = false;
    GameSaves* saves_ = nullptr;
    std::string worldProfile_, lastSaved_;
    double lastSave_ = -5;
    bool saveRequested_ = false, suppressPointerUp_ = false;
    Uint32 pointerButtons_ = 0;
    bool saveSafe() const;
    void saveProgress(bool force);
    void cancelPointerInput();
    void resumeGame();
    bool tryHudMenu(float x, float y);
    std::unique_ptr<Credits> credits_;
    std::unique_ptr<MenuScene> menuScene_;
    Profiles* profiles_ = nullptr;
    std::unique_ptr<ProfileMenu> profileMenu_;
    std::vector<GuiElement> mainMenu_, playAgainMenu_;
    std::string menuPressed_;
    void openMenu(MenuView view);
    void drawMenu();
    std::string menuButtonAt(float x, float y);
    void menuAction(const std::string& button);
    void textInput(const std::string& text);
    struct Image {
        SDL_Texture* texture = nullptr;
        int width = 0, height = 0;
        std::vector<unsigned char> mask;
        ~Image() {
            if (texture)
                SDL_DestroyTexture(texture);
        }
    };
    Image* image(const std::string& name);
    void drawScene(const Scene& scene);
    void drawModel(const Object& object, bool bottomOrigin = false);
    void drawParticles(const Object& object);
    void drawParticleBatches(const std::vector<ParticleBatch>& batches, float alpha = 1,
                             bool bottomOrigin = false);
    std::map<std::string, std::unique_ptr<ColladaScene>> models_;
    void drawHob(const HiddenObjectRound& round);
    void drawCarriage();
    std::string carriageButtonAt(float x, float y);
    std::string carriagePressedButton_;
    std::unique_ptr<LocationHintMenu> locationHintMenu_;
    std::string locationHintPressedButton_;
    bool hintBottlePressed_ = false;
    bool hintBottleInput(const SDL_Event& event);
    std::vector<GuiElement> skipHud_;
    bool skipPressed_ = false;
    std::string confirmationPressed_;
    bool puzzleSkipInput(const SDL_Event& event);
    bool confirmationInput(const SDL_Event& event);
    void drawPuzzleSkip();
    void drawLocationHints();
    std::string locationHintButtonAt(float x, float y);
    void drawActivity(const std::vector<ActivityObject>& objects);
    void drawCutscene(Cutscene& movie);
    void drawWindowCleaning(WindowCleaning& window);
    void clickWindowCleaning(float x, float y);
    std::optional<size_t> windowSectionAt(float x, float y);
    void moveWindowSponge(int x, int y);
    bool windowRubInput_ = false;
    void drawMixing(SpellMixing& mixing);
    void clickMixing(float x, float y);
    unsigned mixAngle(float x, float y) const;
    void moveMixPestle(float x, float y);
    bool mixStirInput_ = false, mixMortarHeld_ = false;
    float mixGrabX_ = 0, mixGrabY_ = 0;
    std::string mixHeld_;
    void drawVoodoo(VoodooDoll& doll);
    void clickVoodoo(float x, float y);
    std::string voodooHeld_, voodooHint_;
    float voodooHintX_ = 0, voodooHintY_ = 0;
    void clickHexBones(float x, float y);
    std::optional<size_t> hexBoneAt(float x, float y);
    bool hitActivity(const ActivityObject& object, float x, float y);
    void clickPiano(float x, float y);
    std::optional<unsigned> pianoKeyAt(float x, float y);
    void clickDoveCote(float x, float y);
    const ActivityObject* doveTarget(float x, float y);
    void clickCellarDoors(float x, float y);
    const ActivityObject* cellarTarget(float x, float y);
    void clickRats(float x, float y);
    const ActivityObject* ratsTarget(float x, float y);
    void drawSpiritLock(SpiritLock& lock);
    const ActivityObject* spiritLockTarget(float x, float y);
    void clickSpiritLock(float x, float y);
    bool spiritLockGrip_ = false;
    int spiritLockSoundChannel_ = -1;
    Mix_Chunk* spiritLockSound_ = nullptr;
    void drawBlockedWell(BlockedWell& well);
    std::optional<size_t> wellLetterAt(float x, float y);
    void clickBlockedWell(float x, float y);
    void drawCemeteryGate(CemeteryGate& gate);
    void clickCemeteryGate(float x, float y);
    std::optional<size_t> cemeteryBlockAt(float x, float y);
    void drawSkullsObey(SkullsObey& skulls);
    const ActivityObject* skullsTarget(float x, float y);
    void clickSkullsObey(float x, float y);
    int skullsWheelChannel_ = -1;
    Mix_Chunk* skullsWheelSound_ = nullptr;
    void drawVoodooCircle(VoodooCircle& circle);
    void drawSoftwareCirclePatch(const CircleShape& shape, const Image& source);
    struct CirclePatchImage {
        std::unique_ptr<Image> image;
        std::vector<unsigned char> colors;
        std::vector<float> alpha;
    };
    std::map<std::string, CirclePatchImage> circlePatchImages_;
    const ActivityObject* circleTarget(float x, float y);
    void clickVoodooCircle(float x, float y);
    bool circleStrokeInput_ = false, circleSuppressMouseUp_ = false;
    int circleChalkChannel_ = -1;
    Mix_Chunk* circleChalkSound_ = nullptr;
    void drawApothecary(Apothecary& shelves);
    void clickApothecary(float x, float y);
    const ActivityObject* apothecaryTarget(float x, float y);
    void clickSail(float x, float y);
    std::optional<size_t> sailPieceAt(float x, float y);
    const ActivityObject* leechesTarget(float x, float y);
    void clickLeeches(float x, float y);
    void moveRevealBrush(int x, int y);
    bool revealPressed_ = false, revealPotPressed_ = false, suppressRevealMouseUp_ = false,
         revealWasInventoryOpen_ = false;
    bool hasDialog() const {
        return !dialogs_.empty() && runtime_.dialogsEnabled();
    }
    int activityAmbientChannel_ = -1;
    Mix_Chunk* activityAmbientSound_ = nullptr;
    std::string hintBubble_;
    std::string tutorialHint_;
    std::optional<TutorialId> tutorialId_;
    void clearTutorial() {
        tutorialHint_.clear();
        tutorialId_.reset();
    }
    void drawTeaching();
    float tutorialX_ = 512, tutorialY_ = 384;
    float hintBubbleX_ = 530, hintBubbleY_ = 670, lastInteractionX_ = 530, lastInteractionY_ = 670;
    void drawOuiji(OuijiBoard& board);
    void drawInventory();
    void drawSpellBook();
    void spellBookAction(const std::string& action);
    bool spellBookButtonEnabled(const GuiElement& control) const;
    const GuiElement* spellBookButtonAt(float x, float y);
    void toggleSpellBook();
    bool bookOpen() const {
        return spellBookOpen_ || plantBookOpen_;
    }
    bool prepareBook();
    void togglePlantBook();
    bool plantBookInput(const SDL_Event& event);
    std::string plantBookButtonAt(float x, float y);
    std::vector<GuiElement> plantBookControls();
    void plantBookAction(const std::string& action);
    void drawPlantBook();
    void updateInventory();
    void syncInventoryPanel();
    std::vector<std::string> inventoryPage() const;
    std::string inventoryItemAt(float x, float y);
    bool inventoryClick(float x, float y, bool activate = true);
    size_t inventoryCount() const;
    void loadHud();
    void drawHobHud(const HiddenObjectRound& round);
    bool hobListAt(float x, float y);
    void pressHobTarget(float x, float y);
    void drawHint();
    void showHint(const std::string& text);
    void drawGui(const std::vector<GuiElement>& elements);
    void drawTutorial(const std::string& text, float anchorX, float anchorY, float alpha = 1);
    TTF_Font* font(const std::string& path);
    bool hit(const std::string& texture, float x, float y, float w, float h, float mouseX, float mouseY);
    void drawText(const std::string& text, int x, int y, int width, SDL_Color color = {240, 235, 220, 255},
                  const std::string& fontPath = {}, float alpha = 1);
    void drawImage(const std::string& path, float x, float y, float w = 0, float h = 0, float alpha = 1);
    void click(float x, float y, bool right);
    void processEvents();
    int sound(const std::string& path, int loops = 0, AudioBus bus = AudioBus::Effects);
    void render();
    void drawCursor();
    void drawDialog();
    bool dialogInput(const std::string& action);
    void startDialog();
    SDL_FRect bounds(const Object& object, const Image& image) const;
    Assets& assets_;
    Runtime& runtime_;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    TTF_Font* font_ = nullptr;
    struct Font {
        std::string bytes;
        TTF_Font* face = nullptr;
        ~Font() {
            if (face)
                TTF_CloseFont(face);
        }
    };
    std::map<std::string, std::unique_ptr<Font>> fonts_;
    std::unique_ptr<SpellBook> spellBook_;
    std::unique_ptr<PlantBook> plantBook_;
    bool plantBookOpen_ = false;
    std::vector<GuiElement> plantBookControls_;
    std::string plantBookPressed_;
    const GuiElement* spellBookPressed_ = nullptr;
    bool spellBookOpen_ = false;
    std::vector<GuiElement> spellBookControls_;
    std::vector<GuiElement> cutsceneControls_;
    uint64_t currentCutsceneGeneration_ = 0;
    bool cutsceneWasActive_ = false;
    std::vector<GuiElement> leftHud_, rightHud_, inventoryBack_, inventorySlots_;
    GuiElement inventoryOpenZone_;
    bool inventoryPinned_ = false, inventoryOpen_ = false, draggingInventory_ = false;
    bool inventoryAllowed_ = true;
    bool activityPressed_ = false;
    bool doveRightPressed_ = false;
    std::string pressedInventory_;
    Uint64 inventoryPressedAt_ = 0;
    unsigned inventoryDragMs_ = 300;
    std::unique_ptr<InventoryFeedback> inventoryFeedback_;
    std::unique_ptr<InventoryPanel> inventoryPanel_;
    std::vector<std::string> inventoryTransitionItems_;
    uint64_t inventoryOverlayGeneration_ = 0;
    std::deque<Event> inventoryTutorials_;
    float inventoryPointerX_ = 0, inventoryPointerY_ = 0;
    std::string listBackground_, listFont_, strikeTexture_;
    float listX_ = 0, listY_ = 0, listStrideX_ = 0, listStrideY_ = 0, listBackX_ = 0, listBackY_ = 0;
    int listRows_ = 1;
    SDL_Color listColor_{255, 255, 255, 255}, listFoundColor_{128, 128, 128, 255};
    float strikeX_ = 0, strikeY_ = 0, strikeExtra_ = 0, strikeThickness_ = 1;
    std::string tooltipFont_;
    SDL_Color tooltipColor_{35, 3, 35, 255};
    float tooltipMarginX_ = 0, tooltipMarginTop_ = 0, tooltipMarginBottom_ = 0, tooltipScreenWidth_ = 1024,
          tooltipArrowMargin_ = 25;
    std::string hobInventoryHint_;
    float hobInventoryHintX_ = 0, hobInventoryHintY_ = 0;
    GuiElement hintBack_, hintText_;
    float hintBackLineX_ = 0, hintBackLineY_ = 0, hintTextLineX_ = 0, hintTextLineY_ = 0,
          hintUsefulRatio_ = 0;
    unsigned hintMaxLines_ = 0;
    double hintOpened_ = 0, hintMinimumSeconds_ = 0, hintHideSeconds_ = 0;
    std::string fontBytes_, selection_, hint_, currentScene_;
    std::map<std::string, std::unique_ptr<Image>> images_;
    struct Animation {
        ImageSequence sequence;
        double started;
    };
    std::map<std::string, Animation> animations_;
    double elapsed_ = 0, animationTime_ = 0;
    std::map<std::string, Mix_Chunk*> sounds_;
    std::map<std::string, std::string> inventoryImages_, inventoryTooltips_;
    std::deque<Dialog> dialogs_;
    std::map<std::string, std::vector<GuiElement>> dialogLayouts_;
    double dialogAnyClickSeconds_ = 2, talkingAnyClickSeconds_ = 3, dialogOpened_ = 0;
    std::map<std::string, std::pair<float, float>> cursorSpots_;
    int previousCursor_ = SDL_ENABLE;
    bool audio_ = false, quit_ = false;
    int inventoryOffset_ = 0;
};
} // namespace deadtime
