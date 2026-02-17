if(NOT VISERA_ASSETHUB_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_freetype in_target)
    message(STATUS "\nLinking FreeType (freetype)")

    if(NOT TARGET freetype)
        # Use Zlib from Visera-Core instead of finding system Zlib
        if(NOT TARGET ZLIB::ZLIB)
            message(FATAL_ERROR "ZLIB::ZLIB target not found. Please ensure Visera-Core is installed before Visera-AssetHub.")
        endif()

        set(SKIP_INSTALL_ALL TRUE CACHE BOOL "Skip FreeType install rules" FORCE)
        set(BUILD_SHARED_LIBS OFF)
        add_subdirectory(${VISERA_ASSETHUB_EXTERNAL_DIR}/FreeType)
        set_target_properties(freetype PROPERTIES FOLDER "Visera/AssetHub/External/FreeType")
    endif()

    target_link_libraries(${in_target} PRIVATE "$<BUILD_INTERFACE:freetype>")
endmacro()