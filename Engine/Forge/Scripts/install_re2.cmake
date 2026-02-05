# RE2 for Forge (depends on Abseil). Include after install.cmake; ensure install_abseil + ensure_abseil/link_abseil are used first.
if(NOT VISERA_FORGE_EXTERNAL_DIR)
    message(FATAL_ERROR "Please include 'install.cmake' before installing any package!")
endif()

macro(link_re2 in_target)

    message(STATUS "Linking RE2 (re2::re2)")
    if(NOT TARGET re2::re2)
        # RE2 requires Abseil to be added first (add_subdirectory); it checks TARGET absl::base.
        if(NOT TARGET absl::base)
            message(FATAL_ERROR "RE2 requires Abseil. Include install_abseil and call ensure_abseil/link_abseil before ensure_re2/link_re2.")
        endif()
        set(BUILD_SHARED_LIBS OFF)
        set(RE2_BUILD_TESTING OFF)
        set(RE2_TEST OFF)
        set(RE2_BENCHMARK OFF)
        set(RE2_INSTALL OFF)
        set(RE2_USE_ICU OFF)
        set(USEPCRE OFF)
        add_subdirectory("${VISERA_FORGE_EXTERNAL_DIR}/RE2")
        set_target_properties(re2 PROPERTIES FOLDER "Visera/Forge/External/RE2")
    endif()

    target_link_libraries(${in_target} PUBLIC re2::re2)
    target_include_directories(${in_target} PRIVATE "${VISERA_FORGE_EXTERNAL_DIR}/RE2")
endmacro()