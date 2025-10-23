message(STATUS "\nInstalling Visera Studio...")

set(VISERA_STUDIO_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source")
set(VISERA_STUDIO_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(VISERA_STUDIO_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global")
set(VISERA_STUDIO_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts")

if(BuildSharedViseraStudio)
    add_library(${VISERA_STUDIO} SHARED)
    target_compile_definitions(${VISERA_STUDIO} PRIVATE VISERA_STUDIO_BUILD_SHARED)
else()
    add_library(${VISERA_STUDIO} STATIC)
    target_compile_definitions(${VISERA_STUDIO} PRIVATE VISERA_STUDIO_BUILD_STATIC)
endif()
add_library(Visera::Studio ALIAS ${VISERA_STUDIO})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${VISERA_APP}>")
add_custom_command(
        TARGET Visera::Studio
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Visera::Studio>
        $<TARGET_FILE_DIR:${VISERA_APP}>)
if(MSVC)
    add_custom_command(
            TARGET Visera::Studio
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E $<IF:$<BOOL:$<TARGET_PDB_FILE:Visera::Studio>>,
            copy_if_different
            $<TARGET_PDB_FILE:Visera::Studio>
            $<TARGET_FILE_DIR:${VISERA_APP}>)
endif()

if(NOT TARGET Visera::Core)
    message(FATAL_ERROR "Visera-Core is not installed!")
endif()
target_link_libraries(${VISERA_STUDIO} PRIVATE Visera::Core)

if(NOT TARGET Visera::Runtime)
    message(FATAL_ERROR "Visera-Runtime is not installed!")
endif()
target_link_libraries(${VISERA_STUDIO} PRIVATE Visera::Runtime)

#
# << Install External Packages >>
#

list(APPEND CMAKE_MODULE_PATH ${VISERA_STUDIO_SCRIPTS_DIR})

if(NOT ${VISERA_OFFSCREEN_MODE})
include(install_imgui)
link_imgui(${VISERA_STUDIO})
endif()

include(install_freetype)
link_freetype(${VISERA_STUDIO})

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