cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

if(VISERA_OFFSCREEN_MODE)
    message(STATUS "Off-Screen Mode")
    add_compile_definitions (VISERA_OFFSCREEN_MODE)
endif()

option(VISERA_PROFILING_MODE "Enable profiling instrumentation" OFF)
if(VISERA_PROFILING_MODE)
    message(STATUS "Profiling Mode")
    add_compile_definitions(VISERA_PROFILING_MODE)
endif()

set(VISERA_CORE_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/Core")
set(VISERA_FORGE_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/Forge")
set(VISERA_PLATFORM_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/Platform")
set(VISERA_RUNTIME_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/Runtime")
if(VISERA_BUILD_FORGE)
set(VISERA_FORGE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Toolkit/Forge" CACHE PATH "Forge executable output directory")
endif()

if(CMAKE_CONFIGURATION_TYPES)
    add_compile_definitions(
        $<$<CONFIG:Debug>:VISERA_DEBUG_MODE>
        $<$<CONFIG:Develop>:VISERA_DEVELOP_MODE>
        $<$<CONFIG:Release>:VISERA_RELEASE_MODE>
    )
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(VISERA_DEBUG_MODE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Develop")
    add_compile_definitions(VISERA_DEVELOP_MODE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(VISERA_RELEASE_MODE)
else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

if(VISERA_BUILD_STATIC)
    add_library(${VISERA} STATIC)
    target_compile_definitions(${VISERA} PUBLIC
        VISERA_CORE_BUILD_STATIC
        VISERA_RUNTIME_BUILD_STATIC
        VISERA_PLATFORM_BUILD_STATIC
    )
else()
    add_library(${VISERA} SHARED)
    target_compile_definitions(${VISERA} PRIVATE
        VISERA_CORE_BUILD_SHARED
        VISERA_RUNTIME_BUILD_SHARED
        VISERA_PLATFORM_BUILD_SHARED
    )
endif()

set_target_properties(${VISERA} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${VISERA_ENGINE_FRAMEWORK_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${VISERA_ENGINE_FRAMEWORK_DIR}"
)
add_subdirectory(${VISERA_CORE_DIR})
add_subdirectory(${VISERA_PLATFORM_DIR})
add_subdirectory(${VISERA_RUNTIME_DIR})
install_visera_core     (${VISERA})
install_visera_platform (${VISERA})
install_visera_runtime  (${VISERA})

if(VISERA_BUILD_FORGE)
    add_subdirectory(${VISERA_FORGE_DIR})
endif()