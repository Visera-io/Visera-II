if(NOT VISERA_PLATFORM_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_glfw in_target)
    message(STATUS "\nLinking GLFW (glfw)")
    
    if(NOT TARGET glfw)
        if(VISERA_MONOLITHIC_MODE)
        set(BUILD_SHARED_LIBS ""                                OFF)
        else()
        set(BUILD_SHARED_LIBS ""                                ON)
        endif()
        set(GLFW_BUILD_DOCS   "Build the GLFW documentation"    OFF)
        set(GLFW_INSTALL      "Generate installation target"    OFF)
        add_subdirectory(${VISERA_PLATFORM_EXTERNAL_DIR}/GLFW)
        set_target_properties(glfw PROPERTIES FOLDER "Visera/Platform/External/GLFW")
        set_target_properties(update_mappings PROPERTIES FOLDER "Visera/Platform/External/GLFW")

        if(BUILD_SHARED_LIBS)
        add_custom_command(
            TARGET ${in_target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:glfw>
            "${VISERA_APP_FRAMEWORK_DIR}"
        )
        endif()
    endif()

    target_link_libraries(${in_target} PUBLIC glfw)
endmacro()