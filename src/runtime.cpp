#include "runtime.hpp"
#include <pugixml.hpp>
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>

namespace deadtime {
namespace {
const std::map<std::string, std::string> mixingPaths{
    {"BanishSpirits", "minigames/_spellmixing_banish_spirits"},
    {"GraveyardGrease", "minigames/_spellmixing_graveyard_grease"},
    {"SpiritRevealingMixture", "minigames/_spellmixing_reveal_spirits"},
    {"AconiteAntidote", "minigames/_spellmixing_aconite_antidote"}};
std::string trim(std::string value) {
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return {};
    return value.substr(start, value.find_last_not_of(" \t\r\n") - start + 1);
}
std::string luaText(lua_State* l, int argument) {
    if (!lua_isstring(l, argument))
        throw std::runtime_error("Expected string at argument " + std::to_string(argument));
    size_t size = 0;
    const char* text = lua_tolstring(l, argument, &size);
    return {text, size};
}
double luaNumber(lua_State* l, int argument) {
    if (!lua_isnumber(l, argument))
        throw std::runtime_error("Expected number at argument " + std::to_string(argument));
    const double n = lua_tonumber(l, argument);
    if (!std::isfinite(n))
        throw std::runtime_error("Non-finite script number");
    return n;
}
std::string stem(const std::string& path) {
    const auto name = path.substr(path.find_last_of('/') + 1);
    return name.substr(0, name.find_last_of('.'));
}
std::string source(pugi::xml_node parent, const char* child, const std::string& base) {
    auto attr = parent.child(child).attribute("src");
    return attr ? resolve(base, attr.value()) : std::string{};
}
std::string hook(pugi::xml_node root, const char* name, const std::string& base) {
    const auto path = root.child(name).attribute("script").as_string();
    return *path ? resolve(base, path) + ".lua" : std::string{};
}
void sampleScriptAnimation(Object& node) {
    auto& playback = *node.scriptAnimation;
    // Copy current values so an alpha-only animation does not undo unrelated
    // Lua changes to visibility, texture or position on subsequent frames.
    ActivityObject visual;
    visual.texture = node.texture;
    visual.x = node.x;
    visual.y = node.y;
    visual.z = node.z;
    visual.width = node.width;
    visual.height = node.height;
    visual.alpha = node.alpha;
    visual.visible = node.visible;
    playback.animation.apply(visual, playback.seconds);
    node.x = visual.x;
    node.y = visual.y;
    node.z = visual.z;
    node.alpha = visual.alpha;
    node.visible = visual.visible;
    node.texture = visual.texture;
    node.width = visual.width;
    node.height = visual.height;
}
} // namespace

std::vector<StateSection> splitStates(const std::string& source) {
    std::string text = source;
    if (text.starts_with("\xEF\xBB\xBF"))
        text.erase(0, 3);
    // Some shipped Lua files use bare CR line endings. Treat CR, LF and CRLF
    // as one logical newline before recognizing state-section comments.
    std::string lines;
    lines.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r') {
            lines += '\n';
            if (i + 1 < text.size() && text[i + 1] == '\n')
                ++i;
        } else
            lines += text[i];
    }
    text = std::move(lines);
    std::vector<StateSection> result;
    std::string prefix;
    size_t current = 0;
    std::istringstream input(text);
    for (std::string line; std::getline(input, line);) {
        const auto cleaned = trim(line);
        bool marker = cleaned.starts_with("---");
        auto name = marker ? trim(cleaned.substr(3)) : std::string{};
        marker =
            marker && !name.empty() && std::all_of(name.begin(), name.end(), [](unsigned char c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
            });
        if (marker) {
            const auto existing =
                std::find_if(result.begin(), result.end(), [&](auto& s) { return s.name == name; });
            // The native loader overwrites a repeated state's body while retaining
            // the first declared state's name as the initial state.
            if (existing == result.end()) {
                current = result.size();
                result.push_back({name, prefix});
            } else {
                current = static_cast<size_t>(existing - result.begin());
                existing->source = prefix;
            }
        } else if (result.empty())
            prefix += line + '\n';
        else
            result[current].source += line + '\n';
    }
    if (result.empty())
        result.push_back({"default", prefix});
    return result;
}

Runtime::Runtime(Assets& assets)
    : assets_(assets), lua_(luaL_newstate()), particleWorld_(assets, std::random_device{}()),
      hobHintRandom_(std::random_device{}()), hobFeedback_(assets) {
    if (!lua_)
        throw std::runtime_error("Cannot create Lua state");
    // Game scripts need math, but have no filesystem, network, OS or module loader.
    luaopen_math(lua_);
    lua_settop(lua_, 0);
    for (const char* name : {"gotoState",
                             "isInState",
                             "isObjectFsmInitialized",
                             "setVisible",
                             "setInteractive",
                             "setEnabled",
                             "setTexture",
                             "setCursor",
                             "setDefaultCursor",
                             "getAlpha",
                             "setAlpha",
                             "setTimer",
                             "playAnimation",
                             "playTreeAnimation",
                             "stopTreeAnimation",
                             "rewindTreeAnimation",
                             "requestParticlesPresim",
                             "setParticleEmittersEnabled",
                             "addInventoryObject",
                             "removeInventoryObject",
                             "reAddInventoryObject",
                             "hasInventoryObject",
                             "wasInventoryObjectFound",
                             "gotoLocation",
                             "gotoInSet",
                             "gotoCarriage",
                             "getString",
                             "showHint",
                             "showMessageDialog",
                             "showMessageDialogWithImage",
                             "showMessageDialogWithSound",
                             "showTalkingFaceDialog",
                             "showConfirmationDialog",
                             "showDialog",
                             "playSound",
                             "playEnvSound",
                             "enableLocationHint",
                             "disableLocationHint",
                             "isLocationHintUsed",
                             "enableSpellBook",
                             "isSpellBookEnabled",
                             "enablePlantBook",
                             "isPlantBookEnabled",
                             "enableSpell",
                             "isSpellEnabled",
                             "enablePlantBookPage",
                             "enableJournalPage",
                             "allowZoneAccess",
                             "disallowZoneAccess",
                             "startTutorials",
                             "showTutorialHint",
                             "setCurrentChapter",
                             "stripmineSnakeBitten",
                             "stripmineGoodItemAction",
                             "stripmineWrongItemAction",
                             "log",
                             "logWarning",
                             "logError",
                             "addAnywhereAppliable",
                             "removeAnywhereAppliable",
                             "rechargeHintBottle",
                             "skipMinigame",
                             "endGame",
                             "gotoHob",
                             "exitMinigame",
                             "gotoMinigameOuijiBoard",
                             "isMinigameFinishedOuijiBoard",
                             "gotoMinigameBanishSpirits",
                             "gotoCutscene",
                             "isIngredientUsedBanishSpirits",
                             "isMixedBanishSpirits",
                             "wasMinigameEnteredBanishSpirits",
                             "wasMinigameEnteredOuijiBoard",
                             "gotoMinigameWindowCleaning",
                             "wasMinigameEnteredWindowCleaning",
                             "gotoMinigameVoodooDoll",
                             "wasMinigameEnteredVoodooDoll",
                             "isIngredientUsedVoodooDoll",
                             "gotoMinigameHexBones",
                             "wasMinigameEnteredHexBones",
                             "gotoMinigamePiano",
                             "wasMinigameEnteredPiano",
                             "isMinigameFinishedWithoutReward_Piano",
                             "gotoMinigameDoveCote",
                             "wasMinigameEnteredDoveCote",
                             "gotoMinigameCellarDoors",
                             "wasMinigameEnteredCellarDoors",
                             "gotoMinigameRats",
                             "wasMinigameEnteredRats",
                             "gotoRevealSpiritsMode",
                             "revealSpirits_finish",
                             "gotoMinigameSpiritLock",
                             "wasMinigameEnteredSpiritLock",
                             "gotoMinigameBlockedWell",
                             "wasMinigameEnteredBlockedWell",
                             "blockedWell_isNameKnown",
                             "blockedWell_addKnownName",
                             "blockedWell_isNameSpelled",
                             "blockedWell_getNumSpelledNames",
                             "gotoMinigameSortApothecaryItems",
                             "wasMinigameEnteredSortApothecaryItems",
                             "isMinigameFinishedWithoutReward_SortApothecaryItems",
                             "gotoMinigameSail",
                             "wasMinigameEnteredSail",
                             "gotoMinigameLeeches",
                             "wasMinigameEnteredLeeches",
                             "isMinigameFinishedWithoutReward_Leeches",
                             "gotoMinigameCemeteryGate",
                             "wasMinigameEnteredCemeteryGate",
                             "gotoMinigameVoodooCircle",
                             "wasMinigameEnteredVoodooCircle",
                             "gotoMinigameSkullsObey",
                             "wasMinigameEnteredSkullsObey",
                             "isMinigameStartedHexBones",
                             "isLoaFullSpiritRevealingMixture",
                             "isMinigameFinishedWithoutReward_OuijiBoard",
                             "voodooCircle_isCircleCleared",
                             "voodooCircle_isCircleDrawn",
                             "sail_isFabricFit",
                             "voodooDoll_areIngredientsReady"})
        bind(name);
    for (const auto* suffix : {"BanishSpirits",
                               "GraveyardGrease",
                               "SpiritRevealingMixture",
                               "AconiteAntidote",
                               "VoodooDoll",
                               "WindowCleaning",
                               "HexBones",
                               "Piano",
                               "DoveCote",
                               "Rats",
                               "CellarDoors",
                               "SortApothecaryItems",
                               "SpiritLock",
                               "BlockedWell",
                               "Sail",
                               "Leeches",
                               "CemeteryGate",
                               "VoodooCircle",
                               "SkullsObey",
                               "RevealSpirits"})
        bind(std::string("isMinigameFinished") + suffix);
    for (const auto& [suffix, _] : mixingPaths)
        for (const auto* prefix : {"gotoMinigame", "wasMinigameEntered", "isIngredientUsed", "isMixed"})
            bind(std::string(prefix) + suffix);
}
Runtime::~Runtime() {
    if (lua_)
        lua_close(lua_);
}
void Runtime::bind(const std::string& name) {
    lua_pushlightuserdata(lua_, this);
    lua_pushlstring(lua_, name.data(), name.size());
    lua_pushcclosure(lua_, dispatch, 2);
    lua_setglobal(lua_, name.c_str());
}
int Runtime::dispatch(lua_State* l) {
    // Throw within C++, then raise a Lua error only after C++ locals are destroyed.
    try {
        auto* runtime = static_cast<Runtime*>(lua_touserdata(l, lua_upvalueindex(1)));
        return runtime->api(lua_tostring(l, lua_upvalueindex(2)));
    } catch (const std::exception& e) {
        lua_pushstring(l, e.what());
    }
    return lua_error(l);
}
void Runtime::check(int code, const std::string& origin) {
    if (code == 0)
        return;
    const char* error = lua_tostring(lua_, -1);
    const std::string message = origin + ": " + (error ? error : "Lua error");
    lua_pop(lua_, 1);
    throw std::runtime_error(message);
}
int Runtime::compile(const std::string& text, const std::string& origin, const std::string* dialogResult) {
    if (!text.empty() && text[0] == '\x1B')
        throw std::runtime_error("Bytecode not accepted: " + origin);
    const int top = lua_gettop(lua_);
    try {
        check(luaL_loadbuffer(lua_, text.data(), text.size(), origin.c_str()), origin);
        lua_newtable(lua_);
        if (dialogResult) {
            lua_pushlstring(lua_, dialogResult->data(), dialogResult->size());
            lua_setfield(lua_, -2, "dialogResult");
        }
        lua_newtable(lua_);
        lua_pushvalue(lua_, LUA_GLOBALSINDEX);
        lua_setfield(lua_, -2, "__index");
        lua_setmetatable(lua_, -2);
        lua_pushvalue(lua_, -1);
        const int ref = luaL_ref(lua_, LUA_REGISTRYINDEX);
        lua_setfenv(lua_, -2);
        scriptStack_.push_back(origin);
        const int code = lua_pcall(lua_, 0, 0, 0);
        scriptStack_.pop_back();
        if (code) {
            luaL_unref(lua_, LUA_REGISTRYINDEX, ref);
            check(code, origin);
        }
        return ref;
    } catch (...) {
        lua_settop(lua_, top);
        throw;
    }
}
void Runtime::run(const std::string& text, const std::string& origin) {
    const int ref = compile(text, origin);
    luaL_unref(lua_, LUA_REGISTRYINDEX, ref);
}
void Runtime::runFile(const std::string& path) {
    if (!path.empty())
        run(assets_.read(path), path);
}
void Runtime::finishDialog(const std::string& after, const std::string& origin, const std::string& result) {
    if (origin.starts_with("hob_reward:")) {
        if (hob() && origin == "hob_reward:" + activeHob_ && hobFeedback_.confirmReward()) {
            auto* round = hob();
            round->rewarded = true;
            if (!round->completeScript.empty()) {
                const int ref = compile(assets_.read(round->completeScript), round->completeScript, &result);
                luaL_unref(lua_, LUA_REGISTRYINDEX, ref);
            }
        }
        flushNavigation();
        return;
    }
    // Native dialog close supplies this variable in the callback environment;
    // it must not overwrite a global visible to unrelated object scripts.
    if (!after.empty()) {
        const int ref = compile(after, origin, &result);
        luaL_unref(lua_, LUA_REGISTRYINDEX, ref);
    }
    flushNavigation();
}
void Runtime::compileObject(Object& object) {
    if (object.script.empty())
        return;
    for (const auto& section : splitStates(assets_.read(object.script)))
        object.states.emplace_back(section.name, compile(section.source, object.script));
}
void Runtime::load() {
    hintBottle_.load(assets_);
    loadPuzzleSkip();
    if (!scenes_.empty())
        throw std::runtime_error("World already loaded");
    if (assets_.contains("strings.xml")) {
        pugi::xml_document doc;
        const auto data = assets_.read("strings.xml");
        if (!doc.load_buffer(data.data(), data.size()))
            throw std::runtime_error("Invalid strings.xml");
        std::function<void(pugi::xml_node, std::string)> visit = [&](auto node, auto prefix) {
            for (auto child : node.children()) {
                std::string path = prefix.empty() ? child.name() : prefix + '/' + child.name();
                if (child.attribute("text"))
                    strings_[path] = child.attribute("text").value();
                visit(child, path);
            }
        };
        visit(doc.child("strings"), "");
    }
    for (const auto& path : assets_.files(".loc")) {
        if (!path.starts_with("zones/"))
            continue;
        const auto end = path.find('/', 6);
        if (end == std::string::npos)
            continue;
        readScene(path, path.substr(6, end - 6));
    }
    // Resolve references only after every object exists.
    for (auto& [_, object] : objects_)
        compileObject(*object);
}
void Runtime::readScene(const std::string& path, const std::string& zone) {
    const auto data = assets_.read(path);
    pugi::xml_document doc;
    const auto parsed = doc.load_buffer(data.data(), data.size());
    if (!parsed)
        throw std::runtime_error(path + ": " + parsed.description());
    const auto root = doc.child("location");
    if (!root)
        throw std::runtime_error("Expected location: " + path);
    Scene scene;
    scene.id = zone + '.' + stem(path);
    scene.path = path;
    const auto base = directory(path);
    scene.firstVisitScript = hook(root, "onFirstVisit", base);
    scene.enterScript = hook(root, "onEnter", base);
    scene.leaveScript = hook(root, "onExit", base);
    if (scene.leaveScript.empty())
        scene.leaveScript = hook(root, "onLeave", base);
    for (auto track : root.child("music").children("track"))
        scene.music.push_back(resolve(base, track.attribute("src").value()));
    scene.ambient = source(root, "ambient", base);
    for (auto node : root.child("objects").children("object")) {
        auto object = std::make_unique<Object>();
        object->id = scene.id + '.' + node.attribute("name").value();
        object->texture = source(node, "texture", base);
        for (auto child : node.children("scene")) {
            // The native scene loader reads animation_looped here. Several
            // shipped layouts say looped instead; that spelling is ignored by
            // the original loader too. Lua playTreeAnimation can override it.
            Object::SceneResource resource{resolve(base, child.attribute("src").value()),
                                           child.attribute("animation_looped").as_bool(true),
                                           {}};
            if (auto particles = child.attribute("particles"))
                resource.particles.push_back({resolve(base, particles.value()), 0});
            for (auto particles : child.children("particles"))
                resource.particles.push_back({resolve(base, particles.attribute("src").value()),
                                              particles.attribute("presim").as_double()});
            object->sceneResources.push_back(std::move(resource));
        }
        for (auto child : node.children("particles"))
            object->particleResources.push_back(
                {resolve(base, child.attribute("src").value()), child.attribute("presim").as_double()});
        object->script = source(node, "script", base);
        if (!object->script.empty())
            object->script += ".lua";
        const auto pos = node.child("position");
        object->x = pos.attribute("x").as_float();
        object->y = pos.attribute("y").as_float();
        object->z = pos.attribute("z").as_float();
        const auto size = node.child("size");
        object->width = size.attribute("width").as_float();
        object->height = size.attribute("height").as_float();
        object->visible = node.child("visible").attribute("value").as_bool(true);
        object->interactive = node.child("interactive").attribute("value").as_bool(true);
        object->alpha = node.child("alpha").attribute("value").as_float(1);
        if (objects_.contains(object->id))
            throw std::runtime_error("Duplicate object: " + object->id);
        scene.objects.push_back(object.get());
        objects_[object->id] = std::move(object);
    }
    if (scenes_.contains(scene.id))
        throw std::runtime_error("Duplicate scene: " + scene.id);
    scenes_[scene.id] = std::move(scene);
}
Object* Runtime::findObject(const std::string& id) {
    auto it = objects_.find(id);
    return it == objects_.end() ? nullptr : it->second.get();
}
Scene* Runtime::findScene(const std::string& id) {
    auto it = scenes_.find(id);
    return it == scenes_.end() ? nullptr : &it->second;
}
void Runtime::report(const std::string& message) {
    if (std::find(errors_.begin(), errors_.end(), message) == errors_.end()) {
        errors_.push_back(message);
        std::cerr << "runtime: " << message << '\n';
    }
}
void Runtime::callback(Object& object, const std::string& event, const std::string& argument) {
    auto state = std::find_if(object.states.begin(), object.states.end(),
                              [&](auto& s) { return s.first == object.state; });
    if (state == object.states.end())
        return;
    const int top = lua_gettop(lua_);
    lua_rawgeti(lua_, LUA_REGISTRYINDEX, state->second);
    lua_getfield(lua_, -1, event.c_str());
    lua_remove(lua_, -2);
    if (!lua_isfunction(lua_, -1)) {
        lua_settop(lua_, top);
        return;
    }
    lua_pushlstring(lua_, object.id.data(), object.id.size());
    int count = 1;
    if (!argument.empty()) {
        lua_pushlstring(lua_, argument.data(), argument.size());
        count = 2;
    }
    scriptStack_.push_back(object.script);
    const int result = lua_pcall(lua_, count, 0, 0);
    scriptStack_.pop_back();
    check(result, object.id + '.' + event);
}
void Runtime::transition(Object& object, const std::string& state) {
    if (std::none_of(object.states.begin(), object.states.end(), [&](auto& s) { return s.first == state; }))
        throw std::runtime_error("Unknown state " + object.id + ':' + state);
    if (transitionDepth_ >= 64)
        throw std::runtime_error("Recursive state transition: " + object.id);
    ++transitionDepth_;
    try {
        if (object.initialized)
            callback(object, "onStateLeave");
        object.state = state;
        object.initialized = true;
        callback(object, "onStateEnter");
    } catch (...) {
        --transitionDepth_;
        throw;
    }
    --transitionDepth_;
}
void Runtime::enter(const std::string& id, bool inset) {
    auto* scene = findScene(id);
    if (!scene)
        throw std::runtime_error("Unknown scene: " + id);
    closeCarriage();
    closeLocationHints();
    if (reveal_)
        closeRevealSpirits();
    if (!inset && windowCleaning())
        closeWindowCleaning();
    if (!inset && hexBones())
        closeHexBones();
    if (!inset && piano())
        closePiano();
    if (!inset && doveCote())
        closeDoveCote();
    if (!inset && cellarDoors())
        closeCellarDoors();
    if (!inset && rats())
        closeRats(false);
    if (!inset) {
        if (!layers_.empty())
            runFile(findScene(layers_.back())->leaveScript);
        layers_.clear();
    } else if (std::find(layers_.begin(), layers_.end(), id) != layers_.end())
        return;
    layers_.push_back(id);
    for (auto* object : scene->objects) {
        if (!object->initialized && !object->states.empty()) {
            try {
                transition(*object, object->states.front().first);
            } catch (const std::exception& e) {
                report(e.what());
            }
        }
    }
    if (!scene->visited) {
        scene->visited = true;
        try {
            runFile(scene->firstVisitScript);
        } catch (const std::exception& e) {
            report(e.what());
        }
    }
    try {
        runFile(scene->enterScript);
    } catch (const std::exception& e) {
        report(e.what());
    }
    // Native HintRegistry suppresses this introduction once any hint was bought.
    if (!inset && id.ends_with(".front_gate") && hintHistory_.empty())
        requestTutorial(TutorialId::LocationHint);
}
void Runtime::flushNavigation() {
    int guard = 0;
    while (!navigation_.empty()) {
        if (++guard > 32)
            throw std::runtime_error("Recursive navigation");
        auto request = navigation_.front();
        navigation_.erase(navigation_.begin());
        if (request.first.empty() && request.second) {
            if (layers_.size() > 1) {
                runFile(findScene(layers_.back())->leaveScript);
                layers_.pop_back();
            }
        } else
            enter(request.first, request.second);
    }
}
void Runtime::tickVisual(Object& object, double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid visual tick duration");
    if (!object.enabled)
        return;
    if (object.scriptAnimation) {
        auto& playback = *object.scriptAnimation;
        playback.seconds += seconds;
        sampleScriptAnimation(object);
        if (!playback.animation.looped() && playback.seconds >= playback.animation.duration())
            object.scriptAnimation.reset();
    }
    if (object.treePlaying)
        object.treeTime += seconds;
    particleWorld_.tick(object, seconds);
}
void Runtime::tick(double seconds) {
    if (!std::isfinite(seconds) || seconds < 0)
        throw std::runtime_error("Invalid tick duration");
    hintBottle_.tick(seconds);
    tickPuzzleSkip(seconds);
    const bool hobWasActive = hob() != nullptr;
    const bool hobWasBusy = hobFeedback_.busy();
    if (hobWasActive || hobFeedback_.returning()) {
        const bool penalty = hobFeedback_.penalized();
        const bool fadeOut = hobFeedback_.fadingOut();
        if (auto found = hobFeedback_.tick(seconds))
            finishHobPick(found->first, found->second, {});
        if (fadeOut && hobFeedback_.returning())
            releaseHob(true);
        if (penalty && !hobFeedback_.penalized()) {
            hobTutorialArmed_ = true;
            hobTutorialSeconds_ = 0;
        }
    }
    if (hob() && !hobWasBusy && hobTutorialArmed_) {
        hobTutorialSeconds_ += seconds;
        if (hobTutorialSeconds_ > 10) {
            hobTutorialArmed_ = false;
            hobTutorialSeconds_ = 0;
            if (requestTutorial(TutorialId::HobIdle))
                hintBottle_.refill();
        }
    }
    if (hob() && !hobHint_.particleResources.empty()) {
        // Stop births at the configured deadline; let existing particles expire.
        const double generating = std::min(seconds, hobHintSeconds_);
        if (generating > 0)
            particleWorld_.tick(hobHint_, generating);
        hobHintSeconds_ -= generating;
        if (hobHintSeconds_ <= 0) {
            hobHint_.particleGenerating = false;
            particleWorld_.setGenerating(hobHint_, false);
        }
        if (seconds > generating)
            particleWorld_.tick(hobHint_, seconds - generating);
    }
    if (auto* movie = cutscene()) {
        movie->tick(seconds);
        for (const auto& sound : movie->takeSounds())
            events.push_back({"cutsceneSound", {sound.path, sound.music ? "music" : "voice"}, "cutscene"});
        if (movie->complete())
            finishCutscene();
        flushNavigation();
        return;
    }
    // The native HOB state replaces Location, whose frame handler owns
    // the location's timers and animation updates.
    if (hobWasActive) {
        flushNavigation();
        return;
    }
    if (auto* lock = spiritLock()) {
        lock->tick(seconds);
        for (auto& [id, effect] : lock->effects())
            if (effect->enabled)
                particleWorld_.tick(*effect, seconds);
        for (const auto& event : lock->takeEvents()) {
            if (event.kind == SpiritLockEvent::Sound)
                events.push_back({"playSound", {event.value}, "spirit_lock"});
            else if (event.kind == SpiritLockEvent::FinalSound)
                events.push_back({"spiritLockFinalSound", {event.value}, "spirit_lock"});
            else if (event.kind == SpiritLockEvent::Hint || event.kind == SpiritLockEvent::PopupHint) {
                events.push_back({"showHint", {string(event.value)}, "spirit_lock"});
                if (event.kind == SpiritLockEvent::PopupHint)
                    events.push_back({"playSound", {"sound/sfx/sfx_reactive_pop_up"}, "spirit_lock"});
            } else if (!lock->rewarded) {
                lock->rewarded = true;
                events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "spirit_lock"});
                runFile(lock->onFinish);
                closeSpiritLock();
            }
        }
        flushNavigation();
        return;
    }
    if (auto* shelves = apothecary()) {
        shelves->tick(seconds);
        for (auto& [id, effect] : shelves->effects())
            if (effect->enabled)
                particleWorld_.tick(*effect, seconds);
        processApothecaryEvents(*shelves);
        flushNavigation();
        return;
    }
    if (auto* worms = leeches()) {
        worms->tick(seconds);
        processLeechesEvents();
        flushNavigation();
        return;
    }
    if (auto* cloth = sail()) {
        cloth->tick(seconds);
        for (const auto& event : cloth->takeEvents()) {
            if (event.kind == SailEvent::Sound)
                events.push_back({"playSound", {event.value}, "sail"});
            else if (event.kind == SailEvent::Hint)
                events.push_back({"showHint", {string(event.value)}, "sail"});
            else if (event.kind == SailEvent::ClearHint)
                events.push_back({"clearHintBubble", {}, "sail"});
            else if (!cloth->rewarded) {
                cloth->rewarded = true;
                events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "sail"});
                runFile(cloth->onFinish);
                events.push_back({"showMessageDialog",
                                  {string("puzzles/sail/congratulations"), "exitMinigame()"},
                                  "sail"});
            }
        }
        flushNavigation();
        return;
    }
    if (auto* circle = voodooCircle()) {
        circle->setSpellKnown(flag("enableSpell:" + circle->spell()));
        processVoodooCircleEvents();
        double left = seconds;
        do {
            const double step = std::min(left, .1);
            for (auto& [id, effect] : circle->effects())
                if (effect->enabled) {
                    if (effect->treePlaying)
                        effect->treeTime += step;
                    particleWorld_.tick(*effect, step);
                    if (id == "fx_voodoo_circle_flash") {
                        bool alive = false;
                        for (const auto& batch : particleWorld_.draw(*effect))
                            alive |= !batch.sprites.empty();
                        if (alive)
                            effect->particleGenerating = false;
                        else if (!effect->particleGenerating)
                            effect->enabled = false;
                    }
                }
            circle->tick(step);
            processVoodooCircleEvents();
            left -= step;
        } while (voodooCircle() && left > 1e-10);
        flushNavigation();
        return;
    }
    if (auto* skulls = skullsObey()) {
        processSkullsObeyEvents();
        do {
            const double step = std::min(seconds, .1);
            if (skulls->eyes().enabled) {
                skulls->eyes().treeTime += step;
                particleWorld_.tick(skulls->eyes(), step);
            }
            skulls->tick(step);
            processSkullsObeyEvents();
            seconds -= step;
        } while (seconds > 1e-10);
        flushNavigation();
        return;
    }
    if (auto* gate = cemeteryGate()) {
        // Keep generator births on the rotating part of a long tick only.
        processCemeteryGateEvents();
        do {
            const double step = gate->busy() ? std::min(seconds, gate->rotationRemaining()) : seconds;
            particleWorld_.tick(gate->effect(), step);
            gate->tick(step);
            seconds -= step;
            processCemeteryGateEvents();
        } while (seconds > 1e-10);
        flushNavigation();
        return;
    }
    if (auto* well = blockedWell()) {
        well->tick(seconds);
        for (const auto& event : well->takeEvents()) {
            if (event.kind == WellEvent::Sound)
                events.push_back({"playSound", {event.value}, "blocked_well"});
            else if (event.kind == WellEvent::Hint)
                events.push_back({"showHint", {string(event.value)}, "blocked_well"});
            else if (!well->rewarded) {
                well->rewarded = true;
                events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "blocked_well"});
                runFile(well->onFinish);
            }
        }
        flushNavigation();
        return;
    }
    if (auto* window = windowCleaning()) {
        window->tick(seconds);
        for (const auto& event : window->takeEvents()) {
            if (event.kind == WindowCleaningEvent::Kind::Sound)
                events.push_back({"playSound", {event.value}, "window_cleaning"});
            else if (event.kind == WindowCleaningEvent::Kind::DrySponge) {
                consumeInventory("sponge_wet");
                grantInventory("sponge", false);
                events.push_back({"selectInventoryObject", {"sponge"}, "window_cleaning"});
            } else {
                const auto hint = !flag("enableSpellBook") &&
                                          (event.value == "try_harder" || event.value == "need_wet_sponge")
                                      ? "check_spellbook"
                                      : event.value;
                events.push_back(
                    {"showHint", {string("puzzles/window_cleaning/hints/" + hint)}, "window_cleaning"});
            }
        }
        if (window->complete() && !window->rewarded) {
            window->rewarded = true;
            events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "window_cleaning"});
            runFile(window->onFinish);
        }
        flushNavigation();
        return;
    }
    if (auto* rodents = rats()) {
        rodents->tick(seconds);
        for (const auto& event : rodents->takeEvents()) {
            if (event.kind == RatsEvent::Sound)
                events.push_back({"playSound", {event.value}, "rats"});
            else if (event.kind == RatsEvent::Hint)
                events.push_back({"showHintBubble", {string(event.value)}, "rats"});
            else if (event.kind == RatsEvent::ClearHint)
                events.push_back({"clearHintBubble", {}, "rats"});
            else if (!rodents->rewarded) {
                rodents->rewarded = true;
                events.push_back({"activityAmbient", {""}, "rats"});
                runFile(rodents->onFinish);
            }
        }
        flushNavigation();
        return;
    }
    if (auto* door = cellarDoors()) {
        door->tick(seconds);
        for (const auto& event : door->takeEvents()) {
            if (event.kind == CellarEvent::Sound)
                events.push_back({"playSound", {event.value}, "cellar_doors"});
            else if (event.kind == CellarEvent::Hint)
                events.push_back({"showHintBubble", {string(event.value)}, "cellar_doors"});
            else if (!door->rewarded) {
                door->rewarded = true;
                events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "cellar_doors"});
                runFile(door->onFinish);
            }
        }
        flushNavigation();
        return;
    }
    if (auto* cote = doveCote()) {
        cote->tick(seconds);
        for (const auto& event : cote->takeEvents()) {
            if (event.kind == DoveEvent::Sound)
                events.push_back({"playSound", {event.value}, "dove_cote"});
            else if (event.kind == DoveEvent::ReturnRope) {
                grantInventory("rope");
                events.push_back({"selectInventoryObject", {""}, "dove_cote"});
            } else
                events.push_back({event.kind == DoveEvent::Help ? "showHint" : "showHintBubble",
                                  {string(event.value)},
                                  "dove_cote"});
        }
        if (cote->complete() && !cote->rewarded) {
            cote->rewarded = true;
            consumeInventory("rope");
            consumeInventory("seeds");
            events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "dove_cote"});
            runFile(cote->onFinish);
        }
        flushNavigation();
        return;
    }
    if (auto* keys = piano()) {
        keys->tick(seconds);
        for (const auto& sound : keys->takeSounds())
            events.push_back({"playSound", {sound}, "piano"});
        flushNavigation();
        return;
    }
    if (auto* bones = hexBones()) {
        bones->tick(seconds);
        for (const auto& sound : bones->takeSounds())
            events.push_back({"playSound", {sound}, "hex_bones"});
        if (!bones->feedback.empty()) {
            events.push_back({"showHintBubble", {string(bones->feedback)}, "hex_bones"});
            bones->feedback.clear();
        }
        if (bones->complete() && !bones->rewarded) {
            bones->rewarded = true;
            events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "hex_bones"});
            runFile(bones->onFinish);
        }
        flushNavigation();
        return;
    }
    if (voodoo()) {
        flushNavigation();
        return;
    }
    if (auto* board = ouiji()) {
        board->tick(seconds);
        if (!board->feedback.empty()) {
            events.push_back({"playSound", {board->feedback}, "ouiji_board"});
            board->feedback.clear();
        }
        if (board->complete() && !board->rewarded) {
            board->rewarded = true;
            runFile(board->onFinish);
        }
        if (board->exitRequested)
            closeOuiji();
    }
    if (auto* mix = mixing()) {
        mix->tick(seconds);
        if (!mix->feedback.empty()) {
            events.push_back(
                {"showHint", {string("spellmixing/common/hints/" + mix->feedback)}, "spellmixing"});
            mix->feedback.clear();
        }
    }
    for (auto it = spiritFades_.begin(); it != spiritFades_.end();) {
        auto& object = *it->first;
        object.alpha = std::max(0.f, object.alpha - float(seconds * it->second));
        if (object.alpha == 0)
            it = spiritFades_.erase(it);
        else
            ++it;
    }
    if (reveal_) {
        auto dispatchEvents = [&] {
            for (const auto& event : reveal_->takeEvents())
                events.push_back({event.kind == RevealEvent::Brush ? "revealBrushSound" : "playSound",
                                  {event.value},
                                  "reveal_spirits"});
        };
        inRevealTick_ = true;
        try {
            reveal_->tick(seconds, [&](Object& spirit) {
                dispatchEvents(); // The section sound precedes the Lua callback.
                callback(spirit, "onSpiritRevealed");
            });
        } catch (...) {
            inRevealTick_ = false;
            throw;
        }
        inRevealTick_ = false;
        dispatchEvents();
        if (reveal_->complete() || revealExitRequested_)
            closeRevealSpirits();
    }
    std::vector<Object*> due;
    for (const auto& id : layers_)
        for (auto* object : findScene(id)->objects) {
            try {
                tickVisual(*object, seconds);
            } catch (const std::exception& e) {
                report(object->id + ": " + e.what());
            }
            if (object->enabled && object->timer) {
                *object->timer -= seconds;
                if (*object->timer <= 0) {
                    object->timer.reset();
                    due.push_back(object);
                }
            }
        }
    for (auto* object : due) {
        try {
            callback(*object, "onTimer");
        } catch (const std::exception& e) {
            report(e.what());
        }
    }
    flushNavigation();
}
std::string Runtime::string(const std::string& key) const {
    auto it = strings_.find(key);
    return it == strings_.end() ? key : it->second;
}
void Runtime::closeOuiji() {
    auto* board = ouiji();
    if (!board || board->busy())
        return;
    // Reward Lua must run even when the pickup immediately leaves the activity.
    if (board->complete() && !board->rewarded) {
        board->rewarded = true;
        runFile(board->onFinish);
    }
    board->leave();
    ouijiActive_ = false;
}
void Runtime::openLocationHints() {
    // Native Core::HintRequested (4639A0) also serves active minigames.
    // HOB has its own particle hint reaction; travel/movie overlays do not.
    if (layers_.empty() || carriage() || cutscene() || hob() || locationHintsOpen_)
        return;
    pugi::xml_document doc;
    const auto bytes = assets_.read("deadtime.game");
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid hint script configuration");
    const std::string script = doc.child("deadtime").child("hint_scripts").attribute("location").value();
    if (script.empty())
        throw std::runtime_error("Missing location hint script");
    runFile(normalize(script) + ".lua");
    locationHintsOpen_ = true;
    requestTutorial(TutorialId::HintSelection);
}
std::vector<LocationHintChoice> Runtime::locationHintChoices() const {
    std::vector<LocationHintChoice> result;
    for (const auto& key : enabledHints_) {
        const auto title = directory(key), group = directory(title);
        result.push_back({key, group, string(group), string(title)});
    }
    return result;
}
bool Runtime::takeLocationHint(const std::string& key) {
    if (!locationHintsOpen_ || !enabledHints_.contains(key) || !hintBottle_.consume())
        return false;
    enabledHints_.erase(key);
    flags_.insert("usedHint:" + key);
    hintHistory_.push_back(key);
    hintReplay_ = 0;
    events.push_back({"locationHintText", {string(key)}, "location_hints"});
    events.push_back({"playSound", {"sound/sfx/sfx_hint"}, "location_hints"});
    events.push_back({"playSound", {"sound/sfx/sfx_hint_active"}, "location_hints"});
    closeLocationHints();
    return true;
}
bool Runtime::replayLocationHint() {
    if (!locationHintsOpen_ || hintHistory_.empty())
        return false;
    // Native 538B10 cycles through previously read hints newest first, freely.
    hintReplay_ %= hintHistory_.size();
    events.push_back({"locationHintText",
                      {string(hintHistory_[hintHistory_.size() - 1 - hintReplay_])},
                      "location_hints"});
    hintReplay_ = (hintReplay_ + 1) % hintHistory_.size();
    closeLocationHints();
    return true;
}
HiddenObjectRound* Runtime::hob() {
    auto found = hobRounds_.find(activeHob_);
    return found == hobRounds_.end() ? nullptr : found->second.get();
}
void Runtime::openHob(const std::string& path, const std::string& round) {
    const auto key = normalize(path) + ':' + round;
    if (!hobRounds_.contains(key))
        hobRounds_[key] =
            std::make_unique<HiddenObjectRound>(assets_, path, round, std::random_device{}() & 32767u);
    activeHob_ = key;
    hobFeedback_.reset();
    particleWorld_.reset(hobHint_);
    hobHint_ = Object{};
    hobHintSeconds_ = 0;
    runFile(hob()->enterScript);
    hobTutorialSeconds_ = 0;
    hobTutorialArmed_ = true;
    requestTutorial(TutorialId::FirstHob);
    if (round == "round1")
        requestTutorial(TutorialId::PlantTool);
}
void Runtime::closeHob() {
    if (hobFeedback_.transient())
        return;
    releaseHob(false);
}
void Runtime::releaseHob(bool keepFeedback) {
    activeHob_.clear();
    particleWorld_.reset(hobHint_);
    if (!keepFeedback)
        hobFeedback_.reset();
    hobHint_ = Object{};
    hobHintSeconds_ = 0;
    hobTutorialSeconds_ = 0;
    hobTutorialArmed_ = false;
    // Native HOB exit returns through GotoInZoneLocation, which runs onEnter
    // once the location is back (at the start of its fade-in after a reward).
    if (!layers_.empty()) {
        try {
            runFile(findScene(layers_.back())->enterScript);
        } catch (const std::exception& e) {
            report(e.what());
        }
    }
}
bool Runtime::requestHobHint(const std::function<std::pair<float, float>(const HobPiece&)>& center) {
    auto* round = hob();
    if (!round || hobFeedback_.busy() || round->rewarded || round->complete() || !hintBottle_.available())
        return false;
    // Native Seek rerolls completed slots before choosing a remaining piece.
    size_t index;
    do {
        index = round->targets[hobHintRandom_.next() % round->targets.size()];
    } while (round->items[index].found());
    const auto& item = round->items[index];
    std::vector<size_t> remaining;
    for (size_t i = 0; i < item.pieces.size(); ++i)
        if (!item.pieces[i].found)
            remaining.push_back(i);
    const auto& piece = item.pieces[remaining[hobHintRandom_.next() % remaining.size()]];
    const auto [x, y] = center(piece);
    if (!std::isfinite(x) || !std::isfinite(y))
        throw std::runtime_error("Invalid HOB hint center");
    pugi::xml_document doc;
    const auto bytes = assets_.read("hobs/_config.xml");
    if (!doc.load_buffer(bytes.data(), bytes.size()))
        throw std::runtime_error("Invalid HOB hint configuration");
    const auto hint = doc.child("hobs").child("hint");
    const double duration = hint.attribute("time").as_double();
    if (!std::isfinite(duration) || duration <= 0 || !hint.attribute("src"))
        throw std::runtime_error("Invalid HOB hint definition");
    if (hobHint_.particleResources.empty()) {
        hobHint_.particleResources.push_back({resolve("hobs", hint.attribute("src").value()), 0});
        hobHint_.particleGenerating = false;
        try {
            particleWorld_.tick(hobHint_, 0);
        } // Validate before spending charge.
        catch (...) {
            particleWorld_.reset(hobHint_);
            hobHint_ = Object{};
            throw;
        }
    }
    hobHint_.x = x;
    hobHint_.y = y;
    hobHint_.z = -50;
    hobHint_.particleGenerating = true;
    particleWorld_.setGenerating(hobHint_, true);
    hobHintSeconds_ = duration;
    hintBottle_.consume();
    return true;
}
void Runtime::openCarriage() {
    if (layers_.empty())
        throw std::runtime_error("Carriage has no current location");
    if (!carriage_)
        carriage_ = std::make_unique<Carriage>(assets_);
    const auto& location = layers_.back();
    const auto zone = location.substr(0, location.find('.'));
    carriage_->enter(zone, [&](const auto& destination) { return flag("allowZoneAccess:" + destination); });
    carriageActive_ = true;
}
bool Runtime::chooseCarriage(const std::string& button) {
    if (!carriage())
        return false;
    if (button == "button_exit") {
        closeCarriage();
        return true;
    }
    const auto destination = carriage_->destination(button);
    if (!destination || !findScene(*destination))
        return false;
    closeCarriage();
    enter(*destination);
    flushNavigation();
    return true;
}
bool Runtime::applyAnywhere(const std::string& item) {
    if (carriage())
        return false;
    if (layers_.empty() || !hasInventory(item) || reveal_ || hob() || mixing() || ouiji() ||
        windowCleaning() || voodoo() || hexBones() || piano() || doveCote() || cellarDoors() || rats() ||
        spiritLock() || blockedWell() || apothecary() || sail() || leeches() || cemeteryGate() ||
        voodooCircle() || skullsObey() || cutscene())
        return false;
    const auto found = anywhereApplications_.find({layers_.back(), item});
    if (found == anywhereApplications_.end())
        return false;
    const auto script = found->second;
    runFile(script);
    events.push_back({"selectInventoryObject", {""}, "anywhere_application"});
    flushNavigation();
    return true;
}
void Runtime::openCutscene(const std::string& name, const std::string& after, bool skippable) {
    auto movie = std::make_unique<Cutscene>(assets_, "cutscenes/" + normalize(name) + ".csc");
    movie->after = after;
    movie->skippable = skippable;
    cutscene_ = std::move(movie);
    cutsceneActive_ = true;
    ++cutsceneGeneration_;
}
void Runtime::finishCutscene() {
    if (!cutscene())
        return;
    const auto after = cutscene_->after;
    cutsceneActive_ = false;
    runFile(after);
}
bool Runtime::skipCutscene() {
    if (!cutscene() || !cutscene_->skippable)
        return false;
    finishCutscene();
    flushNavigation();
    return true;
}
bool Runtime::placeMixing(const std::string& item) {
    auto* mix = mixing();
    if (!mix || !mix->ingredients().contains(item))
        return false;
    const auto& ingredient = mix->ingredients().at(item);
    if (!ingredient.fromTable && !hasInventory(item))
        return false;
    if (!mix->place(item))
        return false;
    if (!ingredient.fromTable)
        consumeInventory(item);
    runFile(ingredient.addScript);
    return true;
}
bool Runtime::mixIngredient(const std::string& item) {
    auto* mix = mixing();
    if (!mix || !mix->putInMortar(item))
        return false;
    runFile(mix->ingredients().at(item).mixScript);
    return true;
}
bool Runtime::finishMixing(const std::string& item) {
    auto* mix = mixing();
    if (!mix || item != mix->finalInventory() || (!item.empty() && !hasInventory(item)) || !mix->finish())
        return false;
    if (!item.empty())
        consumeInventory(item);
    closeMixing();
    runFile(mix->onFinish);
    return true;
}
SpellMixing* Runtime::findMixing(const std::string& name) {
    const auto found = mixingGames_.find(name);
    return found == mixingGames_.end() ? nullptr : found->second.get();
}
SpellMixing& Runtime::ensureMixing(const std::string& name) {
    const auto definition = mixingPaths.find(name);
    if (definition == mixingPaths.end())
        throw std::runtime_error("Unknown mixing activity: " + name);
    if (!mixingGames_.contains(name)) {
        auto mix = std::make_unique<SpellMixing>(assets_, definition->second, std::random_device{}(),
                                                 name == "BanishSpirits" ? "red_string" : "");
        static const std::map<std::string, std::string> pages{
            {"BanishSpirits", "banish_spirits"},
            {"GraveyardGrease", "graveyard_grease"},
            {"SpiritRevealingMixture", "spirit_revealing_mixture"},
            {"AconiteAntidote", "aconite_antidote"}};
        const auto bytes = assets_.read("spellbook/sections/" + pages.at(name) + "/ingredients.xml");
        pugi::xml_document doc;
        if (!doc.load_buffer(bytes.data(), bytes.size()))
            throw std::runtime_error("Invalid skip recipe requirements");
        for (auto item : doc.child("spell").children("ingredient")) {
            const std::string id = item.attribute("item").value();
            const auto ingredient = mix->ingredients().find(id);
            const bool table = !item.attribute("container").as_bool() &&
                               !item.attribute("special").as_bool() &&
                               ingredient != mix->ingredients().end() && ingredient->second.fromTable;
            if (!table)
                mix->skipRequirements.push_back(id);
        }
        if (mix->skipRequirements.empty())
            throw std::runtime_error("Missing skip recipe requirements");
        mixingGames_[name] = std::move(mix);
    }
    return *mixingGames_.at(name);
}
void Runtime::closeRevealSpirits() {
    if (!reveal_)
        return;
    if (inRevealTick_) {
        revealExitRequested_ = true;
        return;
    }
    auto mode = std::move(reveal_);
    revealExitRequested_ = false;
    for (const auto& target : mode->targets())
        if (target.spirit->alpha > 0 && target.spirit->alpha < 1)
            spiritFades_.emplace_back(target.spirit, mode->vanishSpeed);
    runFile(mode->onExit);
    events.push_back({"revealBrushSound", {""}, "reveal_spirits"});
    events.push_back({"endRevealSpirits", {}, "reveal_spirits"});
    events.push_back({"selectInventoryObject", {""}, "reveal_spirits"});
}
void Runtime::closeHexBones() {
    if (!hexBones())
        return;
    hexBones_->leave();
    hexBonesActive_ = false;
}
bool Runtime::applySpiritLockPiece(size_t slot, const std::string& item) {
    auto* lock = spiritLock();
    if (!lock || !hasInventory(item) || !lock->place(slot, item))
        return false;
    consumeInventory(item);
    events.push_back({"selectInventoryObject", {""}, "spirit_lock"});
    return true;
}
void Runtime::closeSpiritLock() {
    if (!spiritLock())
        return;
    spiritLockActive_ = false;
    for (auto& [id, effect] : spiritLock_->effects())
        particleWorld_.reset(*effect);
    events.push_back({"selectInventoryObject", {""}, "spirit_lock"});
    events.push_back({"clearHintBubble", {}, "spirit_lock"});
    // Native ExitMinigame requests the previous world location again. Merely
    // removing the activity skips its onEnter hook (the gazebo ending lives
    // there, rather than in the lock's onFinish script).
    if (!layers_.empty())
        navigation_.emplace_back(layers_.back(), false);
    runFile(spiritLock_->onExit);
}
BlockedWell& Runtime::ensureBlockedWell() {
    if (!blockedWell_)
        blockedWell_ = std::make_unique<BlockedWell>(assets_, "minigames/blocked_well");
    return *blockedWell_;
}
void Runtime::processCemeteryGateEvents() {
    if (!cemeteryGate())
        return;
    for (const auto& event : cemeteryGate_->takeEvents()) {
        if (event.kind == GateEvent::Sound)
            events.push_back({"playSound", {event.value}, "cemetery_gate"});
        else if (event.kind == GateEvent::Hint) {
            events.push_back({"showHint", {string(event.value)}, "cemetery_gate"});
            events.push_back({"playSound", {"sound/sfx/sfx_reactive_pop_up"}, "cemetery_gate"});
        } else if (!cemeteryGate_->rewarded) {
            cemeteryGate_->rewarded = true;
            events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "cemetery_gate"});
            runFile(cemeteryGate_->onFinish);
        }
    }
}
void Runtime::processSkullsObeyEvents() {
    if (!skullsObey())
        return;
    for (const auto& event : skullsObey_->takeEvents()) {
        if (event.kind == SkullEvent::Sound)
            events.push_back({"playSound", {event.value}, "skulls_obey"});
        else if (event.kind == SkullEvent::WheelLoop)
            events.push_back({"skullsWheelSound", {event.value}, "skulls_obey"});
        else if (event.kind == SkullEvent::Hint) {
            events.push_back({"showHint", {string(event.value)}, "skulls_obey"});
            events.push_back({"playSound", {"sound/sfx/sfx_reactive_pop_up"}, "skulls_obey"});
        } else if (!skullsObey_->rewarded) {
            skullsObey_->rewarded = true;
            runFile(skullsObey_->onFinish);
        }
    }
}
void Runtime::closeSkullsObey() {
    if (!skullsObey() || skullsObey_->busy())
        return;
    skullsObey_->leave();
    processSkullsObeyEvents();
    skullsObeyActive_ = false;
    particleWorld_.reset(skullsObey_->eyes());
    events.push_back({"selectInventoryObject", {""}, "skulls_obey"});
    runFile(skullsObey_->onExit);
}
void Runtime::closeCemeteryGate() {
    if (!cemeteryGate())
        return;
    processCemeteryGateEvents();
    cemeteryGateActive_ = false;
    cemeteryGate_->leave();
    particleWorld_.reset(cemeteryGate_->effect());
    events.push_back({"selectInventoryObject", {""}, "cemetery_gate"});
    runFile(cemeteryGate_->onExit);
}
bool Runtime::chooseCircleChalk() {
    auto* circle = voodooCircle();
    if (!circle || !hasInventory("chalk"))
        return false;
    const bool result = circle->chooseChalk();
    processVoodooCircleEvents();
    return result;
}
bool Runtime::interactVoodooCircle(const std::string& target, const std::string& inventory) {
    auto* circle = voodooCircle();
    if (!circle)
        return false;
    circle->setSpellKnown(flag("enableSpell:" + circle->spell()));
    bool result = false;
    if (!inventory.empty()) {
        if (!hasInventory(inventory))
            return false;
        std::optional<size_t> slot;
        if (target == "spell_item_slot_central")
            slot = circle->items().size() - 1;
        else if (target.starts_with("spell_item_slot_"))
            slot = size_t(std::stoul(target.substr(16))) - 1;
        result = circle->apply(inventory, slot);
        if (!circle->drawing())
            events.push_back({"selectInventoryObject", {""}, "voodoo_circle"});
    } else if (target.starts_with("obj_") && !target.ends_with("_moved"))
        result = circle->clearGarbage(size_t(std::stoul(target.substr(4))) - 1);
    else if (target.starts_with("spell_item_") && !target.starts_with("spell_item_slot_") &&
             !target.ends_with("_placed"))
        result = circle->takeItem(size_t(std::stoul(target.substr(11))) - 1);
    else if (target == "moss")
        result = circle->takeMoss();
    else
        circle->idleClick(hasInventory("chalk"));
    processVoodooCircleEvents();
    return result;
}
void Runtime::processVoodooCircleEvents() {
    auto* circle = voodooCircle();
    if (!circle)
        return;
    for (const auto& event : circle->takeEvents()) {
        if (event.kind == CircleEvent::Sound)
            events.push_back({"playSound", {event.value}, "voodoo_circle"});
        else if (event.kind == CircleEvent::ChalkSound)
            events.push_back({"circleChalkSound", {event.value}, "voodoo_circle"});
        else if (event.kind == CircleEvent::Hint || event.kind == CircleEvent::PopupHint) {
            events.push_back({"showHint", {string(event.value)}, "voodoo_circle"});
            if (event.kind == CircleEvent::PopupHint)
                events.push_back({"playSound", {"sound/sfx/sfx_reactive_pop_up"}, "voodoo_circle"});
        } else if (event.kind == CircleEvent::Award)
            grantInventory(event.value);
        else if (event.kind == CircleEvent::Consume)
            consumeInventory(event.value);
        else if (event.kind == CircleEvent::Select)
            events.push_back({"selectInventoryObject", {event.value}, "voodoo_circle"});
        else if (event.kind == CircleEvent::Flash) {
            const auto found = circle->effects().find("fx_voodoo_circle_flash");
            if (found != circle->effects().end()) {
                auto& effect = *found->second;
                particleWorld_.reset(effect);
                effect.enabled = true;
                effect.particleGenerating = true;
                effect.treeTime = 0;
            }
        } else if (!circle->rewarded) {
            circle->rewarded = true;
            const auto script = circle->onFinish;
            closeVoodooCircle();
            runFile(script);
        }
    }
}
void Runtime::closeVoodooCircle() {
    auto* circle = voodooCircle();
    if (!circle)
        return;
    circle->leave();
    processVoodooCircleEvents();
    voodooCircleActive_ = false;
    for (auto& [id, effect] : circle->effects())
        particleWorld_.reset(*effect);
    runFile(circle->onExit);
}
void Runtime::closeBlockedWell() {
    if (!blockedWell())
        return;
    blockedWellActive_ = false;
    events.push_back({"selectInventoryObject", {""}, "blocked_well"});
    events.push_back({"clearHintBubble", {}, "blocked_well"});
    runFile(blockedWell_->onExit);
}
bool Runtime::placeApothecaryItem(size_t slot, const std::string& item) {
    auto* shelves = apothecary();
    if (!shelves || !hasInventory(item) || !shelves->place(slot, item))
        return false;
    consumeInventory(item);
    return true;
}
void Runtime::closeApothecary() {
    if (!apothecary())
        return;
    // Input can enqueue a reward and then exit in the same SDL frame. Deliver
    // its inventory and Lua events before the activity stops receiving ticks.
    processApothecaryEvents(*apothecary_);
    if (!apothecary())
        return;
    apothecaryActive_ = false;
    apothecary_->leave();
    for (auto& [id, effect] : apothecary_->effects())
        particleWorld_.reset(*effect);
    events.push_back({"selectInventoryObject", {""}, "apothecary"});
    events.push_back({"clearHintBubble", {}, "apothecary"});
    runFile(apothecary_->onExit);
}
void Runtime::processApothecaryEvents(Apothecary& shelves) {
    for (const auto& event : shelves.takeEvents()) {
        if (event.kind == ApothecaryEvent::Sound)
            events.push_back({"playSound", {event.value}, "apothecary"});
        else if (event.kind == ApothecaryEvent::Hint || event.kind == ApothecaryEvent::PopupHint) {
            events.push_back({"showHint", {string(event.value)}, "apothecary"});
            if (event.kind == ApothecaryEvent::PopupHint)
                events.push_back({"playSound", {"sound/sfx/sfx_reactive_pop_up"}, "apothecary"});
        } else if (event.kind == ApothecaryEvent::Award)
            grantInventory(event.value);
        else if (event.kind == ApothecaryEvent::Solve)
            runFile(shelves.onSolve);
        else if (event.kind == ApothecaryEvent::Finish)
            runFile(shelves.onFinish);
    }
}
bool Runtime::applySailInventory(const std::string& item, float x, float y) {
    auto* cloth = sail();
    if (!cloth || !hasInventory(item))
        return false;
    if (cloth->ready())
        return cloth->sew(item);
    if (!cloth->place(item, x, y))
        return false;
    consumeInventory(item);
    requestSailTutorial();
    return true;
}
void Runtime::requestSailTutorial() {
    if (!sail() || tutorialSeen(TutorialId::SailPiece))
        return;
    for (size_t i = 0; i < sail()->pieces().size(); ++i)
        if (sail()->pieces()[i].placed) {
            const auto& sprite = sail()->piece(i);
            const float angle = sprite.angle * 3.14159265358979323846f / 180;
            const float x = (sprite.width * .5f - sprite.pivotX) * sprite.scale,
                        y = (sprite.height * .5f - sprite.pivotY) * sprite.scale;
            requestTutorial(TutorialId::SailPiece,
                            std::pair{sprite.x + x * std::cos(angle) - y * std::sin(angle),
                                      sprite.y + x * std::sin(angle) + y * std::cos(angle)});
            break;
        }
}
void Runtime::closeSail() {
    if (!sail())
        return;
    sailActive_ = false;
    sail_->leave();
    sail_->takeEvents();
    events.push_back({"selectInventoryObject", {""}, "sail"});
    events.push_back({"clearHintBubble", {}, "sail"});
    runFile(sail_->onExit);
    if (!layers_.empty())
        navigation_.emplace_back(layers_.back(), false);
}
bool Runtime::catchLeech(size_t index, const std::string& item) {
    auto* worms = leeches();
    if (!worms || (!item.empty() && !hasInventory(item)))
        return false;
    const bool caught = worms->catchLeech(index, item);
    processLeechesEvents();
    return caught;
}
bool Runtime::throwLeech() {
    auto* worms = leeches();
    if (!worms || !worms->holding())
        return false;
    const auto orange = worms->heldOrange();
    if (!hasInventory(orange) || !worms->throwIntoSwamp())
        return false;
    consumeInventory(orange);
    processLeechesEvents();
    return true;
}
void Runtime::inspectLeeches(const std::string& target) {
    auto* worms = leeches();
    if (!worms)
        return;
    if (worms->holding() && target == "swamp") {
        throwLeech();
        return;
    }
    worms->inspect(target);
    processLeechesEvents();
}
void Runtime::processLeechesEvents() {
    if (!leeches_)
        return;
    for (const auto& event : leeches_->takeEvents()) {
        if (event.kind == LeechesEvent::Sound)
            events.push_back({"playSound", {event.value}, "leeches"});
        else if (event.kind == LeechesEvent::Hint)
            events.push_back({"showHint", {string(event.value)}, "leeches"});
        else if (!leeches_->rewarded) {
            leeches_->rewarded = true;
            events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "leeches"});
            runFile(leeches_->onFinish);
        }
    }
}
void Runtime::closeLeeches() {
    if (!leeches())
        return;
    processLeechesEvents();
    leechesActive_ = false;
    leeches_->leave();
    events.push_back({"selectInventoryObject", {""}, "leeches"});
    events.push_back({"clearHintBubble", {}, "leeches"});
    runFile(leeches_->onExit);
    if (!layers_.empty())
        navigation_.emplace_back(layers_.back(), false);
}
void Runtime::closePiano() {
    if (!piano())
        return;
    piano_->enter();
    piano_->takeSounds();
    pianoActive_ = false;
    runFile(piano_->onExit);
}
void Runtime::closeDoveCote() {
    if (!doveCote())
        return;
    doveCote_->leave();
    doveCote_->takeEvents();
    doveCoteActive_ = false;
    events.push_back({"selectInventoryObject", {""}, "dove_cote"});
    runFile(doveCote_->onExit);
}
void Runtime::closeCellarDoors() {
    if (!cellarDoors())
        return;
    cellarDoors_->enter();
    cellarDoors_->takeEvents();
    cellarDoorsActive_ = false;
    events.push_back({"selectInventoryObject", {""}, "cellar_doors"});
}
void Runtime::closeRats(bool returnToLocation) {
    if (!rats())
        return;
    rats_->leave();
    rats_->takeEvents();
    ratsActive_ = false;
    events.push_back({"activityAmbient", {""}, "rats"});
    events.push_back({"selectInventoryObject", {""}, "rats"});
    // Re-entering the suspended location executes its original on_enter. In
    // the cellar that script sends an unfinished attempt back to the backyard.
    if (returnToLocation && !layers_.empty())
        navigation_.emplace_back(layers_.back(), false);
}
bool Runtime::applyCellarAsh() {
    if (!cellarDoors() || !hasInventory("ash") || !cellarDoors_->applyAsh())
        return false;
    consumeInventory("ash");
    events.push_back({"selectInventoryObject", {""}, "cellar_doors"});
    return true;
}
bool Runtime::feedDoveCote() {
    if (!doveCote() || !hasInventory("seeds") || !doveCote_->feed())
        return false;
    events.push_back({"selectInventoryObject", {""}, "dove_cote"});
    return true;
}
bool Runtime::connectDoveRope(size_t roll) {
    auto* cote = doveCote();
    if (!cote || !hasInventory("rope") || !cote->connect(roll))
        return false;
    if (cote->secured()) {
        consumeInventory("rope");
        events.push_back({"selectInventoryObject", {""}, "dove_cote"});
    }
    return true;
}
bool Runtime::takePianoReward() {
    auto* keys = piano();
    if (!keys || !keys->takeReward())
        return false;
    grantInventory(keys->reward);
    events.push_back({"playSound", {"sound/sfx/sfx_item_found"}, "piano"});
    runFile(keys->onFinish);
    return true;
}
void Runtime::finishVoodoo() {
    auto* doll = voodoo();
    if (doll && doll->complete() && !doll->rewarded) {
        doll->rewarded = true;
        events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "voodoo_doll"});
        runFile(doll->onFinish);
    }
}
bool Runtime::placeVoodoo(const std::string& item, bool centralTarget, float x, float y) {
    auto* doll = voodoo();
    if (!doll || !hasInventory(item) || !doll->place(item, centralTarget, x, y))
        return false;
    consumeInventory(item);
    finishVoodoo();
    return true;
}
bool Runtime::applyVoodoo(const std::string& item) {
    auto* doll = voodoo();
    if (!doll || !doll->apply(item))
        return false;
    finishVoodoo();
    return true;
}
void Runtime::grantInventory(const std::string& item, bool effect) {
    const bool added = inventory_.insert(item).second;
    if (added)
        inventoryOrder_.push_back(item);
    found_.insert(item);
    if (added && effect) {
        events.push_back({"inventoryAdded", {item}, "inventory"});
    }
}
void Runtime::inventoryFeedbackFinished() {
    for (const auto id : {TutorialId::FirstItem, TutorialId::InventoryHover, TutorialId::InventoryPin})
        if (!tutorialSeen(id)) {
            requestTutorial(id);
            break;
        }
}
void Runtime::consumeInventory(const std::string& item) {
    inventory_.erase(item);
    inventoryOrder_.erase(std::remove(inventoryOrder_.begin(), inventoryOrder_.end(), item),
                          inventoryOrder_.end());
}
bool Runtime::placeWindowMixture() {
    auto* window = windowCleaning();
    if (!window || !hasInventory("mixture_grease_cleaning") || !window->placeMixture())
        return false;
    consumeInventory("mixture_grease_cleaning");
    return true;
}
bool Runtime::wetWindowSponge() {
    auto* window = windowCleaning();
    if (!window || window->phase() != WindowCleaning::Phase::Idle || !window->mixturePlaced() ||
        !hasInventory("sponge"))
        return false;
    consumeInventory("sponge");
    grantInventory("sponge_wet", false);
    events.push_back({"playSound", {"sound/sfx/mg/window_cleaning/mg_item_wet"}, "window_cleaning"});
    events.push_back({"selectInventoryObject", {"sponge_wet"}, "window_cleaning"});
    return true;
}
bool Runtime::beginWindowCleaning(size_t section, const std::string& sponge, int x, int y) {
    auto* window = windowCleaning();
    if (!window || (sponge != "sponge" && sponge != "sponge_wet") || !hasInventory(sponge) ||
        !window->begin(section, sponge == "sponge_wet", x, y))
        return false;
    events.push_back({"playSound", {"sound/sfx/mg/window_cleaning/mg_window_cleaning"}, "window_cleaning"});
    return true;
}
void Runtime::closeWindowCleaning() {
    if (!windowCleaning())
        return;
    windowCleaning_->leave();
    windowCleaningActive_ = false;
    if (hasInventory("sponge_wet")) {
        consumeInventory("sponge_wet");
        grantInventory("sponge", false);
        events.push_back(
            {"showHint", {string("puzzles/window_cleaning/hints/sponge_dried_out")}, "window_cleaning"});
    }
}
bool Runtime::pickHob(size_t item, size_t piece, const std::string& inventory) {
    auto* round = hob();
    if (!round || hobFeedback_.transient() || (!inventory.empty() && !hasInventory(inventory)))
        return false;
    if (item >= round->items.size() || piece >= round->items[item].pieces.size())
        return false;
    const auto& target = round->items[item];
    const auto& picked = target.pieces[piece];
    if (!target.target || picked.found || round->rewarded || picked.inventory != inventory)
        return false;
    if (inventory.empty()) {
        hobFeedback_.found({item, piece});
        hobTutorialArmed_ = false;
        hobTutorialSeconds_ = 0;
        hobHintSeconds_ = 0;
        hobHint_.particleGenerating = false;
        particleWorld_.setGenerating(hobHint_, false);
        events.push_back({"playSound", {"sound/sfx/hob/ho_found"}, "hob"});
        if (assets_.contains("hobs/found.ogg"))
            events.push_back({"playSound", {"hobs/found"}, "hob"});
    } else {
        hobFeedback_.applied();
        finishHobPick(item, piece, inventory);
    }
    return true;
}
bool Runtime::missHob(float x, float y) {
    if (!hob() || !hobFeedback_.miss(x, y))
        return false;
    hobTutorialArmed_ = false;
    hobTutorialSeconds_ = 0;
    hobHintSeconds_ = 0;
    hobHint_.particleGenerating = false;
    particleWorld_.setGenerating(hobHint_, false);
    events.push_back({"selectInventoryObject", {""}, "hob"});
    events.push_back({"playSound", {"sound/sfx/hob/ho_penalty"}, "hob"});
    return true;
}
void Runtime::finishHobPick(size_t item, size_t piece, const std::string& inventory) {
    auto* round = hob();
    if (!round || !round->pick(item, piece, inventory))
        return;
    const auto& picked = round->items[item].pieces[piece];
    hobTutorialSeconds_ = 0;
    hobTutorialArmed_ = true;
    if (picked.removeInventory)
        consumeInventory(inventory);
    if (picked.dropInventory || picked.removeInventory)
        events.push_back({"selectInventoryObject", {""}, "hob"});
    if (round->complete() && !round->rewarded) {
        hobFeedback_.waitForReward();
        hobTutorialArmed_ = false;
        hobTutorialSeconds_ = 0;
        events.push_back({"playSound", {"sound/sfx/mg/mg_finish"}, "hob"});
        events.push_back({"showMessageDialogWithImage",
                          {round->congratulations, round->rewardImage, ""},
                          "hob_reward:" + activeHob_});
    }
}
bool Runtime::requestTutorial(TutorialId id, std::optional<std::pair<float, float>> anchor) {
    const auto& spec = tutorialSpec(id);
    if (!flag("tutorialsEnabled") || layers_.empty() || (!spec.repeat && tutorialSeen(id)))
        return false;
    const auto [x, y] = anchor.value_or(std::pair{spec.x, spec.y});
    if (!std::isfinite(x) || !std::isfinite(y))
        throw std::runtime_error("Invalid tutorial anchor");
    flags_.insert("tutorial:" + std::string(spec.key));
    const auto payload =
        spec.picture ? "hud/common/hint_bubble_inventory" : string("hint_bubbles/" + std::string(spec.key));
    events.push_back({"showTutorialHint",
                      {payload, std::to_string(x), std::to_string(y), std::to_string(static_cast<int>(id))},
                      "tutorial:" + std::string(spec.key)});
    return true;
}
int Runtime::api(const std::string& name) {
    auto text = [&](int i) { return luaText(lua_, i); };
    if (name == "skipMinigame") {
        skipMinigame();
        return 0;
    }
    auto boolean = [&](bool value) {
        lua_pushboolean(lua_, value);
        return 1;
    };
    auto value = [&](const std::string& s) {
        lua_pushlstring(lua_, s.data(), s.size());
        return 1;
    };
    auto object = [&]() -> Object& {
        const auto id = text(1);
        auto* found = findObject(id);
        if (!found)
            throw std::runtime_error("Unknown object in " + name + ": " + id);
        return *found;
    };
    if (name == "getString")
        return value(string(text(1)));
    if (name == "playAnimation") {
        // Native 5663B0 resolves the animation against the calling Lua file,
        // not the target object (the final fade object has no own script).
        auto& node = object();
        const auto base = scriptStack_.empty() ? std::string{} : directory(scriptStack_.back());
        auto path = resolve(base, text(2));
        if (!path.ends_with(".animset"))
            path += ".animset";
        AnimationSet animation(assets_, path);
        for (const auto& track : animation.tracks())
            if (track.property == AnimatedProperty::Scale || track.property == AnimatedProperty::Rotation ||
                track.positionOffset)
                throw std::runtime_error("Scene animation transform not supported: " + path);
        node.scriptAnimation = Object::ScriptAnimation{std::move(animation), 0};
        sampleScriptAnimation(node);
        return 0;
    }
    if (name == "playTreeAnimation") {
        if (!lua_isboolean(lua_, 2) || !lua_isboolean(lua_, 3))
            throw std::runtime_error("playTreeAnimation requires two boolean flags");
        auto& node = object();
        node.treePlaying = true;
        node.treeLooped = lua_toboolean(lua_, 2);
        node.treeLoopOverridden = true;
        node.treePlayOption = lua_toboolean(lua_, 3);
        return 0;
    }
    if (name == "stopTreeAnimation") {
        object().treePlaying = false;
        return 0;
    }
    if (name == "rewindTreeAnimation") {
        const auto time = luaNumber(lua_, 2);
        if (time < 0)
            throw std::runtime_error("Invalid animation tree time");
        object().treeTime = time;
        return 0;
    }
    // The native binding converts the numeric argument to a boolean. It is
    // a request flag, not a duration (the original scripts often pass 2.0).
    if (name == "requestParticlesPresim") {
        auto& node = object();
        const bool requested = luaNumber(lua_, 2) != 0;
        node.particlesPresimRequested = requested;
        if (requested)
            node.particlePresimPending += 1;
        return 0;
    }
    if (name == "setParticleEmittersEnabled") {
        if (!lua_isboolean(lua_, 2))
            throw std::runtime_error("setParticleEmittersEnabled requires a boolean");
        auto& node = object();
        node.particleGenerating = lua_toboolean(lua_, 2);
        particleWorld_.setGenerating(node, node.particleGenerating);
        return 0;
    }
    if (name == "setCurrentChapter") {
        const auto chapter = luaNumber(lua_, 1);
        if (chapter < 1 || chapter > std::numeric_limits<int>::max() || std::trunc(chapter) != chapter)
            throw std::runtime_error("Invalid chapter number");
        currentChapter_ = int(chapter);
        return 0;
    }
    if (name == "stripmineSnakeBitten")
        return 0; // Native 56CBF0 has no gameplay state effect.
    if (name == "startTutorials") {
        flags_.insert("tutorialsEnabled");
        return 0;
    }
    if (name == "showTutorialHint") {
        const auto hint = text(1);
        // Native 56C3D0 recognizes just these two Lua-triggered tutorials;
        // core ctor4126A0 leaves them disabled until startTutorials().
        if (hint != "carriage" && hint != "new_areas")
            return 0;
        std::optional<std::pair<float, float>> anchor;
        if (lua_isnumber(lua_, 2) && lua_isnumber(lua_, 3))
            anchor = std::pair{float(luaNumber(lua_, 2)), float(luaNumber(lua_, 3))};
        requestTutorial(hint == "carriage" ? TutorialId::Carriage : TutorialId::NewArea, anchor);
        return 0;
    }
    if (name == "stripmineGoodItemAction" || name == "stripmineWrongItemAction") {
        // Both native bindings only validate/resolve a string in this build;
        // they have no inventory or puzzle-state side effects.
        text(1);
        return 0;
    }
    if (name == "gotoState") {
        auto& o = object();
        transition(o, text(2));
        return 0;
    }
    if (name == "gotoCarriage") {
        openCarriage();
        return 0;
    }
    if (name == "isInState") {
        auto* o = findObject(text(1));
        return boolean(o && o->state == text(2));
    }
    if (name == "isObjectFsmInitialized") {
        auto* o = findObject(text(1));
        return boolean(o && o->initialized);
    }
    if (name == "setVisible") {
        object().visible = lua_toboolean(lua_, 2);
        return 0;
    }
    if (name == "setInteractive") {
        object().interactive = lua_toboolean(lua_, 2);
        return 0;
    }
    if (name == "setEnabled") {
        object().enabled = lua_toboolean(lua_, 2);
        return 0;
    }
    if (name == "setCursor") {
        object().cursor = text(2);
        return 0;
    }
    if (name == "setDefaultCursor") {
        object().cursor.clear();
        return 0;
    }
    if (name == "getAlpha") {
        lua_pushnumber(lua_, object().alpha);
        return 1;
    }
    if (name == "setAlpha") {
        object().alpha = static_cast<float>(std::clamp(luaNumber(lua_, 2), 0., 1.));
        return 0;
    }
    if (name == "setTexture") {
        auto& o = object();
        const auto base = scriptStack_.empty() ? directory(o.script) : directory(scriptStack_.back());
        o.texture = resolve(base, text(2));
        return 0;
    }
    if (name == "setTimer") {
        object().timer = std::max(0., luaNumber(lua_, 2));
        return 0;
    }
    if (name == "hasInventoryObject")
        return boolean(inventory_.contains(text(1)));
    if (name == "wasInventoryObjectFound")
        return boolean(found_.contains(text(1)));
    if (name == "addInventoryObject") {
        grantInventory(text(1));
        return 0;
    }
    if (name == "reAddInventoryObject") {
        const auto item = text(1);
        if (hasInventory(item)) {
            consumeInventory(item);
            grantInventory(item, false);
        }
        return 0;
    }
    if (name == "removeInventoryObject") {
        consumeInventory(text(1));
        return 0;
    }
    if (name == "gotoLocation" || name == "gotoInSet") {
        navigation_.emplace_back(text(1), name == "gotoInSet");
        return 0;
    }
    if (name == "gotoHob") {
        const auto base = scriptStack_.empty() ? std::string{} : directory(scriptStack_.back());
        openHob(resolve(base, text(1)), text(2));
        return 0;
    }
    if (name == "gotoRevealSpiritsMode") {
        if (inRevealTick_)
            throw std::runtime_error("Recursive reveal-mode entry");
        const auto brush = text(1), containerName = text(2);
        if (!hasInventory(brush))
            return 0;
        if (layers_.empty() || lua_gettop(lua_) < 3)
            throw std::runtime_error("Reveal spirits requires a location and targets");
        const auto& scene = *findScene(layers_.back());
        auto local = [&](const std::string& name) -> Object* {
            auto* result = findObject(name.find('.') == std::string::npos ? scene.id + '.' + name : name);
            if (!result ||
                std::find(scene.objects.begin(), scene.objects.end(), result) == scene.objects.end())
                throw std::runtime_error("Unknown reveal spirits scene object: " + name);
            return result;
        };
        auto* container = local(containerName);
        std::vector<RevealSpirits::Target> targets;
        for (int index = 3; index <= lua_gettop(lua_); ++index) {
            auto* spirit = local(text(index));
            targets.push_back({spirit, local(text(index) + "_mask")});
        }
        if (reveal_)
            closeRevealSpirits();
        for (const auto& target : targets)
            std::erase_if(spiritFades_, [&](const auto& fade) { return fade.first == target.spirit; });
        reveal_ = std::make_unique<RevealSpirits>(assets_, std::move(targets), brush, *container);
        const auto count = std::count_if(reveal_->targets().begin(), reveal_->targets().end(),
                                         [](const auto& target) { return target.spirit->alpha < 1; });
        const std::vector<std::string> hintNames{"", "one", "two", "three", "four"};
        if (count > 0 && count <= 4)
            events.push_back({"showHintBubble",
                              {string("puzzles/reveal_spirits/hints/" + hintNames[size_t(count)] +
                                      (count == 1 ? "_spirit_to_find" : "_spirits_to_find"))},
                              "reveal_spirits"});
        events.push_back({"beginRevealSpirits", {brush}, "reveal_spirits"});
        return 0;
    }
    if (name == "revealSpirits_finish") {
        revealFinished_ = true;
        return 0;
    }
    if (name == "isMinigameFinishedRevealSpirits")
        return boolean(revealFinished_);
    if (name == "exitMinigame" && reveal_) {
        closeRevealSpirits();
        return 0;
    }
    if (name == "gotoCutscene") {
        const auto base = scriptStack_.empty() ? std::string{} : directory(scriptStack_.back());
        if (!lua_isboolean(lua_, 3))
            throw std::runtime_error("gotoCutscene requires a skip flag");
        const auto after = text(2);
        openCutscene(text(1), after.empty() ? std::string{} : resolve(base, after) + ".lua",
                     lua_toboolean(lua_, 3));
        return 0;
    }
    for (const auto& [suffix, path] : mixingPaths) {
        auto* mix = findMixing(suffix);
        if (name == "gotoMinigame" + suffix) {
            if (!mix) {
                mix = &ensureMixing(suffix);
            }
            if (!mix->complete()) {
                activeMixing_ = suffix;
                requestTutorial(TutorialId::SpellMixing);
            }
            return 0;
        }
        if (name == "isMinigameFinished" + suffix)
            return boolean(mix && mix->complete());
        if (name == "wasMinigameEntered" + suffix)
            return boolean(mix != nullptr);
        if (name == "isMixed" + suffix)
            return boolean(mix && mix->mixtureReady());
        if (name == "isIngredientUsed" + suffix) {
            const auto id = text(1);
            if (!mix)
                return boolean(false);
            const auto found = mix->ingredients().find(id);
            if (found == mix->ingredients().end())
                return 0;
            return boolean(found->second.bottle ? mix->bottlePlaced() : mix->slot(id).has_value());
        }
    }
    if (name == "gotoMinigameWindowCleaning") {
        if (!windowCleaning_)
            windowCleaning_ = std::make_unique<WindowCleaning>(assets_, "minigames/window_cleaning",
                                                               std::random_device{}());
        if (windowCleaning_->complete())
            return 0;
        windowCleaningActive_ = true;
        return 0;
    }
    if (name == "gotoMinigameVoodooDoll") {
        if (!voodoo_)
            voodoo_ = std::make_unique<VoodooDoll>(assets_, "minigames/_spellmixing_voodoo_doll");
        if (!voodoo_->complete())
            voodooActive_ = true;
        return 0;
    }
    if (name == "gotoMinigameLeeches") {
        if (!leeches_)
            leeches_ = std::make_unique<Leeches>(assets_, "minigames/leeches", std::random_device{}());
        leeches_->enter();
        leechesActive_ = true;
        runFile(leeches_->onEnter);
        return 0;
    }
    if (name == "wasMinigameEnteredLeeches")
        return boolean(leeches_ != nullptr);
    if (name == "isMinigameFinishedLeeches")
        return boolean(leeches_ && leeches_->complete());
    if (name == "isMinigameFinishedWithoutReward_Leeches")
        return boolean(leeches_ && leeches_->solved());
    if (name == "exitMinigame" && leeches()) {
        closeLeeches();
        return 0;
    }
    if (name == "gotoMinigameSail") {
        if (!sail_)
            sail_ = std::make_unique<Sail>(assets_, "minigames/sail", std::random_device{}());
        sail_->enter();
        sailActive_ = true;
        runFile(sail_->onEnter);
        requestSailTutorial();
        return 0;
    }
    if (name == "wasMinigameEnteredSail")
        return boolean(sail_ != nullptr);
    if (name == "isMinigameFinishedSail")
        return boolean(sail_ && sail_->complete());
    if (name == "sail_isFabricFit")
        return boolean(sail_ && sail_->ready());
    if (name == "exitMinigame" && sail()) {
        closeSail();
        return 0;
    }
    if (name == "gotoMinigameSortApothecaryItems") {
        if (!apothecary_)
            apothecary_ = std::make_unique<Apothecary>(assets_, "minigames/sort_apothecary_items",
                                                       std::random_device{}());
        for (auto& [id, effect] : apothecary_->effects())
            particleWorld_.reset(*effect);
        apothecary_->enter();
        apothecaryActive_ = true;
        runFile(apothecary_->onEnter);
        return 0;
    }
    if (name == "wasMinigameEnteredSortApothecaryItems")
        return boolean(apothecary_ != nullptr);
    if (name == "isMinigameFinishedSortApothecaryItems")
        return boolean(apothecary_ && apothecary_->complete());
    if (name == "isMinigameFinishedWithoutReward_SortApothecaryItems")
        return boolean(apothecary_ && apothecary_->solved());
    if (name == "exitMinigame" && apothecary()) {
        closeApothecary();
        return 0;
    }
    if (name == "gotoMinigameVoodooCircle") {
        if (!voodooCircle_)
            voodooCircle_ = std::make_unique<VoodooCircle>(assets_, "minigames/voodoo_circle");
        for (auto& [id, effect] : voodooCircle_->effects())
            particleWorld_.reset(*effect);
        voodooCircle_->enter(flag("enableSpell:" + voodooCircle_->spell()));
        voodooCircleActive_ = true;
        runFile(voodooCircle_->onEnter);
        return 0;
    }
    if (name == "wasMinigameEnteredVoodooCircle")
        return boolean(voodooCircle_ && voodooCircle_->entered());
    if (name == "isMinigameFinishedVoodooCircle")
        return boolean(voodooCircle_ && voodooCircle_->complete());
    if (name == "voodooCircle_isCircleCleared")
        return boolean(voodooCircle_ && voodooCircle_->cleared());
    if (name == "voodooCircle_isCircleDrawn")
        return boolean(voodooCircle_ && voodooCircle_->drawn());
    if (name == "exitMinigame" && voodooCircle()) {
        closeVoodooCircle();
        return 0;
    }
    if (name == "gotoMinigameSkullsObey") {
        if (!skullsObey_)
            skullsObey_ = std::make_unique<SkullsObey>(assets_, "minigames/skulls_obey");
        particleWorld_.reset(skullsObey_->eyes());
        skullsObey_->enter();
        skullsObeyActive_ = true;
        runFile(skullsObey_->onEnter);
        return 0;
    }
    if (name == "wasMinigameEnteredSkullsObey")
        return boolean(skullsObey_ && skullsObey_->entered());
    if (name == "isMinigameFinishedSkullsObey")
        return boolean(skullsObey_ && skullsObey_->complete());
    if (name == "exitMinigame" && skullsObey()) {
        closeSkullsObey();
        return 0;
    }
    if (name == "gotoMinigameCemeteryGate") {
        if (!cemeteryGate_)
            cemeteryGate_ =
                std::make_unique<CemeteryGate>(assets_, "minigames/cemetery_gate", std::random_device{}());
        particleWorld_.reset(cemeteryGate_->effect());
        cemeteryGate_->enter();
        cemeteryGateActive_ = true;
        runFile(cemeteryGate_->onEnter);
        return 0;
    }
    if (name == "wasMinigameEnteredCemeteryGate")
        return boolean(cemeteryGate_ && cemeteryGate_->entered());
    if (name == "isMinigameFinishedCemeteryGate")
        return boolean(cemeteryGate_ && cemeteryGate_->complete());
    if (name == "exitMinigame" && cemeteryGate()) {
        closeCemeteryGate();
        return 0;
    }
    if (name == "gotoMinigameBlockedWell") {
        auto& well = ensureBlockedWell();
        well.enter();
        blockedWellActive_ = true;
        runFile(well.onEnter);
        return 0;
    }
    if (name == "wasMinigameEnteredBlockedWell")
        return boolean(blockedWell_ && blockedWell_->entered());
    if (name == "isMinigameFinishedBlockedWell")
        return boolean(blockedWell_ && blockedWell_->complete());
    if (name == "blockedWell_getNumSpelledNames") {
        lua_pushinteger(lua_, ensureBlockedWell().spelled().size());
        return 1;
    }
    if (name == "blockedWell_addKnownName") {
        if (lua_isstring(lua_, 1))
            ensureBlockedWell().addKnownName(text(1));
        return 0;
    }
    if (name == "blockedWell_isNameKnown" || name == "blockedWell_isNameSpelled") {
        if (!lua_isstring(lua_, 1))
            return 0;
        auto& well = ensureBlockedWell();
        const auto answer =
            name == "blockedWell_isNameKnown" ? well.isNameKnown(text(1)) : well.isNameSpelled(text(1));
        return answer ? boolean(*answer) : 0;
    }
    if (name == "exitMinigame" && blockedWell()) {
        closeBlockedWell();
        return 0;
    }
    if (name == "gotoMinigameSpiritLock") {
        if (!spiritLock_)
            spiritLock_ = std::make_unique<SpiritLock>(assets_, "minigames/spirit_lock");
        for (auto& [id, effect] : spiritLock_->effects())
            particleWorld_.reset(*effect);
        const bool allFound = std::all_of(spiritLock_->pieces().begin(), spiritLock_->pieces().end(),
                                          [&](const auto& piece) { return wasFound(piece); });
        spiritLock_->enter(allFound);
        spiritLockActive_ = true;
        runFile(spiritLock_->onEnter);
        return 0;
    }
    if (name == "wasMinigameEnteredSpiritLock")
        return boolean(spiritLock_ != nullptr);
    if (name == "isMinigameFinishedSpiritLock")
        return boolean(spiritLock_ && spiritLock_->complete());
    if (name == "exitMinigame" && spiritLock()) {
        closeSpiritLock();
        return 0;
    }
    if (name == "gotoMinigameRats") {
        if (!rats_)
            rats_ = std::make_unique<Rats>(assets_, "minigames/rats");
        else
            rats_->enter();
        ratsActive_ = true;
        if (!rats_->complete())
            events.push_back({"activityAmbient", {"sound/sfx/mg/rats/sfx_rats"}, "rats"});
        return 0;
    }
    if (name == "wasMinigameEnteredRats")
        return boolean(rats_ != nullptr);
    if (name == "isMinigameFinishedRats")
        return boolean(rats_ && rats_->complete());
    if (name == "exitMinigame" && rats()) {
        closeRats();
        return 0;
    }
    if (name == "gotoMinigameCellarDoors") {
        if (!cellarDoors_)
            cellarDoors_ =
                std::make_unique<CellarDoors>(assets_, "minigames/cellar_doors", std::random_device{}());
        else
            cellarDoors_->enter();
        cellarDoorsActive_ = true;
        return 0;
    }
    if (name == "wasMinigameEnteredCellarDoors")
        return boolean(cellarDoors_ != nullptr);
    if (name == "isMinigameFinishedCellarDoors")
        return boolean(cellarDoors_ && cellarDoors_->complete());
    if (name == "exitMinigame" && cellarDoors()) {
        closeCellarDoors();
        return 0;
    }
    if (name == "gotoMinigameDoveCote") {
        if (!doveCote_)
            doveCote_ = std::make_unique<DoveCote>(assets_, "minigames/dove_cote", std::random_device{}());
        else
            doveCote_->enter();
        doveCoteActive_ = true;
        runFile(doveCote_->onEnter);
        events.push_back({"showHintBubble", {string("puzzles/dove_cote/hints/no_dove_inside")}, "dove_cote"});
        return 0;
    }
    if (name == "wasMinigameEnteredDoveCote")
        return boolean(doveCote_ != nullptr);
    if (name == "isMinigameFinishedDoveCote")
        return boolean(doveCote_ && doveCote_->complete());
    if (name == "exitMinigame" && doveCote()) {
        closeDoveCote();
        return 0;
    }
    if (name == "gotoMinigamePiano") {
        // The original matches callback requests the parlor before the piano.
        // Commit that earlier navigation before activating the activity.
        flushNavigation();
        if (!piano_)
            piano_ = std::make_unique<Piano>(assets_, "minigames/piano");
        else
            piano_->enter();
        pianoActive_ = true;
        runFile(piano_->onEnter);
        return 0;
    }
    if (name == "wasMinigameEnteredPiano")
        return boolean(piano_ != nullptr);
    if (name == "isMinigameFinishedPiano")
        return boolean(piano_ && piano_->complete());
    if (name == "isMinigameFinishedWithoutReward_Piano")
        return boolean(piano_ && piano_->opened());
    if (name == "exitMinigame" && piano()) {
        closePiano();
        return 0;
    }
    if (name == "gotoMinigameHexBones") {
        const bool first = !hexBones_;
        if (first)
            hexBones_ = std::make_unique<HexBones>(assets_, "minigames/hex_bones", std::random_device{}());
        hexBonesActive_ = true;
        if (first)
            events.push_back(
                {"showHintBubble", {string("puzzles/hex_bones/hints/first_enter")}, "hex_bones"});
        runFile(hexBones_->onEnter);
        return 0;
    }
    if (name == "wasMinigameEnteredHexBones")
        return boolean(hexBones_ != nullptr);
    if (name == "isMinigameStartedHexBones")
        return boolean(hexBones_ && hexBones_->thrown());
    if (name == "isMinigameFinishedHexBones")
        return boolean(hexBones_ && hexBones_->complete());
    if (name == "exitMinigame" && hexBones()) {
        closeHexBones();
        return 0;
    }
    if (name == "wasMinigameEnteredVoodooDoll")
        return boolean(voodoo_ != nullptr);
    if (name == "isMinigameFinishedVoodooDoll")
        return boolean(voodoo_ && voodoo_->complete());
    if (name == "voodooDoll_areIngredientsReady")
        return boolean(voodoo_ && voodoo_->arranged());
    if (name == "isIngredientUsedVoodooDoll") {
        const auto item = text(1);
        if (!voodoo_)
            return boolean(false);
        const auto index = voodoo_->ingredient(item);
        if (!index)
            return 0;
        return boolean(voodoo_->used(*index));
    }
    if (name == "exitMinigame" && voodoo()) {
        closeVoodoo();
        return 0;
    }
    if (name == "exitMinigame" && windowCleaning()) {
        closeWindowCleaning();
        return 0;
    }
    if (name == "exitMinigame" && mixing()) {
        closeMixing();
        return 0;
    }
    if (name == "gotoMinigameOuijiBoard") {
        if (!ouiji_)
            ouiji_ = std::make_unique<OuijiBoard>(assets_);
        ouiji_->enter();
        ouijiActive_ = true;
        requestTutorial(TutorialId::Ouiji);
        return 0;
    }
    if (name == "isMinigameFinishedOuijiBoard")
        return boolean(ouiji_ && ouiji_->complete());
    if (name == "isMinigameFinishedWithoutReward_OuijiBoard")
        return boolean(ouiji_ && ouiji_->solved());
    if (name == "isLoaFullSpiritRevealingMixture") {
        const auto* mix = findMixing("SpiritRevealingMixture");
        return boolean(mix && mix->arranged());
    }
    if (name == "isMinigameFinishedWindowCleaning")
        return boolean(windowCleaning_ && windowCleaning_->complete());
    if (name == "wasMinigameEnteredWindowCleaning")
        return boolean(windowCleaning_ != nullptr);
    if (name == "wasMinigameEnteredOuijiBoard")
        return boolean(ouiji_ != nullptr);
    // Other activities have not been played in this runtime, so their finish flags are false.
    if (name.starts_with("isMinigameFinished"))
        return boolean(false);
    if (name == "exitMinigame" && ouiji()) {
        closeOuiji();
        return 0;
    }
    if (name == "exitMinigame") {
        // Native 568500 only posts ExitMinigame; a late reward callback may
        // post it again after the skip transition already returned to a scene.
        if (hob())
            closeHob();
        return 0;
    }
    if (name == "rechargeHintBottle") {
        hintBottle_.refill();
        return 0;
    }
    if (name == "enableLocationHint") {
        const auto key = text(1);
        if (!flag("usedHint:" + key))
            enabledHints_.insert(key);
        return 0;
    }
    if (name == "disableLocationHint") {
        enabledHints_.erase(text(1));
        return 0;
    }
    if (name == "isLocationHintUsed")
        return boolean(flags_.contains("usedHint:" + text(1)));
    if (name == "enableSpellBook" || name == "enablePlantBook") {
        flags_.insert(name);
        return 0;
    }
    if (name == "isSpellBookEnabled")
        return boolean(flags_.contains("enableSpellBook"));
    if (name == "isPlantBookEnabled")
        return boolean(flags_.contains("enablePlantBook"));
    if (name == "isSpellEnabled")
        return boolean(flags_.contains("enableSpell:" + text(1)));
    if (name == "addAnywhereAppliable") {
        const auto scene = text(1), item = text(2), path = text(3);
        if (findScene(scene) && !item.empty()) {
            const auto base = scriptStack_.empty() ? std::string{} : directory(scriptStack_.back());
            auto script = resolve(base, path);
            if (!script.ends_with(".lua"))
                script += ".lua";
            anywhereApplications_[{scene, item}] = std::move(script);
        }
        return 0;
    }
    if (name == "removeAnywhereAppliable") {
        anywhereApplications_.erase({text(1), text(2)});
        return 0;
    }
    if (name == "enableSpell") {
        const auto spell = text(1);
        if (flags_.insert(name + ':' + spell).second)
            latestSpell_ = spell;
        events.push_back({"playSound", {"sound/sfx/sfx_book_updated"}, "spellbook"});
        requestTutorial(TutorialId::NewSpell);
        return 0;
    }
    if (name == "enablePlantBookPage" || name == "enableJournalPage" || name == "allowZoneAccess") {
        flags_.insert(name + ':' + text(1));
        return 0;
    }
    if (name == "disallowZoneAccess") {
        flags_.erase("allowZoneAccess:" + text(1));
        return 0;
    }
    if (name == "endGame")
        flags_.insert("gameCompleted");
    // Presentation commands are explicit events; the player must handle or report them.
    Event event{name, {}, scriptStack_.empty() ? std::string{} : scriptStack_.back()};
    for (int i = 1; i <= lua_gettop(lua_); ++i) {
        if (lua_isstring(lua_, i))
            event.args.push_back(text(i));
        else if (lua_isboolean(lua_, i))
            event.args.push_back(lua_toboolean(lua_, i) ? "true" : "false");
        else
            throw std::runtime_error("Unsupported event argument: " + name);
    }
    events.push_back(std::move(event));
    return 0;
}
} // namespace deadtime
