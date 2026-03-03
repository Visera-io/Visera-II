set(VISERA_UI_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_UI_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_UI_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_UI_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_ui in_target)
    message(STATUS "\nInstalling Visera UI...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_UI_SCRIPTS_DIR})

    include(install_imgui)
    link_imgui(${in_target})

    file(GLOB_RECURSE VISERA_UI_MODULES "${VISERA_UI_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        $<BUILD_INTERFACE:${VISERA_UI_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Runtime/UI>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_ui_modules" TYPE CXX_MODULES
        FILES ${VISERA_UI_MODULES})
endmacro()
