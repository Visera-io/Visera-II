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
        $<BUILD_INTERFACE:${VISERA_ASSETHUB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Runtime/AssetHub>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_assets_modules" TYPE CXX_MODULES
        FILES ${VISERA_ASSETHUB_MODULES})
endmacro()
