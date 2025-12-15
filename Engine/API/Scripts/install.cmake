set(VISERA_API_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"     CACHE PATH "")
set(VISERA_API_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"   CACHE PATH "")
set(VISERA_API_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"     CACHE PATH "")
set(VISERA_API_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"    CACHE PATH "")
set(VISERA_API_CSHARP_DIR   "${PROJECT_SOURCE_DIR}/CSharp"     CACHE PATH "")

macro(install_visera_api in_target)
    message(STATUS "\nInstalling Visera API...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_API_SCRIPTS_DIR})

    file(GLOB_RECURSE VISERA_API_MODULES "${VISERA_API_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_API_GLOBAL_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_api_modules" TYPE CXX_MODULES
        FILES ${VISERA_API_MODULES})

    add_custom_command(
        TARGET ${in_target}  POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VISERA_API_CSHARP_DIR}/Visera.runtimeconfig.json"
        "${VISERA_APP_FRAMEWORK_DIR}/DotNET/Visera.runtimeconfig.json"
    )
endmacro()