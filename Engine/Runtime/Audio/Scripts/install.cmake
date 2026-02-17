set(VISERA_AUDIO_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_AUDIO_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_AUDIO_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_AUDIO_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_audio in_target)
    message(STATUS "\nInstalling Visera Audio...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_AUDIO_SCRIPTS_DIR})

    include(install_miniaudio)
    link_miniaudio(${in_target})

    include(install_wwise)
    link_wwise(${in_target})

    file(GLOB_RECURSE VISERA_AUDIO_MODULES "${VISERA_AUDIO_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        $<BUILD_INTERFACE:${VISERA_AUDIO_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Runtime/Audio>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_audio_modules" TYPE CXX_MODULES
        FILES ${VISERA_AUDIO_MODULES})
endmacro()
