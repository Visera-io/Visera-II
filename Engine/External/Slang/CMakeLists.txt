cmake_minimum_required(VERSION 4.0.0)

message(STATUS "Version 2025.19.1")

# ------------------------------------------------------------------
# Detect platform and file naming conventions
# ------------------------------------------------------------------
if (WIN32)
    set(SLANG_PLATFORM "Windows")
    set(SLANG_LIB_NAME "slang.lib")
    set(SLANG_DLL_NAME "slang.dll")
elseif (APPLE)
    set(SLANG_PLATFORM "MacOS")
    set(SLANG_LIB_NAME "libslang.dylib")
    set(SLANG_DLL_NAME "libslang.dylib")
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

set(SLANG_ARCHIVE       "${CMAKE_CURRENT_SOURCE_DIR}/${SLANG_PLATFORM}.zip")
set(SLANG_EXTRACT_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/${SLANG_PLATFORM}")

if(NOT EXISTS "${SLANG_EXTRACT_DIR}")
    message(STATUS "Extracting ${SLANG_ARCHIVE}")
    file(ARCHIVE_EXTRACT
            INPUT       "${SLANG_ARCHIVE}"
            DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

# ------------------------------------------------------------------
# Define paths relative to this script
# ------------------------------------------------------------------
set(SLANG_ROOT_DIR     "${CMAKE_CURRENT_SOURCE_DIR}")
set(SLANG_INCLUDE_DIR  "${SLANG_ROOT_DIR}/Include")
set(SLANG_PLATFORM_DIR "${SLANG_ROOT_DIR}/${SLANG_PLATFORM}")

set(SLANG_LIB_PATH "${SLANG_PLATFORM_DIR}/${SLANG_LIB_NAME}")
set(SLANG_DLL_PATH "${SLANG_PLATFORM_DIR}/${SLANG_DLL_NAME}" PARENT_SCOPE)

# ------------------------------------------------------------------
# Create imported Slang_Core target
# ------------------------------------------------------------------
add_library(Slang_Core SHARED IMPORTED)

message(STATUS "Path: ${SLANG_LIB_PATH}")

set_target_properties(Slang_Core PROPERTIES
    IMPORTED_LOCATION_ALL         "${SLANG_DLL_PATH}"
    IMPORTED_IMPLIB               "${SLANG_LIB_PATH}"
    INTERFACE_INCLUDE_DIRECTORIES "${SLANG_INCLUDE_DIR}"
)
if (WIN32)
    set(SLANG_PDB_PATH "${SLANG_PLATFORM_DIR}/slang.pdb" PARENT_SCOPE)
    set_target_properties(Slang_Core PROPERTIES
        IMPORTED_PDB "${SLANG_PDB_PATH}")
endif()

add_library(Slang INTERFACE)
target_link_libraries(Slang INTERFACE Slang_Core)