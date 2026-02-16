cmake_minimum_required(VERSION 3.20)

project(charted
    VERSION      1.0.0
    LANGUAGES    CXX
    HOMEPAGE_URL "charted.ljyc.me"
)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Explicitly require the standard
set(CMAKE_CXX_EXTENSIONS        OFF) # Vendor-specific Extensions

option(CHARTED_BUILD_EXAMPLES "Build examples"              OFF)
option(CHARTED_ENABLE_MODULES "Build C++20 module bindings" OFF)

add_library(charted INTERFACE)
target_compile_features(charted INTERFACE cxx_std_20)
target_include_directories(charted INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set(CHARTED_SELECTED_TARGET charted)

if (CHARTED_ENABLE_MODULES)
    if (CMAKE_VERSION VERSION_LESS "3.28")
        message(WARNING "CHARTED_ENABLE_MODULES requires CMake >= 3.28. Module bindings target will be skipped.")
    else()
        add_library(charted_modules)
        target_compile_features(charted_modules PUBLIC cxx_std_20)
        target_include_directories(charted_modules PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )
        target_link_libraries(charted_modules PUBLIC charted)
        target_sources(charted_modules PUBLIC
            FILE_SET "charted_modules"
            TYPE     CXX_MODULES
            FILES
                module/charted-core.cppm
                module/charted-json.cppm
                module/charted.cppm
        )

        set(CHARTED_SELECTED_TARGET charted_modules)
    endif()
endif()

add_library(charted::charted ALIAS ${CHARTED_SELECTED_TARGET})

if (CHARTED_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/overview.cpp")
    add_executable(charted_example examples/overview.cpp)
    target_link_libraries(charted_example PRIVATE charted::charted)

    if (CHARTED_ENABLE_MODULES
        AND NOT CMAKE_VERSION VERSION_LESS "3.28"
        AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/charted_module_main.cpp"
        AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/charted_module.cppm")
        add_executable(charted_module_example)
        target_compile_features(charted_module_example PRIVATE cxx_std_20)
        target_link_libraries(charted_module_example PRIVATE charted::charted)
        target_sources(charted_module_example
            PRIVATE
                examples/charted_module_main.cpp
            PUBLIC
                FILE_SET CXX_MODULES FILES
                    examples/charted_module.cppm
        )
    endif()
endif()
