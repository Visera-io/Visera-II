set(VISERA_CORE_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"   CACHE STRING "")
set(VISERA_CORE_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External" CACHE STRING "")
set(VISERA_CORE_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/Include"  CACHE STRING "")
set(VISERA_CORE_CMAKE_DIR    "${PROJECT_SOURCE_DIR}/CMake"  CACHE STRING "")

macro(install_visera_core in_target)
    message(STATUS "\nInstalling Visera Core...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_CORE_CMAKE_DIR})

    include(install_charted)
    link_charted(${in_target})

    include(install_spdlog)
    link_spdlog(${in_target})

    include(install_eigen)
    link_eigen(${in_target})

    include(install_imath)
    link_imath(${in_target})

    include(install_zlib)
    link_zlib(${in_target})

    include(install_libdeflate)
    link_libdeflate(${in_target})

    include(install_stb)
    link_stb(${in_target})

    include(install_simdutf)
    link_simdutf(${in_target})

    include(install_ankerl)
    link_ankerl(${in_target})

    include(install_doubleconversion)
    link_doubleconversion(${in_target})

    file(GLOB_RECURSE VISERA_CORE_MODULES "${VISERA_CORE_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        $<BUILD_INTERFACE:${VISERA_CORE_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${VISERA_CORE_EXTERNAL_DIR}/Spdlog/include>
        $<BUILD_INTERFACE:${VISERA_CORE_EXTERNAL_DIR}/Ankerl/include>
        $<BUILD_INTERFACE:${VISERA_CORE_EXTERNAL_DIR}/Charted/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Visera/Core>)

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_core_modules" TYPE CXX_MODULES
        FILES ${VISERA_CORE_MODULES})
endmacro()
