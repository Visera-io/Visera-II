message(STATUS "\nInstalling Visera Studio...")

set(VISERA_STUDIO_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source")
set(VISERA_STUDIO_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(VISERA_STUDIO_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global")
set(VISERA_STUDIO_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts")

add_library(${VISERA_STUDIO} SHARED)
target_compile_definitions(${VISERA_STUDIO} PRIVATE VISERA_STUDIO_BUILD_SHARED)
add_library(Visera::Studio ALIAS ${VISERA_STUDIO})

set_target_properties(${VISERA_STUDIO} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
)
add_custom_command(
    TARGET Visera::Studio
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:Visera::Studio>
    ${VISERA_APP_FRAMEWORK_DIR})
if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
add_custom_command(
    TARGET Visera::Studio
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_PDB_FILE:Visera::Studio>"
    "${VISERA_APP_FRAMEWORK_DIR}"
)
endif()

if(NOT TARGET Visera::Runtime)
    message(FATAL_ERROR "Visera-Runtime is not installed!")
endif()
target_link_libraries(${VISERA_STUDIO} PRIVATE Visera::Runtime)

if(NOT TARGET Visera::Engine)
    message(FATAL_ERROR "Visera-Engine is not installed!")
endif()
target_link_libraries(${VISERA_STUDIO} PRIVATE Visera::Engine)

#
# << Install External Packages >>
#
list(APPEND CMAKE_MODULE_PATH ${VISERA_STUDIO_SCRIPTS_DIR})

include(install_freetype)
link_freetype(${VISERA_STUDIO})

if(NOT VISERA_OFFSCREEN_MODE)
    include(install_imgui)
    link_imgui(${VISERA_STUDIO})
endif()
#
#
#
file(GLOB_RECURSE VISERA_STUDIO_MODULES "${VISERA_STUDIO_SOURCE_DIR}/*.ixx")

target_include_directories(${VISERA_STUDIO}
                           PUBLIC
                           ${VISERA_STUDIO_GLOBAL_DIR})

target_sources(${VISERA_STUDIO}
               PUBLIC
               FILE_SET "visera_studio_modules" TYPE CXX_MODULES
               FILES ${VISERA_STUDIO_MODULES})

set_target_properties(${VISERA_STUDIO} PROPERTIES FOLDER "Visera/Studio")