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

add_subdirectory("${PROJECT_SOURCE_DIR}/Core")
add_subdirectory("${PROJECT_SOURCE_DIR}/Runtime")
add_subdirectory("${PROJECT_SOURCE_DIR}/Engine")
add_subdirectory("${PROJECT_SOURCE_DIR}/Studio")

add_subdirectory(Apps/${VISERA_APP})