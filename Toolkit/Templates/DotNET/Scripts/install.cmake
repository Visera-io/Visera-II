set(VISERA_SCRIPTING_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_SCRIPTING_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_SCRIPTING_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_SCRIPTING_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_scripting in_target)
    message(STATUS "\nInstalling Visera Scripting...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_SCRIPTING_SCRIPTS_DIR})

    include(install_dotnet)
    link_dotnet(${in_target})

    file(GLOB_RECURSE VISERA_SCRIPTING_MODULES "${VISERA_SCRIPTING_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_SCRIPTING_INCLUDE_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_scripting_modules" TYPE CXX_MODULES
        FILES ${VISERA_SCRIPTING_MODULES})
endmacro()
