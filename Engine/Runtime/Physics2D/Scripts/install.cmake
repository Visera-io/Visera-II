set(VISERA_PHYSICS2D_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_PHYSICS2D_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_PHYSICS2D_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_PHYSICS2D_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_physics2d in_target)
    message(STATUS "\nInstalling Visera Physics2D...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_PHYSICS2D_SCRIPTS_DIR})
    
    include(install_box2d)
    link_box2d(${in_target})

    file(GLOB_RECURSE VISERA_PHYSICS2D_MODULES "${VISERA_PHYSICS2D_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_PHYSICS2D_INCLUDE_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_physics2d_modules" TYPE CXX_MODULES
        FILES ${VISERA_PHYSICS2D_MODULES})
endmacro()
