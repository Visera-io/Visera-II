set(VISERA_PLATFORM_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_PLATFORM_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_PLATFORM_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_PLATFORM_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_platform in_target)
    message(STATUS "\nInstalling Visera Platform...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_PLATFORM_SCRIPTS_DIR})

    include(install_glfw)
    link_glfw(${in_target})

    set(VISERA_PLATFORM_MODULES "")
    # Shared Modules
    list(APPEND VISERA_PLATFORM_MODULES "${VISERA_PLATFORM_SOURCE_DIR}/Visera-Platform.ixx")
    file(GLOB_RECURSE _cross_modules "${VISERA_PLATFORM_SOURCE_DIR}/Cross/*.ixx")
    file(GLOB_RECURSE _interface_modules "${VISERA_PLATFORM_SOURCE_DIR}/Interface/*.ixx")
    list(APPEND VISERA_PLATFORM_MODULES ${_cross_modules} ${_interface_modules})
    # Platform-Specific Modules
    if(WIN32)
        file(GLOB_RECURSE _platform_modules "${VISERA_PLATFORM_SOURCE_DIR}/Windows/*.ixx")
    elseif(APPLE)
        file(GLOB_RECURSE _platform_modules "${VISERA_PLATFORM_SOURCE_DIR}/MacOS/*.ixx")
    else()
        set(_platform_modules "")
    endif()
    list(APPEND VISERA_PLATFORM_MODULES ${_platform_modules})

    target_include_directories(${in_target}
        PUBLIC
        $<BUILD_INTERFACE:${VISERA_PLATFORM_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Platform>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_platform_modules" TYPE CXX_MODULES
        FILES ${VISERA_PLATFORM_MODULES})
endmacro()
