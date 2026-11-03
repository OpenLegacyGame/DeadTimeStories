# Editable source assets are the default. A legacy installation can still be
# supplied explicitly for compatibility comparisons, without becoming a source dependency.
set(DEADTIME_ASSET_DIR "${PROJECT_SOURCE_DIR}/assets" CACHE PATH "Editable game input tree")
set(DEADTIME_DATA_DIR "" CACHE PATH "Optional prebuilt legacy PAK directory")
if(DEADTIME_DATA_DIR)
  set(DEADTIME_PACKAGED_DATA_DIR "${DEADTIME_DATA_DIR}")
  foreach(layer resources localization)
    if(NOT EXISTS "${DEADTIME_DATA_DIR}/${layer}.pak")
      message(FATAL_ERROR "Missing legacy game data: ${DEADTIME_DATA_DIR}/${layer}.pak")
    endif()
  endforeach()
else()
  set(DEADTIME_PACKAGED_DATA_DIR "${PROJECT_BINARY_DIR}/data")
  set(game_archives)
  foreach(layer resources localization)
    if(NOT IS_DIRECTORY "${DEADTIME_ASSET_DIR}/${layer}")
      message(FATAL_ERROR "Missing source assets: ${DEADTIME_ASSET_DIR}/${layer}")
    endif()
    file(GLOB_RECURSE layer_inputs CONFIGURE_DEPENDS "${DEADTIME_ASSET_DIR}/${layer}/*")
    # Track removals as well as additions/edits (also with Makefile generators).
    file(GENERATE OUTPUT "${PROJECT_BINARY_DIR}/${layer}-inputs.txt" CONTENT "${layer_inputs}\n")
    set(archive "${DEADTIME_PACKAGED_DATA_DIR}/${layer}.pak")
    add_custom_command(OUTPUT "${archive}"
      COMMAND "${CMAKE_COMMAND}" "-DINPUT_ROOT=${DEADTIME_ASSET_DIR}/${layer}"
              "-DOUTPUT_ARCHIVE=${archive}" -P "${PROJECT_SOURCE_DIR}/tools/PackGameData.cmake"
      DEPENDS ${layer_inputs} "${PROJECT_BINARY_DIR}/${layer}-inputs.txt" "${PROJECT_SOURCE_DIR}/tools/PackGameData.cmake"
      WORKING_DIRECTORY "${DEADTIME_ASSET_DIR}/${layer}"
      COMMENT "Packing editable ${layer} inputs" VERBATIM)
    list(APPEND game_archives "${archive}")
  endforeach()
  add_custom_target(game_data DEPENDS ${game_archives})
  add_dependencies(deadtime game_data)
endif()
