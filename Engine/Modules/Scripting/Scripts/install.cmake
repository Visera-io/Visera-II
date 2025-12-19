set(VISERA_SCRIPTING_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"     CACHE PATH "")
set(VISERA_SCRIPTING_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"   CACHE PATH "")
set(VISERA_SCRIPTING_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"     CACHE PATH "")
set(VISERA_SCRIPTING_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"    CACHE PATH "")

macro(install_visera_scripting in_target)
    message(STATUS "\nInstalling Visera Scripting...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_SCRIPTING_SCRIPTS_DIR})

    include(install_dotnet)
    link_dotnet(${in_target})

    file(GLOB_RECURSE VISERA_SCRIPTING_MODULES "${VISERA_SCRIPTING_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_SCRIPTING_GLOBAL_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_scripting_modules" TYPE CXX_MODULES
        FILES ${VISERA_SCRIPTING_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_scripting(...)
else()
    add_library(${VISERA_SCRIPTING} SHARED)
    target_compile_definitions(${VISERA_SCRIPTING} PRIVATE VISERA_SCRIPTING_BUILD_SHARED)
    add_library(Visera::Scripting ALIAS ${VISERA_SCRIPTING})

    set_target_properties(${VISERA_SCRIPTING} PROPERTIES
        SCRIPTING_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
   if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(
       TARGET Visera::Scripting
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "$<TARGET_PDB_FILE:Visera::Scripting>"
       "${VISERA_APP_FRAMEWORK_DIR}"
   )
   endif()

    if(NOT TARGET Visera::Scripting)
        message(FATAL_ERROR "Visera-Scripting is not installed!")
    endif()
    target_link_libraries(${VISERA_SCRIPTING} PRIVATE Visera::Scripting)

    install_visera_scripting(${VISERA_SCRIPTING})
    set_target_properties(${VISERA_SCRIPTING} PROPERTIES FOLDER "Visera/Scripting")
endif()