cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

set(VISERA "Visera")
project("${VISERA} (${VISERA_CORE_VERSION})"
        VERSION         "${VISERA_CORE_VERSION}"
        HOMEPAGE_URL    "www.visera.io"
        LANGUAGES       CXX
)
message("Installing ${PROJECT_NAME}")

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
#    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules")
#    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fmodules")
#
#    set(CLANG_MODULE_CACHE "${CMAKE_BINARY_DIR}/clang-modcache")
#    add_compile_options(
#            -fmodules
#            -fmodules-cache-path=${CLANG_MODULE_CACHE})
#
#    include(CheckCXXCompilerFlag)
#    CHECK_CXX_COMPILER_FLAG("-stdlib=libc++" HAS_LIBCPP)
#
#    if (HAS_LIBCPP)
#        message(STATUS "Visera: Using Libc++")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -D_LIBCPP_VERSION")
#        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
#        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
#    else()
#        message(FATAL_ERROR "libc++ is recommended in conjunction with clang. Please insteall the libc++ development headers, provided e.g. by the packages 'libc++-dev' and 'libc++abi-dev' on Debian/Ubuntu.")
#    endif()
else()
    add_compile_definitions (NOMINMAX)
    add_compile_options     (/utf-8)
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/Core")
add_subdirectory("${PROJECT_SOURCE_DIR}/Runtime")
add_subdirectory("${PROJECT_SOURCE_DIR}/Engine")
add_subdirectory("${PROJECT_SOURCE_DIR}/Studio")

add_subdirectory(Apps/${VISERA_APP})