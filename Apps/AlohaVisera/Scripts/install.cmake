message(STATUS "\nInstalling ${VISERA_APP}...")

set(VISERA_APP_ASSETS_DIR   "${PROJECT_SOURCE_DIR}/Assets")
set(VISERA_APP_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source")
set(VISERA_APP_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(VISERA_APP_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global")
set(VISERA_APP_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts")

add_executable(${VISERA_APP})
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

# App Icon
if(MSVC)
    enable_language(RC)
    # Optional: Set icon path variable (if you want to make it configurable)
    # set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}")
    target_sources(${VISERA_APP}
                   PRIVATE
                   "${VISERA_APP_ASSETS_DIR}/Icons/App.ico.rc")
endif()