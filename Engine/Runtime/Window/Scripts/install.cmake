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
        ${VISERA_WINDOW_INCLUDE_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_window_modules" TYPE CXX_MODULES
        FILES ${VISERA_WINDOW_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_window(...)
else()
    add_library(${VISERA_WINDOW} SHARED)
    add_library(Visera::Window ALIAS ${VISERA_WINDOW})

    set_target_properties(${VISERA_WINDOW} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
   if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(
       TARGET Visera::Window
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "$<TARGET_PDB_FILE:Visera::Window>"
       "${VISERA_APP_FRAMEWORK_DIR}"
   )
   endif()

    if(NOT TARGET Visera::Global)
        message(FATAL_ERROR "Visera-Global is not installed!")
    endif()
    target_link_libraries(${VISERA_WINDOW} PRIVATE Visera::Global)

    install_visera_window(${VISERA_WINDOW})
    set_target_properties(${VISERA_WINDOW} PROPERTIES FOLDER "Visera/Window")
endif()