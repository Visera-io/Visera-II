# V8 prebuilt for engine embedding (extract MacOS.zip / Windows.zip into MacOS/ and Windows/).
# Layout: V8/MacOS/libv8_monolith.a (flat) or V8/MacOS/Release/libv8_monolith.a (zip layout);
#         legacy extract to V8/Release/libv8_monolith.a is also accepted.
#         V8/Windows/Release/v8_monolith.lib (optional Debug/).
# Include: V8/include/ (shared). Use install_v8.cmake to extract zips before add_subdirectory.
#
# Compatible with C++23 and C++ Modules: IMPORTED target only; parent keeps CXX_STANDARD 23.

cmake_minimum_required(VERSION 3.16)
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
  cmake_policy(SET CMP0139 NEW)
endif()

set(_V8_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(_V8_INCLUDE "${_V8_ROOT}/include")

# --- Platform subdir: MacOS, Windows (matches extracted zip layout) ---
if(WIN32)
  set(_V8_LIB_PLATFORM "Windows")
  set(_V8_LIB_NAME "v8_monolith.lib")
elseif(APPLE)
  set(_V8_LIB_PLATFORM "MacOS")
  set(_V8_LIB_NAME "libv8_monolith.a")
else()
  message(FATAL_ERROR "V8 supports only Windows and macOS.")
endif()

set(_V8_LIB_DIR "${_V8_ROOT}/${_V8_LIB_PLATFORM}")

# --- Detect layout: Windows has Release/Debug; MacOS is flat ---
set(_V8_HAS_RELEASE FALSE)
set(_V8_HAS_DEBUG FALSE)
set(_V8_HAS_FLAT FALSE)

if(WIN32)
  if(EXISTS "${_V8_LIB_DIR}/Release/${_V8_LIB_NAME}")
    set(_V8_HAS_RELEASE TRUE)
  endif()
  if(EXISTS "${_V8_LIB_DIR}/Debug/${_V8_LIB_NAME}")
    set(_V8_HAS_DEBUG TRUE)
  endif()
else()
  if(EXISTS "${_V8_LIB_DIR}/${_V8_LIB_NAME}")
    set(_V8_HAS_FLAT TRUE)
  elseif(EXISTS "${_V8_LIB_DIR}/Release/${_V8_LIB_NAME}")
    set(_V8_HAS_RELEASE TRUE)
  elseif(EXISTS "${_V8_ROOT}/Release/${_V8_LIB_NAME}")
    # Older installs extracted MacOS.zip to the V8 root (Release/ at top level).
    set(_V8_LIB_DIR "${_V8_ROOT}/Release")
    set(_V8_HAS_FLAT TRUE)
  endif()
endif()

# --- Validation ---
if(NOT EXISTS "${_V8_INCLUDE}/v8.h")
  message(FATAL_ERROR "V8 include not found: ${_V8_INCLUDE}/v8.h. Extract the V8 archive first.")
endif()
if(WIN32)
  if(NOT _V8_HAS_RELEASE AND NOT _V8_HAS_DEBUG)
    message(FATAL_ERROR "V8 lib not found under ${_V8_LIB_DIR}. Extract Windows.zip first.")
  endif()
else()
  if(NOT _V8_HAS_FLAT AND NOT _V8_HAS_RELEASE)
    message(FATAL_ERROR "V8 lib not found under ${_V8_ROOT}/MacOS (flat or Release/) or ${_V8_ROOT}/Release. Extract MacOS.zip first.")
  endif()
endif()

# --- Imported target ---
add_library(ViseraEXT::V8 STATIC IMPORTED GLOBAL)
set_target_properties(ViseraEXT::V8 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_V8_INCLUDE}"
)

if(_V8_HAS_FLAT)
  set(_V8_LIB_PATH "${_V8_LIB_DIR}/${_V8_LIB_NAME}")
  set_target_properties(ViseraEXT::V8 PROPERTIES
    IMPORTED_LOCATION            "${_V8_LIB_PATH}"
    IMPORTED_LOCATION_DEBUG      "${_V8_LIB_PATH}"
    IMPORTED_LOCATION_RELEASE    "${_V8_LIB_PATH}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${_V8_LIB_PATH}"
    IMPORTED_LOCATION_MINSIZEREL "${_V8_LIB_PATH}"
  )
else()
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
