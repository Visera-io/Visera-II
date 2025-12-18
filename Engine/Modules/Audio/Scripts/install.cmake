set(VISERA_AUDIO_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"     CACHE PATH "")
set(VISERA_AUDIO_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"   CACHE PATH "")
set(VISERA_AUDIO_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"     CACHE PATH "")
set(VISERA_AUDIO_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"    CACHE PATH "")

macro(install_visera_audio in_target)
    message(STATUS "\nInstalling Visera Audio...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_AUDIO_SCRIPTS_DIR})

    file(GLOB_RECURSE VISERA_AUDIO_MODULES "${VISERA_AUDIO_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_AUDIO_GLOBAL_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_audio_modules" TYPE CXX_MODULES
        FILES ${VISERA_AUDIO_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_audio(...)
else()
    add_library(${VISERA_AUDIO} SHARED)
    target_compile_definitions(${VISERA_AUDIO} PRIVATE VISERA_AUDIO_BUILD_SHARED)
    add_library(Visera::Audio ALIAS ${VISERA_AUDIO})

    set_target_properties(${VISERA_AUDIO} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
   if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(
       TARGET Visera::Audio
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "$<TARGET_PDB_FILE:Visera::Audio>"
       "${VISERA_APP_FRAMEWORK_DIR}"
   )
   endif()

    if(NOT TARGET Visera::Runtime)
        message(FATAL_ERROR "Visera-Runtime is not installed!")
    endif()
    target_link_libraries(${VISERA_AUDIO} PRIVATE Visera::Runtime)

    install_visera_audio(${VISERA_AUDIO})
    set_target_properties(${VISERA_AUDIO} PROPERTIES FOLDER "Visera/Audio")
endif()