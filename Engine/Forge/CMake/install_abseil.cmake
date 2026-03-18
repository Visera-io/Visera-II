if(NOT VISERA_FORGE_EXTERNAL_DIR)
    message(FATAL_ERROR "Please include 'install.cmake' before installing any package!")
endif()

# Ensures Abseil is built as static libs; optionally link in_target to common absl targets (for Forge exe or RE2).
macro(link_abseil in_target)
    message(STATUS "Linking Abseil (static)")
    if(NOT TARGET absl::base)
        set(BUILD_SHARED_LIBS       OFF)
        set(ABSL_BUILD_TESTING      OFF)
        set(ABSL_BUILD_TEST_HELPERS OFF)
        set(ABSL_ENABLE_INSTALL     OFF)
        set(ABSL_IDE_FOLDER "Visera/Forge/External/Abseil")
        add_subdirectory("${VISERA_FORGE_EXTERNAL_DIR}/Abseil")
    endif()

    target_link_libraries(${in_target} PRIVATE
        absl::strings)
    target_include_directories(${in_target} PRIVATE "${VISERA_FORGE_EXTERNAL_DIR}/Abseil")
endmacro()
