message(STATUS "\nInstalling Visera Runtime...")

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
else()
    add_compile_definitions (NOMINMAX)
    add_compile_options     (/utf-8)
endif()

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(VISERA_RUNTIME_ROOT_DIR     ${PROJECT_SOURCE_DIR})
set(VISERA_RUNTIME_SOURCE_DIR   "${VISERA_RUNTIME_ROOT_DIR}/Source")
set(VISERA_RUNTIME_EXTERNAL_DIR "${VISERA_RUNTIME_ROOT_DIR}/External")
set(VISERA_RUNTIME_GLOBAL_DIR   "${VISERA_RUNTIME_ROOT_DIR}/Global")
set(VISERA_RUNTIME_SCRIPTS_DIR  "${VISERA_RUNTIME_ROOT_DIR}/Scripts")

add_library(${VISERA_RUNTIME})
add_library(Visera::Runtime ALIAS ${VISERA_RUNTIME})

if(NOT TARGET Visera::Core)
    message(FATAL_ERROR "Visera-Core is not installed!")
endif()
target_link_libraries(${VISERA_RUNTIME} PRIVATE Visera::Core)

#
# << Install External Packages >>
#

list(APPEND CMAKE_MODULE_PATH ${VISERA_RUNTIME_SCRIPTS_DIR})

include(install_glfw)
link_glfw(${VISERA_RUNTIME})

#
#
#
file(GLOB_RECURSE VISERA_RUNTIME_MODULES "${VISERA_RUNTIME_SOURCE_DIR}/*.ixx")

target_include_directories(${VISERA_RUNTIME}
                           PUBLIC
                           ${VISERA_RUNTIME_GLOBAL_DIR})

target_sources(${VISERA_RUNTIME}
               PUBLIC
               FILE_SET "visera_runtime_modules" TYPE CXX_MODULES
               FILES ${VISERA_RUNTIME_MODULES})