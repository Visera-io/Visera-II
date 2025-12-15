if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_glfw in_target)
    message(STATUS "\nLinking GLFW (glfw)")
    
    if(NOT TARGET glfw)
        set(BUILD_SHARED_LIBS OFF)
        option(GLFW_BUILD_DOCS   "Build the GLFW documentation" OFF)
        option(GLFW_INSTALL "Generate installation target"      OFF)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/GLFW)
        set_target_properties(glfw PROPERTIES FOLDER "Visera/Runtime/External/GLFW")
        set_target_properties(update_mappings PROPERTIES FOLDER "Visera/Runtime/External/GLFW")
    endif()

    target_link_libraries(${in_target} PRIVATE glfw)
endmacro()