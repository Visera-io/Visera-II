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
option(VISERA_MONOLITHIC_MODE   "Create a single Visera.dll"        OFF)

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

if(APPLE)
  target_link_options(YourTarget PRIVATE
    "--ld-path=/opt/homebrew/opt/llvm@20/bin/ld64.lld"
  )
endif()

if(MSVC)
    add_compile_definitions (NOMINMAX)
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

set(VISERA_APP          "AlohaVisera")
set(VISERA_APP_AUTHOR   "LJYC (ljyc.me)")
set(VISERA_APP_DIR      "Demos/AlohaVisera")

if(NOT VISERA_APP OR NOT VISERA_APP_DIR)
    message(FATAL_ERROR "Please specify both VISERA_APP and VISERA_APP_DIR!")
endif()
# Turn VISERA_APP_DIR into an absolute path
get_filename_component(
    VISERA_APP_DIR    ${VISERA_APP_DIR}
    REALPATH BASE_DIR "${PROJECT_SOURCE_DIR}"
)
message(STATUS "App Name  : \"${VISERA_APP}\"")
message(STATUS "App Author: \"${VISERA_APP_AUTHOR}\"")
message(STATUS "App Path  : \"${VISERA_APP_DIR}\"")

if(WIN32)
    add_compile_definitions(VISERA_ON_WINDOWS_SYSTEM)
    set(VISERA_APP_BUNDLE_DIR     "$<TARGET_FILE_DIR:${VISERA_APP}>")
    set(VISERA_APP_EXECUTABLE_DIR "${VISERA_APP_BUNDLE_DIR}")
    set(VISERA_APP_FRAMEWORK_DIR  "${VISERA_APP_BUNDLE_DIR}")
    set(VISERA_APP_RESOURCE_DIR   "${VISERA_APP_BUNDLE_DIR}")
elseif(APPLE)
    add_compile_definitions(VISERA_ON_APPLE_SYSTEM)
    set(VISERA_APP_BUNDLE_DIR     "$<TARGET_BUNDLE_DIR:${VISERA_APP}>")
    set(VISERA_APP_EXECUTABLE_DIR "${VISERA_APP_BUNDLE_DIR}/MacOS")
    set(VISERA_APP_FRAMEWORK_DIR  "${VISERA_APP_BUNDLE_DIR}/Contents/Frameworks")
    set(VISERA_APP_RESOURCE_DIR   "${VISERA_APP_BUNDLE_DIR}/Contents/Resources")
endif()

add_subdirectory(Engine)
add_subdirectory(${VISERA_APP_DIR})

#add_custom_target(
#    "Visera-App" ALL COMMAND
#    dotnet build ${VISERA_APP_PROJECT}
#    -c $<IF:$<CONFIG:Release>,Release,Debug>
#
#    -p:VISERA_ENGINE_APIS=${VISERA_API_DIR}
#
#    -p:OutputPath=${VISERA_APP_FRAMEWORK_DIR}/
#    -p:BaseIntermediateOutputPath=${CMAKE_BINARY_DIR}/API/
#    -p:IntermediateOutputPath=${CMAKE_BINARY_DIR}/API/
#
#    WORKING_DIRECTORY ${VISERA_APP_DIR}
#)
#add_dependencies(${VISERA_APP} "Visera-App")
#set_target_properties("Visera-App" PROPERTIES FOLDER "Visera/App")