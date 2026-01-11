set(VISERA_PLATFORM_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"     CACHE PATH "")
set(VISERA_PLATFORM_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"   CACHE PATH "")
set(VISERA_PLATFORM_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"     CACHE PATH "")
set(VISERA_PLATFORM_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"    CACHE PATH "")

macro(install_visera_platform in_target)
    message(STATUS "\nInstalling Visera Platform...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_PLATFORM_SCRIPTS_DIR})
    
    if(NOT VISERA_OFFSCREEN_MODE)
    include(install_glfw)
    link_glfw(${in_target})
    endif()

    file(GLOB_RECURSE VISERA_PLATFORM_MODULES "${VISERA_PLATFORM_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_PLATFORM_GLOBAL_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_platform_modules" TYPE CXX_MODULES
        FILES ${VISERA_PLATFORM_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_platform(...)
else()
    add_library(${VISERA_PLATFORM} SHARED)
    target_compile_definitions(${VISERA_PLATFORM} PRIVATE VISERA_PLATFORM_BUILD_SHARED)
    add_library(Visera::Platform ALIAS ${VISERA_PLATFORM})

    set_target_properties(${VISERA_PLATFORM} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
   if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(
       TARGET Visera::Platform
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "$<TARGET_PDB_FILE:Visera::Platform>"
       "${VISERA_APP_FRAMEWORK_DIR}"
   )
   endif()

    if(NOT TARGET Visera::Global)
        message(FATAL_ERROR "Visera-Global is not installed!")
    endif()
    target_link_libraries(${VISERA_PLATFORM} PRIVATE Visera::Global)

    install_visera_platform(${VISERA_PLATFORM})
    set_target_properties(${VISERA_PLATFORM} PROPERTIES FOLDER "Visera/Platform")
endif()