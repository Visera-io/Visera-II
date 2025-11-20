message(STATUS "\nInstalling ${VISERA_APP}...")

set(VISERA_APP_ASSETS_DIR   "${PROJECT_SOURCE_DIR}/Assets")
set(VISERA_APP_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source")
set(VISERA_APP_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(VISERA_APP_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global")
set(VISERA_APP_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts")

if(APPLE)
add_executable(${VISERA_APP} MACOSX_BUNDLE)
else()
add_executable(${VISERA_APP})
endif()

target_include_directories(${VISERA_APP} PRIVATE ${VISERA_APP_GLOBAL_DIR})

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

# <<App Icon>>
if(WIN32)
    set(APP_ICON_WINDOWS "${VISERA_APP_ASSETS_DIR}/Icon/App.ico.rc")
    enable_language(RC)
    # Optional: Set icon path variable (if you want to make it configurable)
    # set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}")
    target_sources(${VISERA_APP}
                   PRIVATE
                   ${APP_ICON_WINDOWS})
elseif(APPLE)
    set(APP_ICON_MACOS "${VISERA_APP_ASSETS_DIR}/Icon/App.icns")

    set_source_files_properties(${APP_ICON_MACOS}
            PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")

    target_sources(${VISERA_APP}
                   PRIVATE
                   ${APP_ICON_MACOS})

    # Set Info.plist values (can be overridden by your own Info.plist if needed)
    set_target_properties(AlohaVisera PROPERTIES
            MACOSX_BUNDLE_ICON_FILE "App.icns"
            RESOURCE "${APP_ICON_MACOS}"
    )
endif()