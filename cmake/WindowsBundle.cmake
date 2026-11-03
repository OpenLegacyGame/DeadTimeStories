if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "Windows bundles require a 64-bit Windows build")
endif()
if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()
get_filename_component(runtime_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
install(TARGETS deadtime RUNTIME_DEPENDENCY_SET deadtime_dependencies RUNTIME DESTINATION .)
install(RUNTIME_DEPENDENCY_SET deadtime_dependencies
  DIRECTORIES "${runtime_bin}"
  PRE_EXCLUDE_REGEXES "[Aa][Pp][Ii]-[Mm][Ss]-" "[Ee][Xx][Tt]-[Mm][Ss]-"
  POST_EXCLUDE_REGEXES [[.*[/\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\].*]]
  RUNTIME DESTINATION . LIBRARY DESTINATION .)
install(FILES "${DEADTIME_PACKAGED_DATA_DIR}/resources.pak" "${DEADTIME_PACKAGED_DATA_DIR}/localization.pak" DESTINATION data)
configure_file("${PROJECT_SOURCE_DIR}/tools/release-info.txt.in" "${PROJECT_BINARY_DIR}/release-info.txt" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/release-info.txt" DESTINATION .)
file(WRITE "${PROJECT_BINARY_DIR}/release-version.txt" "${PROJECT_VERSION}")
