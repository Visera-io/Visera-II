set(VISERA_WINDOW_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_WINDOW_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_WINDOW_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_WINDOW_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_window in_target)
    message(STATUS "\nInstalling Visera Window...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_WINDOW_SCRIPTS_DIR})

    file(GLOB_RECURSE VISERA_WINDOW_MODULES "${VISERA_WINDOW_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        $<BUILD_INTERFACE:${VISERA_WINDOW_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Runtime/Window>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_window_modules" TYPE CXX_MODULES
        FILES ${VISERA_WINDOW_MODULES})
endmacro()
