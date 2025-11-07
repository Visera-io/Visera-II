if(NOT VISERA_STUDIO_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_freetype in_target)
    message(STATUS "\nLinking FreeType (freetype)")

    if(NOT TARGET freetype)
        # Use Zlib from Visera-Core instead of finding system Zlib
        if(NOT TARGET ZLIB::ZLIB)
            message(FATAL_ERROR "ZLIB::ZLIB target not found. Please ensure Visera-Core is installed before Visera-Runtime.")
        endif()

        set(SKIP_INSTALL_ALL TRUE CACHE BOOL "Skip FreeType install rules" FORCE)

        add_subdirectory(${VISERA_STUDIO_EXTERNAL_DIR}/FreeType)
        set_target_properties(freetype PROPERTIES FOLDER "Visera/Studio/External/FreeType")
    endif()

    target_link_libraries(${in_target} PUBLIC freetype)
endmacro()