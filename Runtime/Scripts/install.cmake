  set(VISERA_RUNTIME_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source"      CACHE PATH "")
  set(VISERA_RUNTIME_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External"    CACHE PATH "")
  set(VISERA_RUNTIME_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global"      CACHE PATH "")
  set(VISERA_RUNTIME_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts"     CACHE PATH "")

  macro(install_visera_runtime in_target)
    message(STATUS "\nInstalling Visera Runtime...")

    list(APPEND CMAKE_MODULE_PATH ${VISERA_RUNTIME_SCRIPTS_DIR})

    include(install_dotnet)
    link_dotnet(${in_target})

    include(install_onetbb)
    link_onetbb(${in_target})

    if(NOT VISERA_OFFSCREEN_MODE)
        include(install_glfw)
        link_glfw(${in_target})
    endif()

    include(install_vma)
    link_vma(${in_target})

    include(install_vulkan)
    link_vulkan(${in_target})

    include(install_stb)
    link_stb(${in_target})

    include(install_libpng)
    link_libpng(${in_target})

    include(install_miniaudio)
    link_miniaudio(${in_target})

    include(install_bvh)
    link_bvh(${in_target})

    include(install_box2d)
    link_box2d(${in_target})

    file(GLOB_RECURSE VISERA_RUNTIME_MODULES "${VISERA_RUNTIME_SOURCE_DIR}/*.ixx")

    target_include_directories(${in_target}
        PUBLIC
        ${VISERA_RUNTIME_GLOBAL_DIR})

    target_sources(${in_target}
        PUBLIC
        FILE_SET "visera_runtime_modules" TYPE CXX_MODULES
        FILES ${VISERA_RUNTIME_MODULES})
endmacro()

if(VISERA_MONOLITHIC_MODE)
    message(STATUS "\n[Monolithic Mode]: please call \"install_visera_runtime\".")
    #install_visera_runtime(...)
else()
    add_library(${VISERA_RUNTIME} SHARED)
    target_compile_definitions(${VISERA_RUNTIME} PRIVATE VISERA_RUNTIME_BUILD_SHARED)
    add_library(Visera::Runtime ALIAS ${VISERA_RUNTIME})

    set_target_properties(${VISERA_RUNTIME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${VISERA_APP_FRAMEWORK_DIR}"
    )
    if(MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(
            TARGET Visera::Runtime
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_PDB_FILE:Visera::Runtime>"
            "${VISERA_APP_FRAMEWORK_DIR}"
        )
    endif()

    if(NOT TARGET Visera::Core)
        message(FATAL_ERROR "Visera-Core is not installed!")
    endif()
    target_link_libraries(${VISERA_RUNTIME} PUBLIC Visera::Core)

    install_visera_runtime(${VISERA_RUNTIME})
    set_target_properties(${VISERA_RUNTIME} PROPERTIES FOLDER "Visera/Runtime")
endif()