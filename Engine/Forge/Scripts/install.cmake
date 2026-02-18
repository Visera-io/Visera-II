set(VISERA_FORGE_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_FORGE_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_FORGE_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_FORGE_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_forge in_target)
    message(STATUS "\nInstalling Visera Forge...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_FORGE_SCRIPTS_DIR})

    include(install_abseil)
    link_abseil(${in_target})

    include(install_re2)
    link_re2(${in_target})

    include(install_slang)
    link_slang(${in_target})

    file(GLOB_RECURSE VISERA_FORGE_MODULES "${VISERA_FORGE_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_FORGE_INCLUDE_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_forge_modules" TYPE CXX_MODULES
        FILES ${VISERA_FORGE_MODULES})
endmacro()

# Forge: standalone executable (engine toolchain), not linked into Visera.dll
add_executable(${VISERA_FORGE})

# Output to Toolkit/Forge, with per-config subdirs (Debug/Develop/Release)
set_target_properties(${VISERA_FORGE} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${VISERA_FORGE_OUTPUT_DIR}/$<CONFIG>"
)

add_custom_command(
    TARGET ${VISERA_FORGE}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:${VISERA}>"
    "$<TARGET_FILE_DIR:${VISERA_FORGE}>"
)

if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(
        TARGET ${VISERA_FORGE}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_PDB_FILE:${VISERA_FORGE}>"
        "$<TARGET_FILE_DIR:${VISERA_FORGE}>"
    )
    add_custom_command(
        TARGET ${VISERA_FORGE}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_PDB_FILE:${VISERA}>"
        "$<TARGET_FILE_DIR:${VISERA_FORGE}>"
    )
endif()

target_link_libraries(${VISERA_FORGE} PRIVATE Visera)

install_visera_forge(${VISERA_FORGE})
set_target_properties(${VISERA_FORGE} PROPERTIES FOLDER "Visera/Forge")
