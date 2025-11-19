cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

project(${VISERA_APP_NAME}
    VERSION         "${VISERA_APP_VERSION}"
    HOMEPAGE_URL    "www.visera.io"
    LANGUAGES       CXX
)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

include("Scripts/install.cmake")