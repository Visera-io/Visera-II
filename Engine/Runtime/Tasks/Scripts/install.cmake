  set(VISERA_TASKS_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
  set(VISERA_TASKS_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
  set(VISERA_TASKS_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
  set(VISERA_TASKS_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

  macro(install_visera_tasks in_target)
    message(STATUS "\nInstalling Visera Tasks...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_TASKS_SCRIPTS_DIR})

    file(GLOB_RECURSE VISERA_TASKS_MODULES "${VISERA_TASKS_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_TASKS_INCLUDE_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_tasks_modules" TYPE CXX_MODULES
        FILES ${VISERA_TASKS_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_tasks(...)
else()
    add_library(${VISERA_TASKS} SHARED)
    target_compile_definitions(${VISERA_TASKS} PRIVATE VISERA_TASKS_BUILD_SHARED)
    add_library(Visera::Tasks ALIAS ${VISERA_TASKS})

    set_target_properties(${VISERA_TASKS} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
    if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(
        TARGET Visera::Tasks
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_PDB_FILE:Visera::Tasks>"
        "${VISERA_APP_FRAMEWORK_DIR}"
    )
    endif()

    if(NOT TARGET Visera::Global)
        message(FATAL_ERROR "Visera-Global is not installed!")
    endif()
    target_link_libraries(${VISERA_TASKS} PUBLIC Visera::Global)

    install_visera_tasks(${VISERA_TASKS})
    set_target_properties(${VISERA_TASKS} PROPERTIES FOLDER "Visera/Tasks")
endif()