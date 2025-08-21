set(VISERA_CORE_UTESTS "Visera-Core-UTests")
project("${VISERA_CORE_UTESTS} (${VISERA_CORE_VERSION})"
        VERSION         "${VISERA_CORE_VERSION}"
        HOMEPAGE_URL    "www.visera.io"
        LANGUAGES       CXX
)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${VISERA_CORE}>")

add_executable(${VISERA_CORE_UTESTS})

#file(GLOB_RECURSE VISERA_CORE_UTESTS_MODULES
#                  "${CMAKE_CURRENT_SOURCE_DIR}/*.ixx")

target_sources(${VISERA_CORE_UTESTS}
        PUBLIC
        FILE_SET "visera_core_utests_modules" TYPE CXX_MODULES
        FILES Main.ixx)

target_link_libraries(${VISERA_CORE_UTESTS}
                      PRIVATE
                      Visera::Core
                      Visera::Runtime)