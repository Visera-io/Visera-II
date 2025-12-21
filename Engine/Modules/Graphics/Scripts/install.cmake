set(VISERA_GRAPHICS_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"     CACHE PATH "")
set(VISERA_GRAPHICS_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"   CACHE PATH "")
set(VISERA_GRAPHICS_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"     CACHE PATH "")
set(VISERA_GRAPHICS_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"    CACHE PATH "")

macro(install_visera_graphics in_target)
    message(STATUS "\nInstalling Visera Graphics...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_GRAPHICS_SCRIPTS_DIR})

    include(install_bvh)
    link_bvh(${in_target})
    
    include(install_freetype)
    link_freetype(${in_target})

    if(NOT VISERA_OFFSCREEN_MODE)
    include(install_imgui)
    link_imgui(${in_target})
    endif()

    file(GLOB_RECURSE VISERA_GRAPHICS_MODULES "${VISERA_GRAPHICS_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_GRAPHICS_GLOBAL_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_graphics_modules" TYPE CXX_MODULES
        FILES ${VISERA_GRAPHICS_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_graphics(...)
else()
    add_library(${VISERA_GRAPHICS} SHARED)
    target_compile_definitions(${VISERA_GRAPHICS} PRIVATE VISERA_GRAPHICS_BUILD_SHARED)
    add_library(Visera::Graphics ALIAS ${VISERA_GRAPHICS})

    set_target_properties(${VISERA_GRAPHICS} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
   if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(
       TARGET Visera::Graphics
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "$<TARGET_PDB_FILE:Visera::Graphics>"
       "${VISERA_APP_FRAMEWORK_DIR}"
   )
   endif()

    if(NOT TARGET Visera::Runtime)
        message(FATAL_ERROR "Visera-Runtime is not installed!")
    endif()
    target_link_libraries(${VISERA_GRAPHICS} PRIVATE Visera::Runtime)

    if(NOT TARGET Visera::Platform)
        message(FATAL_ERROR "Visera-Platform is not installed!")
    endif()
    target_link_libraries(${VISERA_GRAPHICS} PRIVATE Visera::Platform)

    if(NOT TARGET Visera::RHI)
        message(FATAL_ERROR "Visera-RHI is not installed!")
    endif()
    target_link_libraries(${VISERA_GRAPHICS} PRIVATE Visera::RHI)

    if(NOT TARGET Visera::Shader)
        message(FATAL_ERROR "Visera-Shader is not installed!")
    endif()
    target_link_libraries(${VISERA_GRAPHICS} PRIVATE Visera::Shader)

    install_visera_graphics(${VISERA_GRAPHICS})
    set_target_properties(${VISERA_GRAPHICS} PROPERTIES FOLDER "Visera/Graphics")
endif()