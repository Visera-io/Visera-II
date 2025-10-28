if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_libpng in_target)
    message(STATUS "\nLinking LibPNG (libpng)")
    
    if(NOT TARGET png)
        # Use Zlib from Visera-Core instead of finding system Zlib
        if(NOT TARGET ZLIB::ZLIB)
            message(FATAL_ERROR "ZLIB::ZLIB target not found. Please ensure Visera-Core is installed before Visera-Runtime.")
        endif()

        set(PNG_SHARED ON  CACHE BOOL " " FORCE)
        set(PNG_STATIC OFF CACHE BOOL " " FORCE)
        set(PNG_TESTS  OFF CACHE BOOL " " FORCE)
        set(PNG_SKIP_INSTALL_ALL TRUE)

        if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
            set(PNG_ARM_NEON "on" CACHE STRING " " FORCE)
        endif()

        #[IMPORTANT]: original CMakeLists.txt was replaced by https://github.com/mitsuba-renderer/libpng/blob/cfcd1dc417f39929c3c540c4f945069cedeee693/CMakeLists.txt
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/LibPNG)
        
        # Ensure LibPNG can find the Zlib target from Core
        target_link_libraries(png PRIVATE ZLIB::ZLIB)
    endif()
    
    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:png>
        $<TARGET_FILE_DIR:${in_target}>
    )
    target_link_libraries(${in_target} PUBLIC png)
endmacro()