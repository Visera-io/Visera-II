cmake_minimum_required(VERSION 3.21.0 FATAL_ERROR)

SET(VISERA_CORE_VERSION_MAJOR 1)
SET(VISERA_CORE_VERSION_MINOR 0)
SET(VISERA_CORE_VERSION_PATCH 0)
SET(VISERA_CORE_VERSION "${VISERA_CORE_VERSION_MAJOR}.${VISERA_CORE_VERSION_MINOR}.${VISERA_CORE_VERSION_PATCH}")

set(VISERA_CORE "Visera-Core")
project("${VISERA_CORE} (${VISERA_CORE_VERSION})"
    VERSION         "${VISERA_CORE_VERSION}"
    HOMEPAGE_URL    "www.visera.io"
    LANGUAGES       CXX
)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
      "\n"
      "In-source builds are not allowed.\n"
      "Instead, provide a path to build tree like so:\n"
      "cmake -B <destination>\n"
      "\n"
      "To remove files you accidentally created execute:\n"
      "rm -rf CMakeFiles CMakeCache.txt\n"
    )
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fmodules")

    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-stdlib=libc++" HAS_LIBCPP)

    if (HAS_LIBCPP)
        message(STATUS "Visera: Using Libc++")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -D_LIBCPP_VERSION")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
    else()
        message(FATAL_ERROR "libc++ is recommended in conjunction with clang. Please insteall the libc++ development headers, provided e.g. by the packages 'libc++-dev' and 'libc++abi-dev' on Debian/Ubuntu.")
    endif()
endif()

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

#[Install]
include("Scripts/install.cmake")

add_subdirectory(UTests)