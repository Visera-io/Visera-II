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
        ${VISERA_INPUT_INCLUDE_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_input_modules" TYPE CXX_MODULES
        FILES ${VISERA_INPUT_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_input(...)
else()
    add_library(${VISERA_INPUT} SHARED)
    add_library(Visera::Input ALIAS ${VISERA_INPUT})

    set_target_properties(${VISERA_INPUT} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
   if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(
       TARGET Visera::Input
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "$<TARGET_PDB_FILE:Visera::Input>"
       "${VISERA_APP_FRAMEWORK_DIR}"
   )
   endif()

    if(NOT TARGET Visera::Global)
        message(FATAL_ERROR "Visera-Global is not installed!")
    endif()
    target_link_libraries(${VISERA_INPUT} PRIVATE Visera::Global)

    install_visera_input(${VISERA_INPUT})
    set_target_properties(${VISERA_INPUT} PROPERTIES FOLDER "Visera/Input")
endif()