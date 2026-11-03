#pragma once
#include "apothecary.hpp"
#include "assets.hpp"
#include "blocked_well.hpp"
#include "carriage.hpp"
#include "cellar_doors.hpp"
#include "cemetery_gate.hpp"
#include "cutscene.hpp"
#include "dove_cote.hpp"
#include "hex_bones.hpp"
#include "hint_bottle.hpp"
#include "hob.hpp"
#include "hob_feedback.hpp"
#include "leeches.hpp"
#include "location_hints.hpp"
#include "ouiji.hpp"
#include "piano.hpp"
#include "rats.hpp"
#include "reveal_spirits.hpp"
#include "sail.hpp"
#include "scene_particles.hpp"
#include "skulls_obey.hpp"
#include "spellmixing.hpp"
#include "spirit_lock.hpp"
#include "tutorials.hpp"
#include "voodoo.hpp"
#include "voodoo_circle.hpp"
#include "window_cleaning.hpp"
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>
struct lua_State;

namespace deadtime {
struct StateSection {
    std::string name, source;
};
std::vector<StateSection> splitStates(const std::string& source);
struct Object {
    struct ParticleResource {
        std::string path;
        double presim = 0;
    };
    struct SceneResource {
        std::string path;
        bool looped = true;
        std::vector<ParticleResource> particles;
    };
    std::string id, script, texture, state, cursor;
    std::vector<SceneResource> sceneResources;
    std::vector<ParticleResource> particleResources;
    double treeTime = 0;
    bool treePlaying = true, treeLooped = true, treePlayOption = true, particlesPresimRequested = false;
    bool treeLoopOverridden = false;
    bool particleGenerating = true;
    double particlePresimPending = 0;
    float x = 0, y = 0, z = 0, width = 0, height = 0, alpha = 1;
    bool visible = true, interactive = true, enabled = true, initialized = false;
    std::optional<double> timer;
    std::vector<std::pair<std::string, int>> states;
    struct ScriptAnimation {
        AnimationSet animation;
        double seconds = 0;
    };
    std::optional<ScriptAnimation> scriptAnimation;
};
struct Scene {
    std::string id, path, firstVisitScript, enterScript, leaveScript;
    std::vector<std::string> music;
    std::string ambient;
    std::vector<Object*> objects;
    bool visited = false;
};
struct Event {
    std::string type;
    std::vector<std::string> args;
    std::string origin;
};
class Runtime {
  public:
    explicit Runtime(Assets& assets);
    ~Runtime();
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    void load();
    bool checkpointReady() const;
    std::string checkpoint() const;
    static std::unique_ptr<Runtime> fromCheckpoint(Assets& assets, const std::string& bytes);
    void enter(const std::string& scene, bool inset = false);
    void flushNavigation();
    void run(const std::string& source, const std::string& origin);
    void runFile(const std::string& path);
    void finishDialog(const std::string& after, const std::string& origin, const std::string& result);
    void transition(Object& object, const std::string& state);
    void callback(Object& object, const std::string& event, const std::string& argument = {});
    void tick(double seconds);
    void tickVisual(Object& object, double seconds);
    void forgetVisual(const Object& object) {
        particleWorld_.reset(object);
    }
    void openHob(const std::string& path, const std::string& round);
    bool pickHob(size_t item, size_t piece, const std::string& inventory = {});
    bool missHob(float x, float y);
    void hobToolHint(size_t item, size_t piece) {
        hobFeedback_.toolHint({item, piece});
    }
    const HobFeedback& hobFeedback() const {
        return hobFeedback_;
    }
    void closeHob();
    bool requestHobHint(const std::function<std::pair<float, float>(const HobPiece&)>& center);
    const Object* hobHint() const {
        return !activeHob_.empty() && !hobHint_.particleResources.empty() ? &hobHint_ : nullptr;
    }
    bool applyAnywhere(const std::string& item);
    Carriage* carriage() {
        return carriageActive_ ? carriage_.get() : nullptr;
    }
    void openCarriage();
    void closeCarriage() {
        carriageActive_ = false;
    }
    bool chooseCarriage(const std::string& button);
    SpiritLock* spiritLock() {
        return spiritLockActive_ ? spiritLock_.get() : nullptr;
    }
    bool applySpiritLockPiece(size_t slot, const std::string& item);
    void closeSpiritLock();
    BlockedWell* blockedWell() {
        return blockedWellActive_ ? blockedWell_.get() : nullptr;
    }
    void closeBlockedWell();
    Apothecary* apothecary() {
        return apothecaryActive_ ? apothecary_.get() : nullptr;
    }
    bool placeApothecaryItem(size_t slot, const std::string& item);
    void closeApothecary();
    Sail* sail() {
        return sailActive_ ? sail_.get() : nullptr;
    }
    bool applySailInventory(const std::string& item, float x, float y);
    void closeSail();
    Leeches* leeches() {
        return leechesActive_ ? leeches_.get() : nullptr;
    }
    bool catchLeech(size_t index, const std::string& item);
    bool throwLeech();
    void inspectLeeches(const std::string& target);
    void closeLeeches();
    CemeteryGate* cemeteryGate() {
        return cemeteryGateActive_ ? cemeteryGate_.get() : nullptr;
    }
    void closeCemeteryGate();
    SkullsObey* skullsObey() {
        return skullsObeyActive_ ? skullsObey_.get() : nullptr;
    }
    void closeSkullsObey();
    VoodooCircle* voodooCircle() {
        return voodooCircleActive_ ? voodooCircle_.get() : nullptr;
    }
    bool chooseCircleChalk();
    bool interactVoodooCircle(const std::string& target, const std::string& inventory = {});
    void closeVoodooCircle();
    RevealSpirits* revealSpirits() {
        return reveal_.get();
    }
    void closeRevealSpirits();
    HiddenObjectRound* hob();
    OuijiBoard* ouiji() {
        return ouijiActive_ ? ouiji_.get() : nullptr;
    }
    SpellMixing* mixing() {
        return findMixing(activeMixing_);
    }
    bool placeMixing(const std::string& item);
    bool mixIngredient(const std::string& item);
    bool finishMixing(const std::string& item = {});
    void closeMixing() {
        activeMixing_.clear();
    }
    void closeOuiji();
    VoodooDoll* voodoo() {
        return voodooActive_ ? voodoo_.get() : nullptr;
    }
    bool placeVoodoo(const std::string& item, bool centralTarget, float x, float y);
    bool applyVoodoo(const std::string& item);
    void closeVoodoo() {
        voodooActive_ = false;
    }
    HexBones* hexBones() {
        return hexBonesActive_ ? hexBones_.get() : nullptr;
    }
    void closeHexBones();
    Piano* piano() {
        return pianoActive_ ? piano_.get() : nullptr;
    }
    bool takePianoReward();
    void closePiano();
    DoveCote* doveCote() {
        return doveCoteActive_ ? doveCote_.get() : nullptr;
    }
    bool feedDoveCote();
    bool connectDoveRope(size_t roll);
    void closeDoveCote();
    CellarDoors* cellarDoors() {
        return cellarDoorsActive_ ? cellarDoors_.get() : nullptr;
    }
    bool applyCellarAsh();
    void closeCellarDoors();
    Rats* rats() {
        return ratsActive_ ? rats_.get() : nullptr;
    }
    void closeRats(bool returnToLocation = true);
    bool dialogsEnabled() const {
        return (!cellarDoorsActive_ || !cellarDoors_->holdingDialogs()) &&
               (!ratsActive_ || !rats_->holdingDialogs());
    }
    WindowCleaning* windowCleaning() {
        return windowCleaningActive_ ? windowCleaning_.get() : nullptr;
    }
    bool placeWindowMixture();
    bool wetWindowSponge();
    bool beginWindowCleaning(size_t section, const std::string& sponge, int x, int y);
    void closeWindowCleaning();
    void openCutscene(const std::string& name, const std::string& after = {}, bool skippable = true);
    bool skipCutscene();
    Cutscene* cutscene() {
        return cutsceneActive_ ? cutscene_.get() : nullptr;
    }
    uint64_t cutsceneGeneration() const {
        return cutsceneGeneration_;
    }
    int currentChapter() const {
        return currentChapter_;
    }
    const std::string& latestSpell() const {
        return latestSpell_;
    }
    Object* findObject(const std::string& id);
    Scene* findScene(const std::string& id);
    std::string string(const std::string& key) const;
    void openLocationHints();
    void closeLocationHints() {
        locationHintsOpen_ = false;
    }
    bool locationHintsOpen() const {
        return locationHintsOpen_;
    }
    std::vector<LocationHintChoice> locationHintChoices() const;
    bool takeLocationHint(const std::string& key);
    bool replayLocationHint();
    bool hasHintHistory() const {
        return !hintHistory_.empty();
    }
    const auto& scenes() const {
        return scenes_;
    }
    const auto& objects() const {
        return objects_;
    }
    const auto& inventory() const {
        return inventory_;
    }
    const auto& inventoryOrder() const {
        return inventoryOrder_;
    }
    const auto& errors() const {
        return errors_;
    }
    const auto& layers() const {
        return layers_;
    }
    std::vector<ParticleBatch> particleBatches(const Object& object) {
        return particleWorld_.draw(object);
    }
    size_t particleEmitterCount(const Object& object) const {
        return particleWorld_.emitterCount(object);
    }
    std::deque<Event> events;
    void report(const std::string& message);
    bool hasInventory(const std::string& name) const {
        return inventory_.contains(name);
    }
    bool wasFound(const std::string& name) const {
        return found_.contains(name);
    }
    HintBottle& hintBottle() {
        return hintBottle_;
    }
    bool flag(const std::string& name) const {
        return flags_.contains(name);
    }
    bool requestTutorial(TutorialId id, std::optional<std::pair<float, float>> anchor = {});
    void inventoryFeedbackFinished();
    bool tutorialSeen(TutorialId id) const {
        return flag("tutorial:" + std::string(tutorialSpec(id).key));
    }
    std::string skippablePuzzle() const;
    double puzzleSkipProgress() const;
    bool puzzleSkipVisible() const;
    void setSkipBookOpen(bool open) {
        skipBookOpen_ = open;
    }
    void requestPuzzleSkip();
    bool skipMinigame();

  private:
    void tickPuzzleSkip(double seconds);
    bool hasSkipPuzzle(const std::string& id) const;
    void loadPuzzleSkip();
    std::map<std::string, double> puzzleSkipProgress_;
    double puzzleSkipSeconds_ = 95;
    bool skipBookOpen_ = false;
    void requestSailTutorial();
    double hobTutorialSeconds_ = 0;
    bool hobTutorialArmed_ = false;
    void finishHobPick(size_t item, size_t piece, const std::string& inventory);
    void releaseHob(bool keepFeedback);
    size_t activeActivities() const;
    static int dispatch(lua_State* state);
    int api(const std::string& name);
    int compile(const std::string& source, const std::string& origin,
                const std::string* dialogResult = nullptr);
    void check(int code, const std::string& origin);
    void bind(const std::string& name);
    void grantInventory(const std::string& item, bool effect = true);
    void consumeInventory(const std::string& item);
    void readScene(const std::string& path, const std::string& zone);
    void compileObject(Object& object);
    Assets& assets_;
    lua_State* lua_ = nullptr;
    SceneParticles particleWorld_;
    std::unique_ptr<Carriage> carriage_;
    bool carriageActive_ = false;
    std::map<std::string, Scene> scenes_;
    std::map<std::string, std::unique_ptr<Object>> objects_;
    std::map<std::string, std::string> strings_;
    HintBottle hintBottle_;
    Object hobHint_;
    double hobHintSeconds_ = 0;
    ParticleRandom hobHintRandom_;
    HobFeedback hobFeedback_;
    std::unique_ptr<SpiritLock> spiritLock_;
    bool spiritLockActive_ = false;
    std::unique_ptr<BlockedWell> blockedWell_;
    bool blockedWellActive_ = false;
    BlockedWell& ensureBlockedWell();
    std::unique_ptr<Apothecary> apothecary_;
    bool apothecaryActive_ = false;
    void processApothecaryEvents(Apothecary& shelves);
    std::unique_ptr<Sail> sail_;
    bool sailActive_ = false;
    std::unique_ptr<Leeches> leeches_;
    bool leechesActive_ = false;
    void processLeechesEvents();
    std::unique_ptr<CemeteryGate> cemeteryGate_;
    bool cemeteryGateActive_ = false;
    void processCemeteryGateEvents();
    std::unique_ptr<SkullsObey> skullsObey_;
    bool skullsObeyActive_ = false;
    void processSkullsObeyEvents();
    std::unique_ptr<VoodooCircle> voodooCircle_;
    bool voodooCircleActive_ = false;
    void processVoodooCircleEvents();
    std::unique_ptr<OuijiBoard> ouiji_;
    bool ouijiActive_ = false;
    std::map<std::string, std::unique_ptr<SpellMixing>> mixingGames_;
    std::string activeMixing_;
    SpellMixing* findMixing(const std::string& name);
    SpellMixing& ensureMixing(const std::string& name);
    std::unique_ptr<WindowCleaning> windowCleaning_;
    bool windowCleaningActive_ = false;
    std::unique_ptr<VoodooDoll> voodoo_;
    bool voodooActive_ = false;
    std::unique_ptr<HexBones> hexBones_;
    bool hexBonesActive_ = false;
    std::unique_ptr<Piano> piano_;
    bool pianoActive_ = false;
    std::unique_ptr<DoveCote> doveCote_;
    bool doveCoteActive_ = false;
    std::unique_ptr<CellarDoors> cellarDoors_;
    bool cellarDoorsActive_ = false;
    std::unique_ptr<Rats> rats_;
    bool ratsActive_ = false;
    void finishVoodoo();
    std::unique_ptr<Cutscene> cutscene_;
    bool cutsceneActive_ = false;
    uint64_t cutsceneGeneration_ = 0;
    int currentChapter_ = 0;
    std::string latestSpell_;
    void finishCutscene();
    std::set<std::string> inventory_, found_, flags_, enabledHints_;
    std::vector<std::string> hintHistory_;
    size_t hintReplay_ = 0;
    bool locationHintsOpen_ = false;
    std::vector<std::string> layers_, errors_, inventoryOrder_;
    std::vector<std::pair<std::string, bool>> navigation_;
    std::vector<std::string> scriptStack_;
    std::map<std::string, std::unique_ptr<HiddenObjectRound>> hobRounds_;
    std::map<std::pair<std::string, std::string>, std::string> anywhereApplications_;
    std::unique_ptr<RevealSpirits> reveal_;
    bool revealFinished_ = false, inRevealTick_ = false, revealExitRequested_ = false;
    std::vector<std::pair<Object*, double>> spiritFades_;
    std::string activeHob_;
    int transitionDepth_ = 0;
};
} // namespace deadtime
