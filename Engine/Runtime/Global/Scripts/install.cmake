  set(VISERA_GLOBAL_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"      CACHE PATH "")
  set(VISERA_GLOBAL_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"    CACHE PATH "")
  set(VISERA_GLOBAL_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"      CACHE PATH "")
  set(VISERA_GLOBAL_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"     CACHE PATH "")

  macro(install_visera_global in_target)
    message(STATUS "\nInstalling Visera Global...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_GLOBAL_SCRIPTS_DIR})

    include(install_onetbb)
    link_onetbb(${in_target})

    file(GLOB_RECURSE VISERA_GLOBAL_MODULES "${VISERA_GLOBAL_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_GLOBAL_GLOBAL_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_global_modules" TYPE CXX_MODULES
        FILES ${VISERA_GLOBAL_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_global(...)
else()
    add_library(${VISERA_GLOBAL} SHARED)
    target_compile_definitions(${VISERA_GLOBAL} PRIVATE VISERA_GLOBAL_BUILD_SHARED)
    add_library(Visera::Global ALIAS ${VISERA_GLOBAL})

    set_target_properties(${VISERA_GLOBAL} PROPERTIES
        GLOBAL_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
    if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(
        TARGET Visera::Global
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_PDB_FILE:Visera::Global>"
        "${VISERA_APP_FRAMEWORK_DIR}"
    )
    endif()

    if(NOT TARGET Visera::Core)
        message(FATAL_ERROR "Visera-Core is not installed!")
    endif()
    target_link_libraries(${VISERA_GLOBAL} PUBLIC Visera::Core)

    install_visera_global(${VISERA_GLOBAL})
    set_target_properties(${VISERA_GLOBAL} PROPERTIES FOLDER "Visera/Global")
endif()