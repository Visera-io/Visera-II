cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

SET(VISERA_FORGE_VERSION_MAJOR 1)
SET(VISERA_FORGE_VERSION_MINOR 0)
SET(VISERA_FORGE_VERSION_PATCH 0)
SET(VISERA_FORGE_VERSION "${VISERA_FORGE_VERSION_MAJOR}.${VISERA_FORGE_VERSION_MINOR}.${VISERA_FORGE_VERSION_PATCH}")

set(VISERA_FORGE "Visera-Forge")
project("${VISERA_FORGE} (${VISERA_FORGE_VERSION})"
    VERSION         "${VISERA_FORGE_VERSION}"
    HOMEPAGE_URL    "www.visera.io"
    LANGUAGES       CXX
)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
      "\n"
      "In-source builds are not allowed.\n"
      "Instead, provide a path to build tree like so:\n"
      "cmake -B <destination>\n"
      "\n"
      "To remove files you accidentally created execute:\n"
      "rm -rf CMakeFiles CMakeCache.txt\n"
    )
endif()

# Pre-create output dirs per config (Debug/Develop/Release) to avoid MSBuild etc. issues
foreach(CFG ${CMAKE_CONFIGURATION_TYPES})
    file(MAKE_DIRECTORY "${VISERA_FORGE_OUTPUT_DIR}/${CFG}")
endforeach()

#[Install]
include("CMake/install.cmake")