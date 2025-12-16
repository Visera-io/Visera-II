cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

if(VISERA_OFFSCREEN_MODE)
    message(STATUS "Off-Screen Mode")
    add_compile_definitions (VISERA_OFFSCREEN_MODE)
endif()

if(MSVC)
    add_compile_definitions (NOMINMAX)
    add_compile_options     (/utf-8)
endif()

set(VISERA_API_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/API")
set(VISERA_ASSETS_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/Assets")
set(VISERA_CORE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/Modules/Core")
set(VISERA_RUNTIME_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/Modules/Runtime")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(VISERA_DEBUG_MODE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Develop")
    add_compile_definitions(VISERA_DEVELOP_MODE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(VISERA_RELEASE_MODE)
else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

add_library(${VISERA} SHARED)
set_target_properties(${VISERA} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
)
if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
add_custom_command(
    TARGET ${VISERA}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_PDB_FILE:${VISERA}>"
    "${VISERA_APP_FRAMEWORK_DIR}"
)
endif()

add_subdirectory(${VISERA_CORE_DIR})
add_subdirectory(${VISERA_RUNTIME_DIR})
add_subdirectory(${VISERA_API_DIR})
if(VISERA_MONOLITHIC_MODE)
target_compile_definitions(${VISERA} PRIVATE VISERA_MONOLITHIC_MODE)
install_visera_core     (${VISERA})
install_visera_runtime  (${VISERA})
else()
target_link_libraries   (${VISERA} PUBLIC Visera::Runtime)
endif()
install_visera_api      (${VISERA})

add_custom_command(
    TARGET ${VISERA}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${VISERA_ASSETS_DIR}"
    "${VISERA_APP_RESOURCE_DIR}/Assets/Engine"
)