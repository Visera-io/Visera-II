set(VISERA_RHI_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_RHI_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_RHI_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_RHI_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"  CACHE STRING "")

macro(install_visera_rhi in_target)
    message(STATUS "\nInstalling Visera RHI...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_RHI_SCRIPTS_DIR})

    include(install_vma)
    link_vma(${in_target})

    include(install_vulkan)
    link_vulkan(${in_target})

    file(GLOB_RECURSE VISERA_RHI_MODULES "${VISERA_RHI_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        $<BUILD_INTERFACE:${VISERA_RHI_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Runtime/RHI>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_rhi_modules" TYPE CXX_MODULES
        FILES ${VISERA_RHI_MODULES})
endmacro()
