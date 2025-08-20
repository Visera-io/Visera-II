if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package.")
endif()

macro(link_stb in_target)
    message(STATUS "\nLinking STB (STB)")

    if(NOT TARGET STB)
        file(GLOB_RECURSE STB_SOURCE_FILES "${VISERA_CORE_EXTERNAL_DIR}/STB/*")
        add_custom_target(STB)

        target_sources(STB PRIVATE "${VISERA_CORE_SCRIPTS_DIR}/install_stb.cmake")
        set_target_properties(STB PROPERTIES FOLDER "Visera/Core/External/STB")
    endif()

    target_include_directories(${in_target} PUBLIC "${VISERA_CORE_EXTERNAL_DIR}/STB/")
endmacro()