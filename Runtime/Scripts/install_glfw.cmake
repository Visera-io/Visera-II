if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_glfw in_target)
    message(STATUS "\nLinking GLFW (glfw)")
    
    if(NOT TARGET glfw)
        option(GLFW_BUILD_DOCS  "" OFF)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/GLFW)
    endif()

    target_link_libraries(${in_target} PUBLIC glfw)
endmacro()