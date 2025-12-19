set(VISERA_ASSETS_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"     CACHE PATH "")
set(VISERA_ASSETS_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"   CACHE PATH "")
set(VISERA_ASSETS_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"     CACHE PATH "")
set(VISERA_ASSETS_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"    CACHE PATH "")

macro(install_visera_assets in_target)
    message(STATUS "\nInstalling Visera Assets...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_ASSETS_SCRIPTS_DIR})

    include(install_stb)
    link_stb(${in_target})

    include(install_libpng)
    link_libpng(${in_target})

    include(install_miniaudio)
    link_miniaudio(${in_target})

    file(GLOB_RECURSE VISERA_ASSETS_MODULES "${VISERA_ASSETS_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_ASSETS_GLOBAL_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_assets_modules" TYPE CXX_MODULES
        FILES ${VISERA_ASSETS_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_assets(...)
else()
    add_library(${VISERA_ASSETS} SHARED)
    target_compile_definitions(${VISERA_ASSETS} PRIVATE VISERA_ASSETS_BUILD_SHARED)
    add_library(Visera::Assets ALIAS ${VISERA_ASSETS})

    set_target_properties(${VISERA_ASSETS} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
   if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(
       TARGET Visera::Assets
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "$<TARGET_PDB_FILE:Visera::Assets>"
       "${VISERA_APP_FRAMEWORK_DIR}"
   )
   endif()

    if(NOT TARGET Visera::Runtime)
        message(FATAL_ERROR "Visera-Runtime is not installed!")
    endif()
    target_link_libraries(${VISERA_ASSETS} PRIVATE Visera::Runtime)

    install_visera_assets(${VISERA_ASSETS})
    set_target_properties(${VISERA_ASSETS} PROPERTIES FOLDER "Visera/Assets")
endif()