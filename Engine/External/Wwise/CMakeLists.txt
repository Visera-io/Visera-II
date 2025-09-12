cmake_minimum_required(VERSION 3.29)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(WWISE_BUILD_TYPE "Debug")
else()
    set(WWISE_BUILD_TYPE "Release")
endif()

if(WIN32)
    set(WWISE_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/Windows/${WWISE_BUILD_TYPE}.zip")
    set(EXTRACT_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/Windows")

    set(WWISE_SOUNDENGINE_LIB   "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/AkSoundEngine.lib")
    set(WWISE_MEMORYMGR_LIB     "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/AkMemoryMgr.lib")
    set(WWISE_SPATIALAUDIO_LIB  "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/AkSpatialAudio.lib")
    set(WWISE_STREAMMGR_LIB     "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/AkStreamMgr.lib")
elseif(APPLE)
    set(WWISE_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/${WWISE_BUILD_TYPE}.zip")
    set(EXTRACT_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/MacOS")

    set(WWISE_SOUNDENGINE_LIB   "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/libAkSoundEngine.a")
    set(WWISE_MEMORYMGR_LIB     "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/libAkMemoryMgr.a")
    set(WWISE_SPATIALAUDIO_LIB  "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/libAkSpatialAudio.a")
    set(WWISE_STREAMMGR_LIB     "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/libAkStreamMgr.a")
elseif (UNIX)
    set(WWISE_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/Linux/${WWISE_BUILD_TYPE}.zip")
    set(EXTRACT_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/Linux")

    set(WWISE_SOUNDENGINE_LIB   "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/libAkSoundEngine.a")
    set(WWISE_MEMORYMGR_LIB     "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/libAkMemoryMgr.a")
    set(WWISE_SPATIALAUDIO_LIB  "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/libAkSpatialAudio.a")
    set(WWISE_STREAMMGR_LIB     "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/libAkStreamMgr.a")
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

if(NOT EXISTS "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}")
    message(STATUS "Extracting ${WWISE_ARCHIVE}")
    file(ARCHIVE_EXTRACT
            INPUT       "${WWISE_ARCHIVE}"
            DESTINATION "${EXTRACT_DIR}"
    )
endif()

# Verify that all required Wwise libraries exist
if(NOT EXISTS "${WWISE_SOUNDENGINE_LIB}")
    message(FATAL_ERROR "Wwise SoundEngine library not found: ${WWISE_SOUNDENGINE_LIB}")
endif()
if(NOT EXISTS "${WWISE_MEMORYMGR_LIB}")
    message(FATAL_ERROR "Wwise MemoryMgr library not found: ${WWISE_MEMORYMGR_LIB}")
endif()
if(NOT EXISTS "${WWISE_SPATIALAUDIO_LIB}")
    message(FATAL_ERROR "Wwise SpatialAudio library not found: ${WWISE_SPATIALAUDIO_LIB}")
endif()
if(NOT EXISTS "${WWISE_STREAMMGR_LIB}")
    message(FATAL_ERROR "Wwise StreamMgr library not found: ${WWISE_STREAMMGR_LIB}")
endif()

# Create imported targets for all Wwise libraries
add_library(Wwise_SoundEngine STATIC IMPORTED)
set_target_properties(Wwise_SoundEngine PROPERTIES
    IMPORTED_LOCATION ${WWISE_SOUNDENGINE_LIB}
)

add_library(Wwise_MemoryMgr STATIC IMPORTED)
set_target_properties(Wwise_MemoryMgr PROPERTIES
    IMPORTED_LOCATION ${WWISE_MEMORYMGR_LIB}
)

add_library(Wwise_SpatialAudio STATIC IMPORTED)
set_target_properties(Wwise_SpatialAudio PROPERTIES
    IMPORTED_LOCATION ${WWISE_SPATIALAUDIO_LIB}
)

add_library(Wwise_StreamMgr STATIC IMPORTED)
set_target_properties(Wwise_StreamMgr PROPERTIES
    IMPORTED_LOCATION ${WWISE_STREAMMGR_LIB}
)

# Create a meta-target that links all Wwise components
add_library(Wwise INTERFACE)
target_link_libraries(Wwise INTERFACE
    Wwise_SoundEngine
    Wwise_MemoryMgr
    Wwise_SpatialAudio
    Wwise_StreamMgr
)

target_include_directories(Wwise INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)