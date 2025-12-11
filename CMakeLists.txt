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
message("Welcome to play ${PROJECT_NAME}!")

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

if(VISERA_OFFSCREEN_MODE)
    message(STATUS "Off-Screen Mode")
    add_compile_definitions (VISERA_OFFSCREEN_MODE)
endif()

if(MSVC)
    add_compile_definitions (NOMINMAX)
    add_compile_options     (/utf-8)
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Develop;Release;" CACHE STRING "" FORCE)

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

set(VISERA_CORE_DIR     "${PROJECT_SOURCE_DIR}/Core")
set(VISERA_RUNTIME_DIR  "${PROJECT_SOURCE_DIR}/Runtime")
set(VISERA_ENGINE_DIR   "${PROJECT_SOURCE_DIR}/Engine")
set(VISERA_STUDIO_DIR   "${PROJECT_SOURCE_DIR}/Studio")
if(NOT VISERA_APP_DIR OR NOT VISERA_APP_NAME)
message(FATAL_ERROR "Please specify both VISERA_APP_DIR and VISERA_APP_NAME!")
endif()
if(${VISERA_APP_NAME} STREQUAL "Visera-App")
message(FATAL_ERROR "Please do NOT use \"Visera-App\" as the name!")
endif()
# Turn VISERA_APP_DIR into an absolute path
get_filename_component(
    VISERA_APP_DIR
    ${VISERA_APP_DIR}
    REALPATH
    BASE_DIR "${PROJECT_SOURCE_DIR}"
)
message(STATUS "App Name  : \"${VISERA_APP_NAME}\"")
message(STATUS "App Author: \"${VISERA_APP_AUTHOR}\"")
message(STATUS "App Path  : \"${VISERA_APP_DIR}\"")

set(VISERA_APP_PROJECT "${VISERA_APP_DIR}/Visera-App.csproj")
if(NOT EXISTS "${VISERA_APP_PROJECT}")
message(FATAL_ERROR "Failed to find the \"${VISERA_APP_PROJECT}\"!")
endif()
set(VISERA_APP ${VISERA_APP_NAME})
add_compile_definitions(VISERA_APP=\"${VISERA_APP_NAME}\")

if(WIN32)
    add_compile_definitions(VISERA_ON_WINDOWS_SYSTEM)
    set(VISERA_APP_FRAMEWORK_DIR "$<TARGET_FILE_DIR:${VISERA_APP}>")
elseif(APPLE)
    add_compile_definitions(VISERA_ON_APPLE_SYSTEM)
    set(VISERA_APP_FRAMEWORK_DIR "$<TARGET_BUNDLE_DIR:${VISERA_APP}>/Contents/Frameworks")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(VISERA_DEBUG_MODE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Develop")
    add_compile_definitions(VISERA_DEVELOP_MODE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(VISERA_RELEASE_MODE)
else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

add_subdirectory(${VISERA_CORE_DIR})
add_subdirectory(${VISERA_RUNTIME_DIR})
add_subdirectory(${VISERA_ENGINE_DIR})
add_subdirectory(${VISERA_STUDIO_DIR})
add_subdirectory(${VISERA_APP_DIR}) #[TODO]: App -> Engine

add_custom_target(
    "Visera-App" ALL
    COMMAND
    dotnet build ${VISERA_APP_PROJECT}
    -c $<IF:$<CONFIG:Release>,Release,Debug>

    -p:VISERA_ENGINE_APIS=${VISERA_ENGINE_DIR}/APIs/

    -p:OutputPath=${VISERA_APP_FRAMEWORK_DIR}/
    -p:BaseIntermediateOutputPath=${CMAKE_BINARY_DIR}/App/
    -p:IntermediateOutputPath=${CMAKE_BINARY_DIR}/App/

    WORKING_DIRECTORY ${VISERA_APP_DIR}
)
add_dependencies(${VISERA_APP} "Visera-App")
set_target_properties("Visera-App" PROPERTIES FOLDER "Visera/App")

if(CMAKE_GENERATOR MATCHES "Visual Studio")
message(STATUS "Configuring Visual Studio properties...")
set_property(DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT ${VISERA_APP})
set_property(TARGET ${VISERA_APP}
    PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
endif()