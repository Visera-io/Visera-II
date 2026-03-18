if(NOT VISERA_PLATFORM_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_glfw in_target)
    message(STATUS "\nLinking GLFW (glfw)")
    
    if(NOT TARGET glfw)
        set(BUILD_SHARED_LIBS OFF)
        set(GLFW_BUILD_DOCS   "Build the GLFW documentation"    OFF)
        set(GLFW_INSTALL      "Generate installation target"    OFF)
        add_subdirectory(${VISERA_PLATFORM_EXTERNAL_DIR}/GLFW)
        set_target_properties(glfw PROPERTIES FOLDER "Visera/Platform/External/GLFW")
        set_target_properties(update_mappings PROPERTIES FOLDER "Visera/Platform/External/GLFW")

    endif()

    target_link_libraries(${in_target} PRIVATE "$<BUILD_INTERFACE:glfw>")
endmacro()