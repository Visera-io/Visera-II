set(VISERA_INPUT_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_INPUT_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_INPUT_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_INPUT_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_input in_target)
    message(STATUS "\nInstalling Visera Input...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_INPUT_SCRIPTS_DIR})

    file(GLOB_RECURSE VISERA_INPUT_MODULES "${VISERA_INPUT_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        $<BUILD_INTERFACE:${VISERA_INPUT_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Runtime/Input>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_input_modules" TYPE CXX_MODULES
        FILES ${VISERA_INPUT_MODULES})
endmacro()
