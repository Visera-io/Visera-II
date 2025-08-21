set(VISERA_APP "AlohaVisera")
project("${VISERA_APP} (${VISERA_CORE_VERSION})"
        VERSION         "${VISERA_CORE_VERSION}"
        HOMEPAGE_URL    "www.visera.io"
        LANGUAGES       CXX
)
message("Installing ${PROJECT_NAME}")

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${VISERA_CORE}>")

add_executable(${VISERA_APP})

file(GLOB_RECURSE VISERA_APP_MODULES
                  "${CMAKE_CURRENT_SOURCE_DIR}/*.ixx")

target_sources(${VISERA_APP}
        PUBLIC
        FILE_SET "visera_app_modules" TYPE CXX_MODULES
        FILES ${VISERA_APP_MODULES})

target_link_libraries(${VISERA_APP}
                      PRIVATE
                      Visera::Core
                      Visera::Runtime
                      Visera::Engine
                      Visera::Studio
)