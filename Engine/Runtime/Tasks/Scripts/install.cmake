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
        $<BUILD_INTERFACE:${VISERA_TASKS_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Runtime/Tasks>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_tasks_modules" TYPE CXX_MODULES
        FILES ${VISERA_TASKS_MODULES})
endmacro()
