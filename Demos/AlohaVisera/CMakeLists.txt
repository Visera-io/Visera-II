cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

set(VISERA_APP          "AlohaVisera")
set(VISERA_APP_AUTHOR   "LJYC(ljyc.me)")
set(VISERA_APP_DIR      ${PROJECT_SOURCE_DIR})

SET(VISERA_APP_VERSION_MAJOR 1)
SET(VISERA_APP_VERSION_MINOR 0)
SET(VISERA_APP_VERSION_PATCH 0)
SET(VISERA_APP_VERSION "${VISERA_APP_VERSION_MAJOR}.${VISERA_APP_VERSION_MINOR}.${VISERA_APP_VERSION_PATCH}")

project(${VISERA_APP}
    VERSION         "${VISERA_APP_VERSION}"
    HOMEPAGE_URL    "www.visera.io"
    LANGUAGES       CXX
)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

include("Scripts/install.cmake")