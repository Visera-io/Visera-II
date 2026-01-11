cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

SET(VISERA_GLOBAL_VERSION_MAJOR 1)
SET(VISERA_GLOBAL_VERSION_MINOR 0)
SET(VISERA_GLOBAL_VERSION_PATCH 0)
SET(VISERA_GLOBAL_VERSION "${VISERA_GLOBAL_VERSION_MAJOR}.${VISERA_GLOBAL_VERSION_MINOR}.${VISERA_GLOBAL_VERSION_PATCH}")

set(VISERA_GLOBAL "Visera-Runtime")
project("${VISERA_GLOBAL} (${VISERA_GLOBAL_VERSION})"
    VERSION         "${VISERA_GLOBAL_VERSION}"
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

#[Install]
include("Scripts/install.cmake")