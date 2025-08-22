if(NOT VISERA_APP)
    message(FATAL_ERROR "Empty App Name!")
endif()

if(NOT VISERA_APP_VERSION)
    set(VISERA_APP_NAME "${VISERA_APP} (Sandbox)")
endif()

project("${VISERA_APP_NAME}"
        VERSION         "${VISERA_APP_VERSION}"
        HOMEPAGE_URL    "www.visera.io"
        LANGUAGES       CXX
)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

include("Scripts/install.cmake")