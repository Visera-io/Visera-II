set(VISERA_GRAPHICS_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_GRAPHICS_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_GRAPHICS_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_GRAPHICS_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_graphics in_target)
    message(STATUS "\nInstalling Visera Graphics...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_GRAPHICS_SCRIPTS_DIR})

    file(GLOB_RECURSE VISERA_GRAPHICS_MODULES "${VISERA_GRAPHICS_SOURCE_DIR}/*.ixx")
    list(FILTER VISERA_GRAPHICS_MODULES EXCLUDE REGEX ".*/UI/.*")

    target_include_directories(${in_target}
        PUBLIC
        $<BUILD_INTERFACE:${VISERA_GRAPHICS_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Runtime/Graphics>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_graphics_modules" TYPE CXX_MODULES
        FILES ${VISERA_GRAPHICS_MODULES})
endmacro()
