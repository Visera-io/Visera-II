cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

SET(VISERA_VERSION_MAJOR 1)
SET(VISERA_VERSION_MINOR 0)
SET(VISERA_VERSION_PATCH 0)
SET(VISERA_VERSION "${VISERA_VERSION_MAJOR}.${VISERA_VERSION_MINOR}.${VISERA_VERSION_PATCH}")

set(VISERA "Visera")
project("${VISERA} (${VISERA_VERSION})"
        VERSION         "${VISERA_VERSION}"
        HOMEPAGE_URL    "www.visera.io"
        LANGUAGES       CXX
)
message("Installing ${PROJECT_NAME}")

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

endif()

if(MSVC)
    add_compile_definitions (NOMINMAX)
    add_compile_options     (/utf-8)
endif()

set(VISERA_CORE_DIR     "${PROJECT_SOURCE_DIR}/Core")
set(VISERA_RUNTIME_DIR  "${PROJECT_SOURCE_DIR}/Runtime")
set(VISERA_ENGINE_DIR   "${PROJECT_SOURCE_DIR}/Engine")
set(VISERA_STUDIO_DIR   "${PROJECT_SOURCE_DIR}/Studio")
set(VISERA_APP_DIR      "${PROJECT_SOURCE_DIR}/Apps/${VISERA_APP}")

add_compile_definitions(
    VISERA_CORE_DIR=u8"${VISERA_CORE_DIR}"
    VISERA_RUNTIME_DIR=u8"${VISERA_RUNTIME_DIR}"
    VISERA_ENGINE_DIR=u8"${VISERA_ENGINE_DIR}"
    VISERA_STUDIO_DIR=u8"${VISERA_STUDIO_DIR}"
    VISERA_APP_DIR=u8"${VISERA_APP_DIR}"
)

add_subdirectory(${VISERA_CORE_DIR})
add_subdirectory(${VISERA_RUNTIME_DIR})
add_subdirectory(${VISERA_ENGINE_DIR})
add_subdirectory(${VISERA_STUDIO_DIR})
add_subdirectory(${VISERA_APP_DIR})