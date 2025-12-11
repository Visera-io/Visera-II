if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_glfw in_target)
    message(STATUS "\nLinking GLFW (glfw)")
    
    if(NOT TARGET glfw)
        option(BUILD_SHARED_LIBS "Build shared libraries"       ON)  # Call GLFW in Many Modules
        option(GLFW_BUILD_DOCS   "Build the GLFW documentation" OFF)
        option(GLFW_INSTALL "Generate installation target"      OFF)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/GLFW)
        set_target_properties(glfw PROPERTIES FOLDER "Visera/Runtime/External/GLFW")
        set_target_properties(update_mappings PROPERTIES FOLDER "Visera/Runtime/External/GLFW")
    endif()

    target_link_libraries(${in_target} PRIVATE glfw)

    if(WIN32)
    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:glfw>
        ${VISERA_APP_FRAMEWORK_DIR}
    )
    elseif(APPLE)
    add_custom_command(
            TARGET ${in_target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:glfw>
            ${VISERA_APP_FRAMEWORK_DIR}
    )
    else()
    message(FATAL_ERROR "Unsupported platform!")
    endif()
endmacro()