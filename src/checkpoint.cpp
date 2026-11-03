#include "activity_checkpoints.hpp"
#include "runtime.hpp"
#include <pugixml.hpp>
extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
namespace deadtime {
namespace {
void require(bool ok, const std::string& message) {
    if (!ok)
        throw std::runtime_error("Checkpoint: " + message);
}
struct Stack {
    lua_State* lua;
    int top;
    explicit Stack(lua_State* state) : lua(state), top(lua_gettop(state)) {}
    ~Stack() {
        lua_settop(lua, top);
    }
};
std::string fingerprint(const Assets& assets) {
    uint64_t hash = 14695981039346656037ull;
    auto add = [&](const std::string& bytes) {
        for (unsigned char c : bytes) {
            hash ^= c;
            hash *= 1099511628211ull;
        }
        hash ^= 0xff;
        hash *= 1099511628211ull;
    };
    for (const auto* extension : {".loc", ".lua", ".hob", ".xml", ".animset", ".font"})
        for (const auto& path : assets.files(extension)) {
            add(path);
            add(assets.read(path));
        }
    std::ostringstream text;
    text << std::hex << hash;
    return text.str();
}
double number(pugi::xml_node node, const char* key) {
    const auto attribute = node.attribute(key);
    require(bool(attribute), std::string("missing numeric field ") + key);
    const std::string text = attribute.value();
    size_t used = 0;
    double value = std::stod(text, &used);
    require(used == text.size() && std::isfinite(value), std::string("invalid number ") + key);
    return value;
}
bool boolean(pugi::xml_node node, const char* key) {
    const std::string value = node.attribute(key).as_string();
    require(value == "true" || value == "false", std::string("invalid boolean ") + key);
    return value == "true";
}
void writeValue(lua_State* lua, int index, pugi::xml_node node, unsigned depth, size_t& count,
                std::set<const void*>& tables) {
    require(depth < 32 && ++count < 100000, "Lua state exceeds checkpoint limits");
    if (index < 0)
        index = lua_gettop(lua) + index + 1;
    const auto type = lua_type(lua, index);
    if (type == LUA_TNUMBER) {
        const auto value = lua_tonumber(lua, index);
        require(std::isfinite(value), "non-finite Lua number");
        node.append_attribute("type") = "number";
        node.append_attribute("value").set_value(value);
    } else if (type == LUA_TBOOLEAN) {
        node.append_attribute("type") = "boolean";
        node.append_attribute("value") = bool(lua_toboolean(lua, index));
    } else if (type == LUA_TSTRING) {
        size_t size = 0;
        const auto* value = lua_tolstring(lua, index, &size);
        require(std::string(value, size).find('\0') == std::string::npos, "binary Lua string");
        node.append_attribute("type") = "string";
        node.append_child(pugi::node_pcdata).set_value(value);
    } else if (type == LUA_TTABLE) {
        require(tables.insert(lua_topointer(lua, index)).second,
                "shared or cyclic Lua table is not yet supported");
        node.append_attribute("type") = "table";
        lua_pushnil(lua);
        while (lua_next(lua, index)) {
            const auto keyType = lua_type(lua, -2);
            require(keyType == LUA_TSTRING || keyType == LUA_TNUMBER, "unsupported Lua key");
            auto entry = node.append_child("entry");
            if (keyType == LUA_TSTRING) {
                entry.append_attribute("key") = lua_tostring(lua, -2);
            } else {
                const auto key = lua_tonumber(lua, -2);
                require(std::isfinite(key), "invalid Lua key");
                entry.append_attribute("numeric_key").set_value(key);
            }
            writeValue(lua, -1, entry, depth + 1, count, tables);
            lua_pop(lua, 1);
        }
    } else
        throw std::runtime_error("Checkpoint: unsupported mutable Lua value");
}
void readValue(lua_State* lua, pugi::xml_node node, unsigned depth, size_t& count) {
    require(depth < 32 && ++count < 100000, "Lua state exceeds checkpoint limits");
    const std::string type = node.attribute("type").as_string();
    if (type == "number")
        lua_pushnumber(lua, number(node, "value"));
    else if (type == "boolean")
        lua_pushboolean(lua, boolean(node, "value"));
    else if (type == "string")
        lua_pushstring(lua, node.child_value());
    else if (type == "table") {
        lua_newtable(lua);
        for (auto entry : node.children("entry")) {
            require(bool(entry.attribute("key")) != bool(entry.attribute("numeric_key")),
                    "invalid Lua entry key");
            if (entry.attribute("key"))
                lua_pushstring(lua, entry.attribute("key").value());
            else
                lua_pushnumber(lua, number(entry, "numeric_key"));
            lua_pushvalue(lua, -1);
            lua_rawget(lua, -3);
            require(lua_isnil(lua, -1), "duplicate Lua entry key");
            lua_pop(lua, 1);
            readValue(lua, entry, depth + 1, count);
            lua_rawset(lua, -3);
        }
    } else
        throw std::runtime_error("Checkpoint: unknown Lua value type");
}
void strings(pugi::xml_node parent, const char* name, const auto& values) {
    auto root = parent.append_child(name);
    for (const auto& value : values)
        root.append_child("entry").append_attribute("value") = value.c_str();
}
std::vector<std::string> readStrings(pugi::xml_node parent, const char* name) {
    require(bool(parent.child(name)), std::string("missing ") + name);
    std::vector<std::string> values;
    for (auto entry : parent.child(name).children("entry")) {
        require(bool(entry.attribute("value")), "missing string entry");
        values.emplace_back(entry.attribute("value").value());
    }
    return values;
}
} // namespace
size_t Runtime::activeActivities() const {
    size_t count = 0;
    for (bool active :
         {!activeHob_.empty(), ouijiActive_, !activeMixing_.empty(), hexBonesActive_, pianoActive_,
          windowCleaningActive_, cellarDoorsActive_, ratsActive_, doveCoteActive_, spiritLockActive_,
          bool(reveal_), sailActive_, leechesActive_, blockedWellActive_, apothecaryActive_,
          cemeteryGateActive_, skullsObeyActive_, voodooCircleActive_, voodooActive_})
        count += active;
    return count;
}
bool Runtime::checkpointReady() const {
    if (hobFeedback_.transient())
        return false;
    if (!errors_.empty() || !events.empty() || !navigation_.empty() || !scriptStack_.empty() ||
        layers_.empty() || activeActivities() > 1 || cutsceneActive_ || carriageActive_ ||
        locationHintsOpen_ || inRevealTick_ || revealExitRequested_)
        return false;
    return std::none_of(objects_.begin(), objects_.end(),
                        [](const auto& pair) { return pair.second->scriptAnimation.has_value(); });
}
std::string Runtime::checkpoint() const {
    require(!hobFeedback_.transient(), "HOB feedback transition is active");
    require(errors_.empty() && events.empty() && navigation_.empty() && scriptStack_.empty(),
            "pending events, navigation, script or runtime errors");
    require(!layers_.empty(), "no current location");
    require(activeActivities() <= 1, "multiple active activities");
    require(!cutsceneActive_ && !carriageActive_ && !locationHintsOpen_ && !inRevealTick_ &&
                !revealExitRequested_,
            "transient activity is active");
    pugi::xml_document document;
    auto root = document.append_child("deadtime_checkpoint");
    root.append_attribute("version") = 8;
    root.append_attribute("resources") = fingerprint(assets_).c_str();
    if (!latestSpell_.empty())
        root.append_attribute("latest_spell") = latestSpell_.c_str();
    auto activities = root.append_child("activities");
    activities.append_attribute("ouiji_active") = ouijiActive_;
    activities.append_attribute("active_mixing") = activeMixing_.c_str();
    if (ouiji_)
        ActivityCheckpoints::save(*ouiji_, activities.append_child("ouiji"));
    activities.append_attribute("bones_active") = hexBonesActive_;
    activities.append_attribute("piano_active") = pianoActive_;
    activities.append_attribute("window_active") = windowCleaningActive_;
    if (hexBones_)
        ActivityCheckpoints::save(*hexBones_, activities.append_child("bones"));
    if (piano_)
        ActivityCheckpoints::save(*piano_, activities.append_child("piano"));
    if (windowCleaning_)
        ActivityCheckpoints::save(*windowCleaning_, activities.append_child("window"));
    activities.append_attribute("cellar_active") = cellarDoorsActive_;
    activities.append_attribute("rats_active") = ratsActive_;
    activities.append_attribute("dove_active") = doveCoteActive_;
    if (cellarDoors_)
        ActivityCheckpoints::save(*cellarDoors_, activities.append_child("cellar"));
    if (rats_)
        ActivityCheckpoints::save(*rats_, activities.append_child("rats"));
    if (doveCote_)
        ActivityCheckpoints::save(*doveCote_, activities.append_child("dove"));
    activities.append_attribute("lock_active") = spiritLockActive_;
    if (spiritLock_)
        ActivityCheckpoints::save(*spiritLock_, activities.append_child("lock"));
    if (reveal_)
        ActivityCheckpoints::save(*reveal_, activities.append_child("reveal"));
    activities.append_attribute("sail_active") = sailActive_;
    activities.append_attribute("leeches_active") = leechesActive_;
    if (sail_)
        ActivityCheckpoints::save(*sail_, activities.append_child("sail"));
    if (leeches_)
        ActivityCheckpoints::save(*leeches_, activities.append_child("leeches"));
    activities.append_attribute("well_active") = blockedWellActive_;
    activities.append_attribute("apothecary_active") = apothecaryActive_;
    if (blockedWell_)
        ActivityCheckpoints::save(*blockedWell_, activities.append_child("well"));
    if (apothecary_)
        ActivityCheckpoints::save(*apothecary_, activities.append_child("apothecary"));
    activities.append_attribute("gate_active") = cemeteryGateActive_;
    activities.append_attribute("skulls_active") = skullsObeyActive_;
    activities.append_attribute("circle_active") = voodooCircleActive_;
    activities.append_attribute("doll_active") = voodooActive_;
    if (cemeteryGate_)
        ActivityCheckpoints::save(*cemeteryGate_, activities.append_child("gate"));
    if (skullsObey_)
        ActivityCheckpoints::save(*skullsObey_, activities.append_child("skulls"));
    if (voodooCircle_)
        ActivityCheckpoints::save(*voodooCircle_, activities.append_child("circle"));
    if (voodoo_)
        ActivityCheckpoints::save(*voodoo_, activities.append_child("doll"));
    root.append_attribute("reveal_finished") = revealFinished_;
    auto fades = root.append_child("spirit_fades");
    for (const auto& [object, speed] : spiritFades_) {
        auto entry = fades.append_child("fade");
        entry.append_attribute("id") = object->id.c_str();
        entry.append_attribute("speed") = speed;
    }
    for (const auto& [id, activity] : mixingGames_) {
        auto saved = activities.append_child("mixing");
        saved.append_attribute("id") = id.c_str();
        ActivityCheckpoints::save(*activity, saved);
    }
    root.append_attribute("chapter") = currentChapter_;
    root.append_attribute("hint_charge").set_value(hintBottle_.charge());
    root.append_attribute("active_hob") = activeHob_.c_str();
    root.append_attribute("hint_replay").set_value(static_cast<unsigned long long>(hintReplay_));
    root.append_attribute("hob_tutorial_armed") = hobTutorialArmed_;
    root.append_attribute("hob_tutorial_seconds").set_value(hobTutorialSeconds_);
    auto skip = root.append_child("skip_progress");
    for (const auto& [id, value] : puzzleSkipProgress_) {
        auto entry = skip.append_child("puzzle");
        entry.append_attribute("id") = id.c_str();
        entry.append_attribute("value").set_value(value);
    }
    strings(root, "layers", layers_);
    strings(root, "inventory", inventoryOrder_);
    strings(root, "found", found_);
    strings(root, "flags", flags_);
    strings(root, "enabled_hints", enabledHints_);
    strings(root, "hint_history", hintHistory_);
    auto scenes = root.append_child("scenes");
    for (const auto& [id, scene] : scenes_) {
        auto entry = scenes.append_child("scene");
        entry.append_attribute("id") = id.c_str();
        entry.append_attribute("visited") = scene.visited;
    }
    auto applications = root.append_child("applications");
    for (const auto& [key, path] : anywhereApplications_) {
        auto entry = applications.append_child("application");
        entry.append_attribute("scene") = key.first.c_str();
        entry.append_attribute("item") = key.second.c_str();
        entry.append_attribute("script") = path.c_str();
    }
    auto objects = root.append_child("objects");
    size_t luaCount = 0;
    std::set<const void*> tables;
    Stack stack(lua_);
    for (const auto& [id, pointer] : objects_) {
        const auto& object = *pointer;
        require(!object.scriptAnimation, "scene animation codec not yet available");
        auto entry = objects.append_child("object");
        entry.append_attribute("id") = id.c_str();
        entry.append_attribute("state") = object.state.c_str();
        entry.append_attribute("texture") = object.texture.c_str();
        entry.append_attribute("cursor") = object.cursor.c_str();
#define WRITE(field) entry.append_attribute(#field).set_value(object.field)
        WRITE(x);
        WRITE(y);
        WRITE(z);
        WRITE(width);
        WRITE(height);
        WRITE(alpha);
        WRITE(visible);
        WRITE(interactive);
        WRITE(enabled);
        WRITE(initialized);
        WRITE(treeTime);
        WRITE(treePlaying);
        WRITE(treeLooped);
        WRITE(treePlayOption);
        WRITE(treeLoopOverridden);
        WRITE(particleGenerating);
        WRITE(particlesPresimRequested);
        WRITE(particlePresimPending);
#undef WRITE
        if (object.timer)
            entry.append_attribute("timer").set_value(*object.timer);
        for (const auto& [state, ref] : object.states) {
            auto saved = entry.append_child("lua");
            saved.append_attribute("state") = state.c_str();
            lua_rawgeti(lua_, LUA_REGISTRYINDEX, ref);
            const int table = lua_gettop(lua_);
            lua_pushnil(lua_);
            while (lua_next(lua_, table)) {
                require(lua_type(lua_, -2) == LUA_TSTRING, "unsupported Lua environment key");
                if (!lua_isfunction(lua_, -1)) {
                    auto value = saved.append_child("value");
                    value.append_attribute("key") = lua_tostring(lua_, -2);
                    writeValue(lua_, -1, value, 0, luaCount, tables);
                } else if (object.initialized) {
                    const auto* captured = lua_getupvalue(lua_, -1, 1);
                    if (captured) {
                        lua_pop(lua_, 1);
                        throw std::runtime_error(
                            "Checkpoint: captured Lua callback state is not yet supported");
                    }
                }
                lua_pop(lua_, 1);
            }
            lua_pop(lua_, 1);
        }
    }
    auto rounds = root.append_child("hobs");
    for (const auto& [key, round] : hobRounds_) {
        auto saved = rounds.append_child("hob");
        saved.append_attribute("key") = key.c_str();
        saved.append_attribute("rewarded") = round->rewarded;
        for (auto index : round->targets)
            saved.append_child("target").append_attribute("index").set_value(
                static_cast<unsigned long long>(index));
        for (size_t i = 0; i < round->items.size(); ++i)
            for (size_t j = 0; j < round->items[i].pieces.size(); ++j)
                if (round->items[i].pieces[j].found) {
                    auto piece = saved.append_child("found");
                    piece.append_attribute("item").set_value(static_cast<unsigned long long>(i));
                    piece.append_attribute("piece").set_value(static_cast<unsigned long long>(j));
                }
    }
    std::ostringstream output;
    document.save(output, "  ", pugi::format_default, pugi::encoding_utf8);
    return output.str();
}
std::unique_ptr<Runtime> Runtime::fromCheckpoint(Assets& assets, const std::string& bytes) {
    require(bytes.size() <= 32 * 1024 * 1024, "file too large");
    pugi::xml_document document;
    require(bool(document.load_buffer(bytes.data(), bytes.size(),
                                      pugi::parse_default | pugi::parse_ws_pcdata_single)),
            "invalid XML");
    const auto root = document.child("deadtime_checkpoint");
    require(bool(root) && std::string(root.attribute("version").as_string()) == "8", "unsupported format");
    require(root.attribute("resources").as_string() == fingerprint(assets), "gameplay resources differ");
    auto runtime = std::make_unique<Runtime>(assets);
    runtime->load();
    require(runtime->errors_.empty(), "world compilation failed");
    auto integer = [&](pugi::xml_node node, const char* key, size_t maximum) {
        const double value = number(node, key);
        require(value >= 0 && std::trunc(value) == value && value <= double(maximum),
                std::string("invalid index ") + key);
        return size_t(value);
    };
    runtime->revealFinished_ = boolean(root, "reveal_finished");
    runtime->currentChapter_ = int(integer(root, "chapter", std::numeric_limits<int>::max()));
    runtime->hintBottle_.restoreCharge(number(root, "hint_charge"));
    runtime->layers_ = readStrings(root, "layers");
    require(!runtime->layers_.empty(), "no current location");
    std::set<std::string> seen;
    for (const auto& id : runtime->layers_)
        require(runtime->findScene(id) && seen.insert(id).second, "invalid location stack");
    auto readSet = [&](const char* name, auto& target) {
        for (auto& value : readStrings(root, name))
            require(target.insert(value).second, std::string("duplicate ") + name);
    };
    readSet("found", runtime->found_);
    readSet("flags", runtime->flags_);
    readSet("enabled_hints", runtime->enabledHints_);
    if (const auto latest = root.attribute("latest_spell")) {
        runtime->latestSpell_ = latest.value();
        require(!runtime->latestSpell_.empty() && runtime->flag("enableSpell:" + runtime->latestSpell_),
                "latest spell is not learned");
    }
    runtime->inventoryOrder_ = readStrings(root, "inventory");
    for (const auto& item : runtime->inventoryOrder_)
        require(runtime->found_.contains(item) && runtime->inventory_.insert(item).second,
                "invalid inventory history/order");
    runtime->hintHistory_ = readStrings(root, "hint_history");
    runtime->hintReplay_ = integer(root, "hint_replay", runtime->hintHistory_.size());
    seen.clear();
    require(bool(root.child("scenes")), "missing scenes");
    for (auto entry : root.child("scenes").children("scene")) {
        auto* scene = runtime->findScene(entry.attribute("id").as_string());
        require(scene && seen.insert(scene->id).second, "unknown or duplicate scene");
        scene->visited = boolean(entry, "visited");
    }
    require(seen.size() == runtime->scenes_.size(), "incomplete scenes");
    require(bool(root.child("applications")), "missing anywhere applications");
    for (auto entry : root.child("applications").children("application")) {
        const std::string scene = entry.attribute("scene").as_string(),
                          item = entry.attribute("item").as_string(),
                          script = entry.attribute("script").as_string();
        require(runtime->findScene(scene) && !item.empty() && assets.contains(script),
                "invalid anywhere application");
        require(runtime->anywhereApplications_.emplace(std::make_pair(scene, item), script).second,
                "duplicate anywhere application");
    }
    seen.clear();
    size_t luaCount = 0;
    Stack stack(runtime->lua_);
    auto* lua = runtime->lua_;
    require(bool(root.child("objects")), "missing objects");
    for (auto entry : root.child("objects").children("object")) {
        auto* object = runtime->findObject(entry.attribute("id").as_string());
        require(object && seen.insert(object->id).second, "unknown or duplicate object");
        object->state = entry.attribute("state").as_string();
        object->initialized = boolean(entry, "initialized");
        require((!object->initialized && object->state.empty()) ||
                    std::any_of(object->states.begin(), object->states.end(),
                                [&](const auto& state) { return state.first == object->state; }),
                "unknown object state");
        object->texture = entry.attribute("texture").as_string();
        object->cursor = entry.attribute("cursor").as_string();
#define FLOAT(field)                                                                                         \
    {                                                                                                        \
        const auto value = number(entry, #field);                                                            \
        require(std::abs(value) <= std::numeric_limits<float>::max(), "float overflow");                     \
        object->field = float(value);                                                                        \
    }
        FLOAT(x);
        FLOAT(y);
        FLOAT(z);
        FLOAT(width);
        FLOAT(height);
        FLOAT(alpha);
#undef FLOAT
        require(object->width >= 0 && object->height >= 0 && object->alpha >= 0 && object->alpha <= 1,
                "invalid object dimensions/alpha");
#define BOOL(field) object->field = boolean(entry, #field)
        BOOL(visible);
        BOOL(interactive);
        BOOL(enabled);
        BOOL(treePlaying);
        BOOL(treeLooped);
        BOOL(treePlayOption);
        BOOL(treeLoopOverridden);
        BOOL(particleGenerating);
        BOOL(particlesPresimRequested);
#undef BOOL
        object->treeTime = number(entry, "treeTime");
        object->particlePresimPending = number(entry, "particlePresimPending");
        require(object->treeTime >= 0 && object->particlePresimPending >= 0, "invalid visual time");
        if (entry.attribute("timer")) {
            object->timer = number(entry, "timer");
            require(*object->timer >= 0, "negative timer");
        }
        std::set<std::string> states;
        for (auto environment : entry.children("lua")) {
            const std::string state = environment.attribute("state").as_string();
            const auto found = std::find_if(object->states.begin(), object->states.end(),
                                            [&](const auto& candidate) { return candidate.first == state; });
            require(found != object->states.end() && states.insert(state).second,
                    "unknown or duplicate Lua state");
            lua_rawgeti(lua, LUA_REGISTRYINDEX, found->second);
            const int table = lua_gettop(lua);
            lua_pushnil(lua);
            while (lua_next(lua, table)) {
                if (!lua_isfunction(lua, -1)) {
                    lua_pop(lua, 1);
                    lua_pushvalue(lua, -1);
                    lua_pushnil(lua);
                    lua_rawset(lua, table);
                } else
                    lua_pop(lua, 1);
            }
            std::set<std::string> keys;
            for (auto saved : environment.children("value")) {
                require(bool(saved.attribute("key")), "missing Lua environment key");
                const std::string key = saved.attribute("key").value();
                require(keys.insert(key).second, "duplicate Lua environment key");
                lua_pushlstring(lua, key.data(), key.size());
                lua_rawget(lua, table);
                require(!lua_isfunction(lua, -1), "attempt to overwrite Lua callback");
                lua_pop(lua, 1);
                lua_pushlstring(lua, key.data(), key.size());
                readValue(lua, saved, 0, luaCount);
                lua_rawset(lua, table);
            }
            lua_pop(lua, 1);
        }
        require(states.size() == object->states.size(), "incomplete Lua environments");
    }
    require(seen.size() == runtime->objects_.size(), "incomplete objects");
    require(bool(root.child("hobs")), "missing HOB state");
    for (auto saved : root.child("hobs").children("hob")) {
        const std::string key = saved.attribute("key").as_string();
        const auto separator = key.find_last_of(':');
        require(separator != std::string::npos && !runtime->hobRounds_.contains(key), "invalid HOB key");
        auto round = std::make_unique<HiddenObjectRound>(assets, key.substr(0, separator),
                                                         key.substr(separator + 1), 0);
        const auto expected = round->targets.size();
        round->targets.clear();
        for (auto& item : round->items)
            item.target = false;
        for (auto target : saved.children("target")) {
            const auto index = integer(target, "index", round->items.size() - 1);
            require(!round->items[index].target, "duplicate HOB target");
            round->targets.push_back(index);
            round->items[index].target = true;
        }
        require(round->targets.size() == expected, "wrong HOB target count");
        for (auto piece : saved.children("found")) {
            const auto item = integer(piece, "item", round->items.size() - 1);
            const auto part = integer(piece, "piece", round->items[item].pieces.size() - 1);
            require(round->items[item].target && !round->items[item].pieces[part].found,
                    "invalid found HOB piece");
            round->items[item].pieces[part].found = true;
        }
        round->rewarded = boolean(saved, "rewarded");
        require(!round->rewarded || round->complete(), "rewarded HOB has missing targets");
        runtime->hobRounds_.emplace(key, std::move(round));
    }
    runtime->activeHob_ = root.attribute("active_hob").as_string();
    require(runtime->activeHob_.empty() ||
                (runtime->hobRounds_.contains(runtime->activeHob_) && !runtime->hob()->complete()),
            "invalid active HOB");
    const bool hasArmed = bool(root.attribute("hob_tutorial_armed")),
               hasAge = bool(root.attribute("hob_tutorial_seconds"));
    require(hasArmed == hasAge, "incomplete HOB tutorial timer");
    runtime->hobTutorialArmed_ =
        hasArmed ? boolean(root, "hob_tutorial_armed") : !runtime->activeHob_.empty();
    runtime->hobTutorialSeconds_ = hasAge ? number(root, "hob_tutorial_seconds") : 0;
    require(runtime->hobTutorialSeconds_ >= 0 && runtime->hobTutorialSeconds_ <= 10 &&
                (runtime->hobTutorialArmed_ || runtime->hobTutorialSeconds_ == 0) &&
                (!runtime->activeHob_.empty() || !runtime->hobTutorialArmed_),
            "invalid HOB tutorial timer");
    const auto activities = root.child("activities");
    require(bool(activities), "missing activity states");
    if (auto saved = activities.child("ouiji")) {
        require(!saved.next_sibling("ouiji"), "duplicate spirit board");
        runtime->ouiji_ = std::make_unique<OuijiBoard>(assets);
        ActivityCheckpoints::load(*runtime->ouiji_, saved);
    }
    runtime->ouijiActive_ = boolean(activities, "ouiji_active");
    require(!runtime->ouijiActive_ || runtime->ouiji_, "active spirit board missing");
    if (auto saved = activities.child("bones")) {
        require(!saved.next_sibling("bones"), "duplicate bones");
        runtime->hexBones_ = std::make_unique<HexBones>(assets, "minigames/hex_bones", 0);
        ActivityCheckpoints::load(*runtime->hexBones_, saved);
    }
    if (auto saved = activities.child("piano")) {
        require(!saved.next_sibling("piano"), "duplicate piano");
        runtime->piano_ = std::make_unique<Piano>(assets, "minigames/piano");
        ActivityCheckpoints::load(*runtime->piano_, saved);
    }
    if (auto saved = activities.child("window")) {
        require(!saved.next_sibling("window"), "duplicate window");
        runtime->windowCleaning_ = std::make_unique<WindowCleaning>(assets, "minigames/window_cleaning", 0);
        ActivityCheckpoints::load(*runtime->windowCleaning_, saved);
    }
    runtime->hexBonesActive_ = boolean(activities, "bones_active");
    runtime->pianoActive_ = boolean(activities, "piano_active");
    runtime->windowCleaningActive_ = boolean(activities, "window_active");
    require((!runtime->hexBonesActive_ || runtime->hexBones_) &&
                (!runtime->pianoActive_ || runtime->piano_) &&
                (!runtime->windowCleaningActive_ || runtime->windowCleaning_),
            "active chapter-two model missing");
    if (auto saved = activities.child("cellar")) {
        require(!saved.next_sibling("cellar"), "duplicate cellar");
        runtime->cellarDoors_ = std::make_unique<CellarDoors>(assets, "minigames/cellar_doors", 0);
        ActivityCheckpoints::load(*runtime->cellarDoors_, saved);
    }
    if (auto saved = activities.child("rats")) {
        require(!saved.next_sibling("rats"), "duplicate rats");
        runtime->rats_ = std::make_unique<Rats>(assets, "minigames/rats");
        ActivityCheckpoints::load(*runtime->rats_, saved);
    }
    if (auto saved = activities.child("dove")) {
        require(!saved.next_sibling("dove"), "duplicate dove");
        runtime->doveCote_ = std::make_unique<DoveCote>(assets, "minigames/dove_cote", 0);
        ActivityCheckpoints::load(*runtime->doveCote_, saved);
    }
    runtime->cellarDoorsActive_ = boolean(activities, "cellar_active");
    runtime->ratsActive_ = boolean(activities, "rats_active");
    runtime->doveCoteActive_ = boolean(activities, "dove_active");
    require((!runtime->cellarDoorsActive_ || runtime->cellarDoors_) &&
                (!runtime->ratsActive_ || runtime->rats_) &&
                (!runtime->doveCoteActive_ || runtime->doveCote_),
            "active chapter-three model missing");
    if (auto saved = activities.child("lock")) {
        require(!saved.next_sibling("lock"), "duplicate spirit lock");
        runtime->spiritLock_ = std::make_unique<SpiritLock>(assets, "minigames/spirit_lock");
        ActivityCheckpoints::load(*runtime->spiritLock_, saved);
    }
    runtime->spiritLockActive_ = boolean(activities, "lock_active");
    require(!runtime->spiritLockActive_ || runtime->spiritLock_, "active spirit lock missing");
    if (auto saved = activities.child("reveal")) {
        require(!saved.next_sibling("reveal"), "duplicate reveal mode");
        const auto* scene = runtime->findScene(runtime->layers_.back());
        auto local = [&](const char* id) {
            auto* object = runtime->findObject(id);
            require(object && std::find(scene->objects.begin(), scene->objects.end(), object) !=
                                  scene->objects.end(),
                    "reveal object outside current scene");
            return object;
        };
        const std::string brush = saved.attribute("brush").as_string();
        require(!brush.empty() && runtime->hasInventory(brush), "missing reveal brush");
        auto* container = local(saved.attribute("container").as_string());
        std::vector<RevealSpirits::Target> targets;
        for (auto entry : saved.children("target")) {
            auto* spirit = local(entry.attribute("spirit").as_string());
            auto* mask = local(entry.attribute("mask").as_string());
            require(mask->id == spirit->id + "_mask" && spirit != container, "invalid reveal target binding");
            targets.push_back({spirit, mask});
        }
        runtime->reveal_ = std::make_unique<RevealSpirits>(assets, std::move(targets), brush, *container);
        ActivityCheckpoints::load(*runtime->reveal_, saved);
    }
    if (auto saved = activities.child("sail")) {
        require(!saved.next_sibling("sail"), "duplicate sail");
        runtime->sail_ = std::make_unique<Sail>(assets, "minigames/sail", 0);
        ActivityCheckpoints::load(*runtime->sail_, saved);
    }
    if (auto saved = activities.child("leeches")) {
        require(!saved.next_sibling("leeches"), "duplicate leeches");
        runtime->leeches_ = std::make_unique<Leeches>(assets, "minigames/leeches", 0);
        ActivityCheckpoints::load(*runtime->leeches_, saved);
        require(!runtime->leeches_->holding() || runtime->hasInventory(runtime->leeches_->heldOrange()),
                "captured leech lacks original orange");
    }
    runtime->sailActive_ = boolean(activities, "sail_active");
    runtime->leechesActive_ = boolean(activities, "leeches_active");
    require((!runtime->sailActive_ || runtime->sail_) && (!runtime->leechesActive_ || runtime->leeches_),
            "active chapter-four model missing");
    if (auto saved = activities.child("well")) {
        require(!saved.next_sibling("well"), "duplicate well");
        runtime->blockedWell_ = std::make_unique<BlockedWell>(assets, "minigames/blocked_well");
        ActivityCheckpoints::load(*runtime->blockedWell_, saved);
    }
    if (auto saved = activities.child("apothecary")) {
        require(!saved.next_sibling("apothecary"), "duplicate apothecary");
        runtime->apothecary_ = std::make_unique<Apothecary>(assets, "minigames/sort_apothecary_items", 0);
        ActivityCheckpoints::load(*runtime->apothecary_, saved);
    }
    runtime->blockedWellActive_ = boolean(activities, "well_active");
    runtime->apothecaryActive_ = boolean(activities, "apothecary_active");
    require((!runtime->blockedWellActive_ || runtime->blockedWell_) &&
                (!runtime->apothecaryActive_ || runtime->apothecary_),
            "active chapter-five model missing");
    if (auto saved = activities.child("gate")) {
        require(!saved.next_sibling("gate"), "duplicate gate");
        runtime->cemeteryGate_ = std::make_unique<CemeteryGate>(assets, "minigames/cemetery_gate", 0);
        ActivityCheckpoints::load(*runtime->cemeteryGate_, saved);
    }
    if (auto saved = activities.child("skulls")) {
        require(!saved.next_sibling("skulls"), "duplicate skulls");
        runtime->skullsObey_ = std::make_unique<SkullsObey>(assets, "minigames/skulls_obey");
        ActivityCheckpoints::load(*runtime->skullsObey_, saved);
    }
    if (auto saved = activities.child("circle")) {
        require(!saved.next_sibling("circle"), "duplicate circle");
        runtime->voodooCircle_ = std::make_unique<VoodooCircle>(assets, "minigames/voodoo_circle");
        ActivityCheckpoints::load(*runtime->voodooCircle_, saved);
    }
    if (auto saved = activities.child("doll")) {
        require(!saved.next_sibling("doll"), "duplicate doll");
        runtime->voodoo_ = std::make_unique<VoodooDoll>(assets, "minigames/_spellmixing_voodoo_doll");
        ActivityCheckpoints::load(*runtime->voodoo_, saved);
    }
    runtime->cemeteryGateActive_ = boolean(activities, "gate_active");
    runtime->skullsObeyActive_ = boolean(activities, "skulls_active");
    runtime->voodooCircleActive_ = boolean(activities, "circle_active");
    runtime->voodooActive_ = boolean(activities, "doll_active");
    require((!runtime->cemeteryGateActive_ || runtime->cemeteryGate_) &&
                (!runtime->skullsObeyActive_ || runtime->skullsObey_) &&
                (!runtime->voodooCircleActive_ || runtime->voodooCircle_) &&
                (!runtime->voodooActive_ || runtime->voodoo_),
            "active chapter-six model missing");
    require(bool(root.child("spirit_fades")), "missing spirit fade state");
    seen.clear();
    double vanishSpeed = 0;
    if (root.child("spirit_fades").child("fade")) {
        pugi::xml_document revealConfig;
        const auto config = assets.read("minigames/reveal_spirits/config.xml");
        require(bool(revealConfig.load_buffer(config.data(), config.size())), "invalid reveal config");
        vanishSpeed = revealConfig.child("reveal_spirits")
                          .child("vanish_speed")
                          .attribute("alpha_per_second")
                          .as_double();
    }
    for (auto entry : root.child("spirit_fades").children("fade")) {
        auto* object = runtime->findObject(entry.attribute("id").as_string());
        const auto speed = number(entry, "speed");
        require(object && seen.insert(object->id).second && object->alpha > 0 && object->alpha < 1 &&
                    speed > 0 && speed == vanishSpeed,
                "invalid spirit fade");
        if (runtime->reveal_)
            for (const auto& target : runtime->reveal_->targets())
                require(target.spirit != object, "active spirit is fading outside mode");
        runtime->spiritFades_.emplace_back(object, speed);
    }
    std::set<std::string> mixing;
    for (auto saved : activities.children("mixing")) {
        const std::string id = saved.attribute("id").as_string();
        require(mixing.insert(id).second, "duplicate mixing activity");
        ActivityCheckpoints::load(runtime->ensureMixing(id), saved);
    }
    runtime->activeMixing_ = activities.attribute("active_mixing").as_string();
    require(bool(activities.attribute("active_mixing")), "missing active mixing state");
    require(runtime->activeMixing_.empty() ||
                (runtime->mixingGames_.contains(runtime->activeMixing_) && !runtime->mixing()->complete()),
            "invalid active mixing activity");
    require(runtime->activeActivities() <= 1, "multiple active activities");
    if (auto skip = root.child("skip_progress")) {
        require(!skip.next_sibling("skip_progress"), "duplicate skip progress");
        for (auto entry : skip.children("puzzle")) {
            const std::string id = entry.attribute("id").as_string();
            const auto value = number(entry, "value");
            require(runtime->hasSkipPuzzle(id), "skip progress has no matching puzzle");
            require(value >= 0 && value <= 1 && runtime->puzzleSkipProgress_.emplace(id, value).second,
                    "invalid or duplicate skip progress");
        }
    }
    // Restoring fields is deliberately side-effect free: do not rerun entry,
    // reward or first-visit Lua while loading a saved world.
    return runtime;
}
} // namespace deadtime
