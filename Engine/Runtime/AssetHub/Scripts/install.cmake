set(VISERA_ASSETHUB_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_ASSETHUB_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_ASSETHUB_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_ASSETHUB_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_assethub in_target)
    message(STATUS "\nInstalling Visera AssetHub...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_ASSETHUB_SCRIPTS_DIR})

    include(install_libpng)
    link_libpng(${in_target})

    include(install_openjph)
    link_openjph(${in_target})

    include(install_openexr)
    link_openexr(${in_target})

    include(install_freetype)
    link_freetype(${in_target})

    file(GLOB_RECURSE VISERA_ASSETHUB_MODULES "${VISERA_ASSETHUB_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_ASSETHUB_INCLUDE_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_assets_modules" TYPE CXX_MODULES
        FILES ${VISERA_ASSETHUB_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    #install_visera_assethub(...)
else()
    add_library(${VISERA_ASSETHUB} SHARED)
    target_compile_definitions(${VISERA_ASSETHUB} PRIVATE VISERA_ASSETHUB_BUILD_SHARED)
    add_library(Visera::AssetHub ALIAS ${VISERA_ASSETHUB})

    set_target_properties(${VISERA_ASSETHUB} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
   if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
   add_custom_command(
       TARGET Visera::AssetHub
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "$<TARGET_PDB_FILE:Visera::AssetHub>"
       "${VISERA_APP_FRAMEWORK_DIR}"
   )
   endif()

    if(NOT TARGET Visera::Global)
        message(FATAL_ERROR "Visera-Global is not installed!")
    endif()
    target_link_libraries(${VISERA_ASSETHUB} PRIVATE Visera::Global)

    if(NOT TARGET Visera::RHI)
        message(FATAL_ERROR "Visera-RHI is not installed! AssetHub.Shader requires RHI.Common.")
    endif()
    target_link_libraries(${VISERA_ASSETHUB} PRIVATE Visera::RHI)

    install_visera_assethub(${VISERA_ASSETHUB})
    set_target_properties(${VISERA_ASSETHUB} PROPERTIES FOLDER "Visera/AssetHub")
endif()