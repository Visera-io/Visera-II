set(VISERA_CORE_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"     CACHE PATH "")
set(VISERA_CORE_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"   CACHE PATH "")
set(VISERA_CORE_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"     CACHE PATH "")
set(VISERA_CORE_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"    CACHE PATH "")

macro(install_visera_core in_target)
    message(STATUS "\nInstalling Visera Core...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_CORE_SCRIPTS_DIR})

    include(install_spdlog)
    link_spdlog(${in_target})

    include(install_json)
    link_json(${in_target})

    include(install_eigen)
    link_eigen(${in_target})

    include(install_zlib)
    link_zlib(${in_target})

    include(install_ankerl)
    link_ankerl(${in_target})

    file(GLOB_RECURSE VISERA_CORE_MODULES "${VISERA_CORE_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
            PUBLIC
            ${VISERA_CORE_GLOBAL_DIR})

    target_sources(${in_target}
            PUBLIC
            FILE_SET "visera_core_modules" TYPE CXX_MODULES
            FILES ${VISERA_CORE_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    message(STATUS "\n[Monolithic Mode]: please call \"install_visera_core\".")
    #install_visera_core(...)
else()
    add_library(${VISERA_CORE} SHARED)
    target_compile_definitions(${VISERA_CORE} PRIVATE VISERA_CORE_BUILD_SHARED)
    add_library(Visera::Core ALIAS ${VISERA_CORE})

    set_target_properties(${VISERA_CORE} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
    if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(
        TARGET Visera::Core
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_PDB_FILE:Visera::Core>"
        "${VISERA_APP_FRAMEWORK_DIR}"
    )
    endif()

    install_visera_core(${VISERA_CORE})
    set_target_properties(${VISERA_CORE} PROPERTIES FOLDER "Visera/Core")
endif()