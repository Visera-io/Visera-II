# Single install for Visera Runtime (aligned with Core/CMake/install.cmake).
# VISERA_RUNTIME_DIR and VISERA_RUNTIME_EXTERNAL_DIR must be set by Runtime/CMakeLists.txt before include().

set(VISERA_RUNTIME_CMAKE_DIR   "${VISERA_RUNTIME_DIR}/CMake" CACHE STRING "")
set(VISERA_RUNTIME_SOURCE_DIR  "${VISERA_RUNTIME_DIR}/Source"   CACHE STRING "")
set(VISERA_RUNTIME_INCLUDE_DIR "${VISERA_RUNTIME_DIR}/Include"   CACHE STRING "")

macro(install_visera_runtime in_target)
    message(STATUS "\nInstalling Visera Runtime (all modules)...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_RUNTIME_CMAKE_DIR})

    include(install_vma)
    link_vma(${in_target})

    include(install_vulkan)
    link_vulkan(${in_target})

    include(install_libpng)
    link_libpng(${in_target})

    include(install_openjph)
    link_openjph(${in_target})

    include(install_openexr)
    link_openexr(${in_target})

    include(install_freetype)
    link_freetype(${in_target})

    include(install_wwise)
    link_wwise(${in_target})

    include(install_imgui)
    link_imgui(${in_target})

    include(install_v8)
    link_v8(${in_target})

    file(GLOB_RECURSE VISERA_RUNTIME_MODULES "${VISERA_RUNTIME_SOURCE_DIR}/*.ixx")
    list(SORT VISERA_RUNTIME_MODULES)

    target_include_directories(${in_target}
        PUBLIC
        $<BUILD_INTERFACE:${VISERA_RUNTIME_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Runtime>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_runtime_modules" TYPE CXX_MODULES
        FILES ${VISERA_RUNTIME_MODULES})
endmacro()
