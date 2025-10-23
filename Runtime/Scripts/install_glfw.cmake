if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_glfw in_target)
    message(STATUS "\nLinking GLFW (glfw)")
    
    if(NOT TARGET glfw)
        option(BUILD_SHARED_LIBS "Build shared libraries"       ON) # Call GLFW in Many Modules
        option(GLFW_BUILD_DOCS   "Build the GLFW documentation" ON)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/GLFW)
    endif()

    target_link_libraries(${in_target} PUBLIC glfw)

    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:glfw>
        $<TARGET_FILE_DIR:${VISERA_APP}>
    )
endmacro()