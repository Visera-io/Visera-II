cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

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
message(STATUS "${PROJECT_NAME}")

option(VISERA_OFFSCREEN_MODE    "Create a headless application"     OFF)
option(VISERA_BUILD_FORGE       "Build Visera-Forge (toolchain)"    ON )
option(VISERA_BUILD_STATIC      "Build Visera as static library (single exe, no DLL)" ON)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
add_compile_options(
    -Wno-nullability-completeness        # hides "_Nonnull/_Nullable" notes
    -Wno-nullability-extension           # suppresses "pointer is missing nullability specifier"
    -Wno-unused-parameter                # quiets unused args in engine stubs
    -Wno-missing-field-initializers      # silences C-style struct init warnings
    -Wno-unknown-pragmas                 # hides vendor #pragma notes in external libs
    -Wno-ignored-qualifiers              # avoids 'type qualifiers ignored on cast' noise
    -Wno-extra-semi                      # some headers (GLSL, SPIR-V) produce these
    -Wno-deprecated-declarations         # prevents spam from deprecated macOS APIs
)
endif()

if(MSVC)
    add_compile_definitions (WIN32_LEAN_AND_MEAN NOMINMAX)
    add_compile_options     (/utf-8)
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Develop;Release" CACHE STRING "" FORCE)
if (MSVC)
    set(CMAKE_C_FLAGS_DEVELOP             "/O2 /Zi")
    set(CMAKE_CXX_FLAGS_DEVELOP           "/O2 /Zi")
    set(CMAKE_EXE_LINKER_FLAGS_DEVELOP    "/DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_DEVELOP "/DEBUG")
else()
    set(CMAKE_C_FLAGS_DEVELOP             "-O2 -g")
    set(CMAKE_CXX_FLAGS_DEVELOP           "-O2 -g")
    set(CMAKE_EXE_LINKER_FLAGS_DEVELOP    "")
    set(CMAKE_SHARED_LINKER_FLAGS_DEVELOP "")
endif()

if(WIN32)
    add_compile_definitions(VISERA_ON_WINDOWS_SYSTEM)
elseif(APPLE)
    add_compile_definitions(VISERA_ON_APPLE_SYSTEM)
endif()

# Unified engine output paths (app-independent).
set(VISERA_ENGINE_OUTPUT_ROOT "${CMAKE_BINARY_DIR}/Engine" CACHE PATH "Engine binary output root directory")
set(VISERA_ENGINE_FRAMEWORK_DIR "${VISERA_ENGINE_OUTPUT_ROOT}/$<CONFIG>" CACHE STRING "Engine runtime output directory")
set(VISERA_ENGINE_RESOURCE_DIR "${VISERA_ENGINE_FRAMEWORK_DIR}/Resources" CACHE STRING "Engine runtime resource directory")

# Pre-create engine output directories for each configuration to avoid MSBuild MSB3191.
foreach(CFG ${CMAKE_CONFIGURATION_TYPES})
    file(MAKE_DIRECTORY "${VISERA_ENGINE_OUTPUT_ROOT}/${CFG}")
endforeach()

add_subdirectory(Engine)