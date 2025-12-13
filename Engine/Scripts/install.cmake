set(VISERA_ENGINE_ASSETS_DIR   "${PROJECT_SOURCE_DIR}/Assets"   CACHE PATH "")
set(VISERA_ENGINE_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE PATH "")
set(VISERA_ENGINE_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE PATH "")
set(VISERA_ENGINE_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"   CACHE PATH "")
set(VISERA_ENGINE_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE PATH "")

macro(install_visera_engine in_target)
    message(STATUS "\nInstalling Visera Engine...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_ENGINE_SCRIPTS_DIR})

    include(install_entt)
    link_entt(${in_target})

    include(install_wwise)
    link_wwise(${in_target})

    include(install_slang)
    link_slang(${in_target})

    file(GLOB_RECURSE VISERA_ENGINE_MODULES "${VISERA_ENGINE_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
            PUBLIC
            ${VISERA_ENGINE_GLOBAL_DIR})

    target_sources(${in_target}
            PUBLIC
            FILE_SET "visera_engine_modules" TYPE CXX_MODULES
            FILES ${VISERA_ENGINE_MODULES})

    add_custom_command(
            TARGET ${in_target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${VISERA_ENGINE_ASSETS_DIR}"
            "${VISERA_APP_RESOURCE_DIR}/Assets/Engine"
    )
endmacro()

if(VISERA_MONOLITHIC_MODE)
    message(STATUS "\n[Monolithic Mode]: please call \"install_visera_engine\".")
    #install_visera_engine(...)
else()
    add_library(${VISERA_ENGINE} SHARED)
    target_compile_definitions(${VISERA_ENGINE} PRIVATE VISERA_ENGINE_BUILD_SHARED)
    add_library(Visera::Engine ALIAS ${VISERA_ENGINE})

    set_target_properties(${VISERA_ENGINE} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
    if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(
        TARGET ${VISERA_ENGINE}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_PDB_FILE:Visera::Engine>"
        "${VISERA_APP_FRAMEWORK_DIR}"
    )
    endif()

    if(NOT TARGET Visera::Runtime)
        message(FATAL_ERROR "Visera-Runtime is not installed!")
    endif()
    target_link_libraries(${VISERA_ENGINE} PUBLIC Visera::Runtime)

    install_visera_engine(${VISERA_ENGINE})
    set_target_properties(${in_target} PROPERTIES FOLDER "Visera/Engine")
endif()