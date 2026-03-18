# V8 prebuilt for engine embedding (from visera_ext build v8).
# Multi-arch layout: lib/win64/, lib/macos_x64/, lib/macos_arm64/ (include/ shared).
# Multi-config layout: lib/<platform>/Release/, lib/<platform>/Debug/
# Use: add_subdirectory(path/to/Dist/V8) then target_link_libraries(your_app PRIVATE ViseraEXT::V8)
#
# Compatible with C++23 and C++ Modules: IMPORTED target only; parent keeps CXX_STANDARD 23.

cmake_minimum_required(VERSION 3.16)
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
  cmake_policy(SET CMP0139 NEW)
endif()

set(_V8_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(_V8_INCLUDE "${_V8_ROOT}/include")

# --- Platform subdir: win64, macos_x64, macos_arm64 ---
if(WIN32)
  set(_V8_LIB_PLATFORM "win64")
  set(_V8_LIB_NAME "v8_monolith.lib")
elseif(APPLE)
  if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(_V8_LIB_PLATFORM "macos_arm64")
  else()
    set(_V8_LIB_PLATFORM "macos_x64")
  endif()
  set(_V8_LIB_NAME "libv8_monolith.a")
else()
  message(FATAL_ERROR "V8 Dist supports only Windows and macOS.")
endif()

set(_V8_LIB_DIR "${_V8_ROOT}/lib/${_V8_LIB_PLATFORM}")

# --- Detect layout: multi-config (Release/Debug subdirs) vs legacy (flat) ---
set(_V8_HAS_RELEASE FALSE)
set(_V8_HAS_DEBUG FALSE)
set(_V8_LEGACY FALSE)

if(EXISTS "${_V8_LIB_DIR}/Release/${_V8_LIB_NAME}")
  set(_V8_HAS_RELEASE TRUE)
endif()
if(EXISTS "${_V8_LIB_DIR}/Debug/${_V8_LIB_NAME}")
  set(_V8_HAS_DEBUG TRUE)
endif()

if(NOT _V8_HAS_RELEASE AND NOT _V8_HAS_DEBUG)
  if(EXISTS "${_V8_LIB_DIR}/${_V8_LIB_NAME}")
    set(_V8_LEGACY TRUE)
  elseif(EXISTS "${_V8_ROOT}/lib/${_V8_LIB_NAME}")
    set(_V8_LIB_DIR "${_V8_ROOT}/lib")
    set(_V8_LEGACY TRUE)
  endif()
endif()

# --- Validation ---
if(NOT EXISTS "${_V8_INCLUDE}/v8.h")
  message(FATAL_ERROR "V8 include not found: ${_V8_INCLUDE}/v8.h. Run visera_ext build v8 first.")
endif()
if(NOT _V8_HAS_RELEASE AND NOT _V8_HAS_DEBUG AND NOT _V8_LEGACY)
  message(FATAL_ERROR "V8 lib not found under ${_V8_LIB_DIR}. Run visera_ext build v8 first.")
endif()

# --- Imported target ---
add_library(ViseraEXT::V8 STATIC IMPORTED GLOBAL)
set_target_properties(ViseraEXT::V8 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_V8_INCLUDE}"
)

if(_V8_LEGACY)
  set_target_properties(ViseraEXT::V8 PROPERTIES
    IMPORTED_LOCATION "${_V8_LIB_DIR}/${_V8_LIB_NAME}"
  )
else()
  # Multi-config: set per-config locations for Xcode / VS multi-config generators.
  # IMPORTED_LOCATION (no suffix) is the fallback for single-config generators.
  if(_V8_HAS_RELEASE)
    set_target_properties(ViseraEXT::V8 PROPERTIES
      IMPORTED_LOCATION            "${_V8_LIB_DIR}/Release/${_V8_LIB_NAME}"
      IMPORTED_LOCATION_RELEASE    "${_V8_LIB_DIR}/Release/${_V8_LIB_NAME}"
      IMPORTED_LOCATION_RELWITHDEBINFO "${_V8_LIB_DIR}/Release/${_V8_LIB_NAME}"
      IMPORTED_LOCATION_MINSIZEREL "${_V8_LIB_DIR}/Release/${_V8_LIB_NAME}"
    )
  endif()
  if(_V8_HAS_DEBUG)
    set_target_properties(ViseraEXT::V8 PROPERTIES
      IMPORTED_LOCATION_DEBUG "${_V8_LIB_DIR}/Debug/${_V8_LIB_NAME}"
    )
    if(NOT _V8_HAS_RELEASE)
      set_target_properties(ViseraEXT::V8 PROPERTIES
        IMPORTED_LOCATION "${_V8_LIB_DIR}/Debug/${_V8_LIB_NAME}"
      )
    endif()
  endif()
endif()

# --- V8 ABI compile definitions (must match build config) ---
# Pointer compression is always ON for 64-bit V8 builds.
set_property(TARGET ViseraEXT::V8 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
  V8_COMPRESS_POINTERS
)
# When Debug lib is present, enable V8 DCHECKs for consumer Debug builds.
if(_V8_HAS_DEBUG)
  set_property(TARGET ViseraEXT::V8 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
    $<$<CONFIG:Debug>:V8_ENABLE_CHECKS>
  )
endif()

# --- System libraries ---
if(WIN32)
  set_property(TARGET ViseraEXT::V8 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
    winmm
    dbghelp
  )
endif()
