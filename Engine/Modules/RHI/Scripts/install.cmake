set(VISERA_RHI_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"     CACHE PATH "")
set(VISERA_RHI_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"   CACHE PATH "")
set(VISERA_RHI_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"     CACHE PATH "")
set(VISERA_RHI_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"    CACHE PATH "")

macro(install_visera_rhi in_target)
    message(STATUS "\nInstalling Visera RHI...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_RHI_SCRIPTS_DIR})

    file(GLOB_RECURSE VISERA_RHI_MODULES "${VISERA_RHI_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_RHI_GLOBAL_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_rhi_modules" TYPE CXX_MODULES
        FILES ${VISERA_RHI_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_rhi(...)
else()
    add_library(${VISERA_RHI} SHARED)
    target_compile_definitions(${VISERA_RHI} PRIVATE VISERA_RHI_BUILD_SHARED)
    add_library(Visera::RHI ALIAS ${VISERA_RHI})

    set_target_properties(${VISERA_RHI} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
   if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(
       TARGET Visera::RHI
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "$<TARGET_PDB_FILE:Visera::RHI>"
       "${VISERA_APP_FRAMEWORK_DIR}"
   )
   endif()

    if(NOT TARGET Visera::Runtime)
        message(FATAL_ERROR "Visera-Runtime is not installed!")
    endif()
    target_link_libraries(${VISERA_RHI} PRIVATE Visera::Runtime)

    install_visera_rhi(${VISERA_RHI})
    set_target_properties(${VISERA_RHI} PROPERTIES FOLDER "Visera/RHI")
endif()