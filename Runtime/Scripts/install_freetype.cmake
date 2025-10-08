if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_freetype in_target)
    message(STATUS "\nLinking FreeType (freetype)")

    if(NOT TARGET freetype)
        # Use Zlib from Visera-Core instead of finding system Zlib
        if(NOT TARGET ZLIB::ZLIB)
            message(FATAL_ERROR "ZLIB::ZLIB target not found. Please ensure Visera-Core is installed before Visera-Runtime.")
        endif()

        # Don’t call find_package(ZLIB) again
        set(FT_DISABLE_ZLIB FALSE CACHE BOOL "" FORCE)
        set(FT_REQUIRE_ZLIB TRUE  CACHE BOOL "" FORCE)

        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/FreeType)
    endif()

    target_link_libraries(${in_target} PUBLIC freetype)
endmacro()