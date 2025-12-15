cmake_minimum_required(VERSION 4.0.0)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(WWISE_BUILD_TYPE "Release")
else()
    set(WWISE_BUILD_TYPE "Debug")
endif()

if(WIN32)
    set(WWISE_PLATFORM  "Windows")
    set(LIB_PREFIX      "")
    set(LIB_EXTNAME     ".lib")
elseif(APPLE)
    set(WWISE_PLATFORM  "MacOS")
    set(LIB_PREFIX      "lib")
    set(LIB_EXTNAME     ".a")
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

set(WWISE_ARCHIVE       "${CMAKE_CURRENT_SOURCE_DIR}/${WWISE_PLATFORM}/${WWISE_BUILD_TYPE}.zip")
set(WWISE_EXTRACT_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/${WWISE_PLATFORM}")

set(WWISE_SOUNDENGINE_LIB    "${WWISE_EXTRACT_DIR}/${WWISE_BUILD_TYPE}/${LIB_PREFIX}AkSoundEngine${LIB_EXTNAME}")
set(WWISE_MEMORYMGR_LIB      "${WWISE_EXTRACT_DIR}/${WWISE_BUILD_TYPE}/${LIB_PREFIX}AkMemoryMgr${LIB_EXTNAME}")
set(WWISE_SPATIALAUDIO_LIB   "${WWISE_EXTRACT_DIR}/${WWISE_BUILD_TYPE}/${LIB_PREFIX}AkSpatialAudio${LIB_EXTNAME}")
set(WWISE_STREAMMGR_LIB      "${WWISE_EXTRACT_DIR}/${WWISE_BUILD_TYPE}/${LIB_PREFIX}AkStreamMgr${LIB_EXTNAME}")
if(NOT WWISE_BUILD_TYPE STREQUAL "Release")
set(WWISE_COMMUNICATIONCENTRAL_LIB "${WWISE_EXTRACT_DIR}/${WWISE_BUILD_TYPE}/${LIB_PREFIX}CommunicationCentral${LIB_EXTNAME}")
endif()

if(NOT EXISTS "${WWISE_EXTRACT_DIR}/${WWISE_BUILD_TYPE}")
    message(STATUS "Extracting ${WWISE_ARCHIVE}")
    file(ARCHIVE_EXTRACT
         INPUT       "${WWISE_ARCHIVE}"
         DESTINATION "${WWISE_EXTRACT_DIR}"
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
if(NOT WWISE_BUILD_TYPE STREQUAL "Release")
if(NOT EXISTS "${WWISE_COMMUNICATIONCENTRAL_LIB}")
    message(FATAL_ERROR "Wwise CommunicationCentral library not found: ${WWISE_COMMUNICATIONCENTRAL_LIB}")
endif()
endif()

# Create imported targets for all Wwise libraries
add_library(Wwise_SoundEngine STATIC IMPORTED)
set_target_properties(Wwise_SoundEngine PROPERTIES
    IMPORTED_LOCATION ${WWISE_SOUNDENGINE_LIB}
)
message(STATUS "Sound Engine: ${WWISE_SOUNDENGINE_LIB}")

add_library(Wwise_MemoryMgr STATIC IMPORTED)
set_target_properties(Wwise_MemoryMgr PROPERTIES
    IMPORTED_LOCATION ${WWISE_MEMORYMGR_LIB}
)
message(STATUS "Memory Manager: ${WWISE_MEMORYMGR_LIB}")

add_library(Wwise_SpatialAudio STATIC IMPORTED)
set_target_properties(Wwise_SpatialAudio PROPERTIES
    IMPORTED_LOCATION ${WWISE_SPATIALAUDIO_LIB}
)
message(STATUS "Spatial Audio: ${WWISE_SPATIALAUDIO_LIB}")

add_library(Wwise_StreamMgr STATIC IMPORTED)
set_target_properties(Wwise_StreamMgr PROPERTIES
    IMPORTED_LOCATION ${WWISE_STREAMMGR_LIB}
)
message(STATUS "Stream Manager: ${WWISE_STREAMMGR_LIB}")

if(NOT WWISE_BUILD_TYPE STREQUAL "Release")
add_library(Wwise_CommunicationCentral STATIC IMPORTED)
set_target_properties(Wwise_CommunicationCentral PROPERTIES
    IMPORTED_LOCATION ${WWISE_COMMUNICATIONCENTRAL_LIB}
)
message(STATUS "Communication: ${WWISE_COMMUNICATIONCENTRAL_LIB}")
endif()

# Create a meta-target that links all Wwise components
add_library(Wwise INTERFACE)
target_link_libraries(Wwise INTERFACE
    Wwise_SoundEngine
    Wwise_MemoryMgr
    Wwise_SpatialAudio
    Wwise_StreamMgr
)
if(WWISE_BUILD_TYPE STREQUAL "Release")
# You must define the AK_OPTIMIZED symbol in the release configuration
# of your project (it might be called "Ship", "Retail", or something
# similar in the configuration used to build the retail version of your game).
# This symbol is used in various places to avoid compiling unnecessary code
# in the release version.
target_compile_definitions(Wwise INTERFACE AK_OPTIMIZED)
else()
target_link_libraries(Wwise INTERFACE Wwise_CommunicationCentral)
endif()

target_include_directories(Wwise INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Include")

add_library(WwiseSamples STATIC)
if(WIN32)
    file(GLOB_RECURSE WWISE_SAMPLES_PLATFORM "${CMAKE_CURRENT_SOURCE_DIR}/Include/AKSamples/SoundEngine/Win32/*.cpp")
    set(WwiseSoundEngineIncludes "${CMAKE_CURRENT_SOURCE_DIR}/Include/AKSamples/SoundEngine/Win32")
else()
    file(GLOB_RECURSE WWISE_SAMPLES_PLATFORM "${CMAKE_CURRENT_SOURCE_DIR}/Include/AKSamples/SoundEngine/POSIX/*.cpp")
    set(WwiseSoundEngineIncludes "${CMAKE_CURRENT_SOURCE_DIR}/Include/AKSamples/SoundEngine/POSIX")
endif()
file(GLOB_RECURSE WWISE_SAMPLES_COMMON "${CMAKE_CURRENT_SOURCE_DIR}/Include/AKSamples/SoundEngine/Common/*.cpp")
target_include_directories(WwiseSamples PRIVATE ${WwiseSoundEngineIncludes})
target_sources(WwiseSamples PRIVATE
        ${WWISE_SAMPLES_COMMON}
        ${WWISE_SAMPLES_PLATFORM}
)
target_compile_definitions(WwiseSamples PRIVATE UNICODE)
target_link_libraries(WwiseSamples PRIVATE Wwise)