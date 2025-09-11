cmake_minimum_required(VERSION 3.29)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(WWISE_BUILD_TYPE "Debug")
else()
    set(WWISE_BUILD_TYPE "Release")
endif()

if(WIN32)
    set(WWISE_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/Windows/${WWISE_BUILD_TYPE}.zip")
    set(EXTRACT_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/Windows")
elseif(APPLE)
    set(WWISE_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/MacOS/${WWISE_BUILD_TYPE}.zip")
    set(EXTRACT_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/MacOS")
elseif (UNIX)
    set(WWISE_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/Linux/${WWISE_BUILD_TYPE}.zip")
    set(EXTRACT_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/Linux")
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

add_library(Wwise STATIC IMPORTED)
set_target_properties(Wwise PROPERTIES
    IMPORTED_LOCATION
    "${EXTRACT_DIR}/${WWISE_BUILD_TYPE}/libAkSoundEngine.a"
)
target_include_directories(Wwise INTERFACE "Include")