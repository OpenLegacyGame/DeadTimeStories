#include "player.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        std::filesystem::path data;
        std::string scene = "jessie_home.front_gate";
        std::string hob, minigame, cutscene, round = "round0";
        double at = 0;
        std::filesystem::path profileFile, userDirectory;
        bool inspection = false;
        bool audit = false;
        deadtime::PlayerOptions options;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            auto next = [&]() -> std::string {
                if (++i >= argc)
                    throw std::runtime_error("Missing value for " + arg);
                return argv[i];
            };
            if (arg == "--assets")
                data = next();
            else if (arg == "--scene") {
                scene = next();
                inspection = true;
            } else if (arg == "--hob") {
                hob = next();
                inspection = true;
            } else if (arg == "--minigame") {
                minigame = next();
                inspection = true;
            } else if (arg == "--cutscene") {
                cutscene = next();
                inspection = true;
            } else if (arg == "--at")
                at = std::stod(next());
            else if (arg == "--round")
                round = next();
            else if (arg == "--audit") {
                audit = true;
                inspection = true;
            } else if (arg == "--snapshot")
                options.snapshot = next();
            else if (arg == "--frames")
                options.frames = std::stoi(next());
            else if (arg == "--no-audio")
                options.audio = false;
            else if (arg == "--menu") {
                options.menu = true;
                inspection = true;
            } else if (arg == "--credits") {
                options.credits = true;
                inspection = true;
            } else if (arg == "--profile-menu") {
                profileFile = next();
                options.profileMenu = true;
                inspection = true;
            } else if (arg == "--user-data")
                userDirectory = next();
            else if (arg == "--help") {
                std::cout
                    << "deadtime --assets DIRECTORY [--scene ID | --hob PATH --round ID | --minigame NAME | "
                       "--cutscene NAME --at SECONDS | --menu | --credits | --profile-menu FILE] "
                       "[--user-data DIRECTORY] [--audit] [--frames N] [--snapshot FILE] [--no-audio]\n"
                       "Reads editable resources/localization directories or packaged "
                       "resources.pak/localization.pak.\n";
                return 0;
            } else
                throw std::runtime_error("Unknown option: " + arg);
        }
        if (data.empty()) {
            char* base = SDL_GetBasePath();
            if (!base)
                throw std::runtime_error("Cannot determine application directory");
            const auto executableDirectory =
                std::filesystem::path(std::u8string(base, base + std::strlen(base)));
            SDL_free(base);
            for (const auto& candidate : {executableDirectory / "data", executableDirectory,
#ifdef __APPLE__
                                          executableDirectory / "../Resources/data",
#endif

                                          std::filesystem::current_path() / "assets"}) {
                if (deadtime::Assets::isGameRoot(candidate)) {
                    data = candidate;
                    break;
                }
            }
            if (data.empty())
                throw std::runtime_error(
                    "Game data is missing. Run from the source root, place packaged assets in data beside "
                    "deadtime.exe, or specify --assets DIRECTORY.");
        }
        if (options.frames < 0)
            throw std::runtime_error("Frame count must not be negative");
        if (!std::isfinite(at) || at < 0 || (at > 0 && cutscene.empty()))
            throw std::runtime_error("--at requires a cutscene and a nonnegative finite time");
        deadtime::Assets assets;
        assets.mountGame(data);
        if (inspection && !userDirectory.empty())
            throw std::runtime_error("--user-data is for normal game startup; omit inspection mode options");
        std::unique_ptr<deadtime::Profiles> profiles;
        std::unique_ptr<deadtime::GameSaves> saves;
        std::unique_ptr<deadtime::Settings> settings;
        if (!inspection) {
            if (userDirectory.empty()) {
                char* directory = SDL_GetPrefPath("OpenLegacyGame", "DeadTimeStories");
                if (!directory)
                    throw std::runtime_error("Cannot find user data directory");
                userDirectory =
                    std::filesystem::path(std::u8string(directory, directory + std::strlen(directory)));
                SDL_free(directory);
            }
            profiles = std::make_unique<deadtime::Profiles>(assets, userDirectory / "users.xml");
            saves = std::make_unique<deadtime::GameSaves>(userDirectory / "progress");
            settings = std::make_unique<deadtime::Settings>(userDirectory / "settings.xml");
        } else if (!profileFile.empty())
            profiles = std::make_unique<deadtime::Profiles>(assets, profileFile);
        bool enterNewWorld = false;
        for (;;) {
            std::unique_ptr<deadtime::Runtime> world;
            if (saves && profiles->current() && !enterNewWorld)
                if (auto bytes = saves->load(profiles->ids().at(*profiles->current())))
                    world = deadtime::Runtime::fromCheckpoint(assets, *bytes);
            if (!world) {
                world = std::make_unique<deadtime::Runtime>(assets);
                world->load();
            }
            auto& runtime = *world;
            if (saves) {
                options.credits = false;
                options.profileMenu = profiles->names().empty();
                options.menu = !options.profileMenu;
                if (enterNewWorld) {
                    options.menu = options.profileMenu = false;
                    runtime.enter("jessie_home.front_gate");
                    enterNewWorld = false;
                }
            } else if (options.menu || options.credits || options.profileMenu) {
                // Inspect the original front end without entering a world scene.
            } else if (!cutscene.empty()) {
                runtime.openCutscene(cutscene);
                if (at > 0)
                    runtime.tick(at);
            } else if (!minigame.empty()) {
                if (minigame == "ouiji_board")
                    runtime.run("gotoMinigameOuijiBoard()", "inspection");
                else if (minigame == "banish_spirits")
                    runtime.run("gotoMinigameBanishSpirits()", "inspection");
                else if (minigame == "graveyard_grease")
                    runtime.run("gotoMinigameGraveyardGrease()", "inspection");
                else if (minigame == "spirit_revealing_mixture")
                    runtime.run("gotoMinigameSpiritRevealingMixture()", "inspection");
                else if (minigame == "aconite_antidote")
                    runtime.run("gotoMinigameAconiteAntidote()", "inspection");
                else if (minigame == "window_cleaning")
                    runtime.run("gotoMinigameWindowCleaning()", "inspection");
                else if (minigame == "voodoo_doll")
                    runtime.run("gotoMinigameVoodooDoll()", "inspection");
                else if (minigame == "hex_bones")
                    runtime.run("gotoMinigameHexBones()", "inspection");
                else if (minigame == "piano")
                    runtime.run("gotoMinigamePiano()", "inspection");
                else if (minigame == "dove_cote")
                    runtime.run("gotoMinigameDoveCote()", "inspection");
                else if (minigame == "cellar_doors")
                    runtime.run("gotoMinigameCellarDoors()", "inspection");
                else if (minigame == "rats")
                    runtime.run("gotoMinigameRats()", "inspection");
                else if (minigame == "spirit_lock")
                    runtime.run("gotoMinigameSpiritLock()", "inspection");
                else if (minigame == "blocked_well") {
                    runtime.enter("french_quarter.street_2");
                    runtime.run("gotoMinigameBlockedWell()", "inspection");
                } else if (minigame == "sort_apothecary_items")
                    runtime.run("gotoMinigameSortApothecaryItems()", "inspection");
                else if (minigame == "sail") {
                    runtime.enter("cajun.mangrove_circle");
                    runtime.events.clear();
                    runtime.run("gotoMinigameSail()", "inspection");
                } else if (minigame == "leeches") {
                    runtime.enter("cajun.swamp_island");
                    runtime.events.clear();
                    runtime.run("gotoMinigameLeeches()", "inspection");
                } else if (minigame == "skulls_obey") {
                    runtime.enter("cemetery.voodoo_shop");
                    runtime.events.clear();
                    runtime.run("gotoMinigameSkullsObey()", "inspection");
                } else if (minigame == "cemetery_gate") {
                    runtime.enter("cemetery.gates");
                    runtime.events.clear();
                    runtime.run("gotoMinigameCemeteryGate()", "inspection");
                } else if (minigame == "voodoo_circle") {
                    runtime.enter("cemetery.gates");
                    runtime.events.clear();
                    runtime.run("gotoMinigameVoodooCircle()", "inspection");
                } else
                    throw std::runtime_error("Unsupported inspection minigame: " + minigame);
            } else if (hob.empty())
                runtime.enter(scene);
            else
                runtime.openHob(hob, round);
            std::cout << "Loaded " << runtime.scenes().size() << " scenes, " << runtime.objects().size()
                      << " objects.\n";
            if (audit) {
                size_t states = 0;
                for (const auto& [_, o] : runtime.objects())
                    states += o->states.size();
                std::cout << "Compiled " << states << " state sections; " << runtime.errors().size()
                          << " startup errors.\n";
                return runtime.errors().empty() ? 0 : 2;
            }
            deadtime::Player player(assets, runtime, profiles.get(), saves.get(), settings.get());
            const auto result = player.run(options);
            if (result == deadtime::Player::ReloadProfile && saves)
                continue;
            if (result != deadtime::Player::RestartGame)
                return result;
            if (saves) {
                if (!profiles->current())
                    throw std::runtime_error("Cannot start a game without a user");
                saves->newGame(profiles->ids().at(*profiles->current()));
                enterNewWorld = true;
                continue;
            }
            // Destroy the old player and Lua world before a confirmed new game.
            options.menu = options.credits = options.profileMenu = false;
            cutscene.clear();
            minigame.clear();
            hob.clear();
            scene = "jessie_home.front_gate";
            at = 0;
        }
    } catch (const std::exception& e) {
        std::cerr << "deadtime: " << e.what() << '\n';
        return 1;
    }
}
