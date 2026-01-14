if(NOT VISERA_ASSETS_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_libpng in_target)
    message(STATUS "\nLinking LibPNG (libpng)")
    
    if(NOT TARGET png)
        # Use Zlib from Visera-Core instead of finding system Zlib
        if(NOT TARGET ZLIB::ZLIB)
            message(FATAL_ERROR "ZLIB::ZLIB target not found. Please ensure Visera-Core is installed before Visera-Runtime.")
        endif()

        # Set ZLIB variables for LibPNG's find_package(ZLIB) call
        # These are needed because LibPNG's CMakeLists.txt calls find_package(ZLIB REQUIRED)
        # Get include directory from the ZLIB::ZLIB target
        get_target_property(ZLIB_INCLUDE_DIR_TEMP ZLIB::ZLIB INTERFACE_INCLUDE_DIRECTORIES)
        if(ZLIB_INCLUDE_DIR_TEMP)
            # Handle list of include directories
            if(ZLIB_INCLUDE_DIR_TEMP MATCHES ";")
                list(GET ZLIB_INCLUDE_DIR_TEMP 0 ZLIB_INCLUDE_DIR_TEMP)
            endif()
            set(ZLIB_INCLUDE_DIR "${ZLIB_INCLUDE_DIR_TEMP}" CACHE STRING "ZLIB include directory" FORCE)
        else()
            # Fallback: get include directories from zlibstatic target
            get_target_property(ZLIB_INCLUDE_DIR_TEMP zlibstatic INTERFACE_INCLUDE_DIRECTORIES)
            if(ZLIB_INCLUDE_DIR_TEMP)
                if(ZLIB_INCLUDE_DIR_TEMP MATCHES ";")
                    list(GET ZLIB_INCLUDE_DIR_TEMP 0 ZLIB_INCLUDE_DIR_TEMP)
                endif()
                set(ZLIB_INCLUDE_DIR "${ZLIB_INCLUDE_DIR_TEMP}" CACHE STRING "ZLIB include directory" FORCE)
            endif()
        endif()
        # Mark ZLIB as found so find_package uses the existing target
        set(ZLIB_FOUND TRUE CACHE BOOL "ZLIB found" FORCE)

        set(PNG_SHARED OFF CACHE BOOL " " FORCE)
        set(PNG_STATIC ON  CACHE BOOL " " FORCE)
        set(PNG_TESTS  OFF CACHE BOOL " " FORCE)
        set(PNG_SKIP_INSTALL_ALL TRUE)
        set(BUILD_SHARED_LIBS OFF)

        if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
            set(PNG_ARM_NEON "on" CACHE STRING " " FORCE)
        endif()

        #[IMPORTANT]: original CMakeLists.txt was replaced by https://github.com/mitsuba-renderer/libpng/blob/cfcd1dc417f39929c3c540c4f945069cedeee693/CMakeLists.txt
        add_subdirectory(${VISERA_ASSETS_EXTERNAL_DIR}/LibPNG)
        
        # Ensure LibPNG can find the Zlib target from Core
        target_link_libraries(png_static PRIVATE ZLIB::ZLIB)
        set_target_properties(png_static PROPERTIES FOLDER "Visera/Assets/External/LibPNG")
        set_target_properties(png_genfiles PROPERTIES FOLDER "Visera/Assets/External/LibPNG")
    endif()

#    add_custom_command(
#        TARGET ${in_target}
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE:png>
#        $<TARGET_FILE_DIR:${in_target}>
#    )
    target_link_libraries(${in_target} PRIVATE png_static)
endmacro()