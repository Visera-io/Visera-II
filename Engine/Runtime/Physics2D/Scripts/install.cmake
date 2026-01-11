set(VISERA_PHYSICS2D_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"     CACHE PATH "")
set(VISERA_PHYSICS2D_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"   CACHE PATH "")
set(VISERA_PHYSICS2D_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"     CACHE PATH "")
set(VISERA_PHYSICS2D_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"    CACHE PATH "")

macro(install_visera_physics2d in_target)
    message(STATUS "\nInstalling Visera Physics2D...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_PHYSICS2D_SCRIPTS_DIR})
    
    include(install_box2d)
    link_box2d(${in_target})

    file(GLOB_RECURSE VISERA_PHYSICS2D_MODULES "${VISERA_PHYSICS2D_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_PHYSICS2D_GLOBAL_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_physics2d_modules" TYPE CXX_MODULES
        FILES ${VISERA_PHYSICS2D_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_physics2d(...)
else()
    add_library(${VISERA_PHYSICS2D} SHARED)
    target_compile_definitions(${VISERA_PHYSICS2D} PRIVATE VISERA_PHYSICS2D_BUILD_SHARED)
    add_library(Visera::Audio ALIAS ${VISERA_PHYSICS2D})

    set_target_properties(${VISERA_PHYSICS2D} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
   if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(
       TARGET Visera::Physics2D
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "$<TARGET_PDB_FILE:Visera::Physics2D>"
       "${VISERA_APP_FRAMEWORK_DIR}"
   )
   endif()

    if(NOT TARGET Visera::Global)
        message(FATAL_ERROR "Visera-Global is not installed!")
    endif()
    target_link_libraries(${VISERA_RHI} PRIVATE Visera::Global)

    install_visera_physics2d(${VISERA_PHYSICS2D})
    set_target_properties(${VISERA_PHYSICS2D} PROPERTIES FOLDER "Visera/Physics2D")
endif()