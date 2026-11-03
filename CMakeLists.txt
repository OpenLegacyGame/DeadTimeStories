cmake_minimum_required(VERSION 3.24)
project(DeadTimeStories VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZIP REQUIRED IMPORTED_TARGET libzip)
pkg_check_modules(XML REQUIRED IMPORTED_TARGET pugixml)
pkg_check_modules(LUA REQUIRED IMPORTED_TARGET luajit)
pkg_check_modules(SDL REQUIRED IMPORTED_TARGET sdl2 SDL2_image SDL2_ttf SDL2_mixer)
# Data, Lua world state and game activities are independent of SDL.
add_library(deadtime_core
  src/activity_checkpoints.cpp
  src/activity_layout.cpp
  src/animation.cpp
  src/animation_set.cpp
  src/apothecary.cpp
  src/assets.cpp
  src/audio_timing.cpp
  src/blocked_well.cpp
  src/carriage.cpp
  src/cellar_doors.cpp
  src/cemetery_gate.cpp
  src/checkpoint.cpp
  src/collada.cpp
  src/credits.cpp
  src/cutscene.cpp
  src/dialog.cpp
  src/dove_cote.cpp
  src/game_saves.cpp
  src/gui.cpp
  src/hex_bones.cpp
  src/hint_bottle.cpp
  src/hob.cpp
  src/hob_feedback.cpp
  src/inventory_feedback.cpp
  src/inventory_panel.cpp
  src/leeches.cpp
  src/location_hints.cpp
  src/menu_scene.cpp
  src/options_menu.cpp
  src/ouiji.cpp
  src/particles.cpp
  src/piano.cpp
  src/plantbook.cpp
  src/profiles.cpp
  src/rats.cpp
  src/reveal_spirits.cpp
  src/runtime.cpp
  src/runtime_skip.cpp
  src/sail.cpp
  src/scene_particles.cpp
  src/settings.cpp
  src/skulls_obey.cpp
  src/spellbook.cpp
  src/spellmixing.cpp
  src/spirit_lock.cpp
  src/tutorials.cpp
  src/voodoo.cpp
  src/voodoo_circle.cpp
  src/window_cleaning.cpp
)
target_include_directories(deadtime_core PUBLIC src)
target_link_libraries(deadtime_core PUBLIC PkgConfig::ZIP PkgConfig::XML PkgConfig::LUA)

# SDL presentation, input, menus and audio adapt the core to the desktop.
add_library(deadtime_player
  src/player.cpp
  src/player_menu.cpp
  src/player_skip.cpp
)
target_link_libraries(deadtime_player PUBLIC deadtime_core PkgConfig::SDL)

foreach(target deadtime_core deadtime_player)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /utf-8)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endforeach()

add_executable(deadtime src/main.cpp)
target_link_libraries(deadtime PRIVATE deadtime_player)
if(WIN32)
  enable_language(RC)
  set(DEADTIME_WINDOWS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/windows/deadtime.ico")
  configure_file(assets/windows/deadtime.rc.in deadtime.rc @ONLY)
  set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/deadtime.rc" PROPERTIES
    OBJECT_DEPENDS "${DEADTIME_WINDOWS_ICON}")
  target_sources(deadtime PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/deadtime.rc")
  set_target_properties(deadtime PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
include(cmake/GameData.cmake)
option(DEADTIME_WINDOWS_BUNDLE "Stage game data and Windows runtime libraries" OFF)
if(DEADTIME_WINDOWS_BUNDLE)
  include(cmake/WindowsBundle.cmake)
else()
  install(TARGETS deadtime RUNTIME DESTINATION .)
endif()
