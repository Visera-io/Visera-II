if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_zlib in_target)
    message(STATUS "\nLinking ZLib (ZLIB::ZLIB)")
    
    if(NOT TARGET zlib)    
        option(ZLIB_BUILD_EXAMPLES "Enable Zlib Examples" OFF)

        set(ZLIB_BUILD_STATIC_LIBS ON   CACHE BOOL " " FORCE)
        set(ZLIB_BUILD_SHARED_LIBS OFF  CACHE BOOL " " FORCE)
        add_subdirectory(${VISERA_CORE_EXTERNAL_DIR}/ZLib)

        #set_property(TARGET zlib PROPERTY FOLDER "dependencies")
        set(ZLIB_LIBRARY zlib PARENT_SCOPE) # For LibPNG
        set(ZLIB_INCLUDE_DIR "${VISERA_CORE_EXTERNAL_DIR}/ZLib" CACHE BOOL " " FORCE)

        # libpng expects zlib to be a modern CMake package, let's make an alias for it
        add_library(ZLIB::ZLIB ALIAS zlib)
        target_include_directories(zlib PUBLIC "${VISERA_CORE_EXTERNAL_DIR}/ZLib")

    endif()

    target_link_libraries(${in_target} PUBLIC ZLIB::ZLIB)

#    add_custom_command(
#        TARGET ${in_target}
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE:ZLIB::ZLIB>
#        $<TARGET_FILE_DIR:${VISERA_APP}>
#    )
endmacro()