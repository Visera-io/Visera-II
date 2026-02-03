if(NOT VISERA_ASSETHUB_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_openexr in_target)
    message(STATUS "\nLinking OpenEXR (OpenEXR)")
    
    if(NOT TARGET OpenEXR)
        # Use Imath from Visera-Core instead of finding system Zlib
        if(NOT TARGET Imath::Imath)
            message(FATAL_ERROR "Imath::Imath target not found. Please ensure Visera-Core is installed before Visera-Runtime.")
        endif()

        if(NOT TARGET openjph)
            message(FATAL_ERROR "openjph target not found. Please ensure OpenJPH is installed in Visera-AssetHub.")
        endif()

        # Configure OpenEXR to find Core's libdeflate via find_package
        if(TARGET libdeflate::libdeflate_static)
            # Get the build directory of libdeflate where libdeflate-config.cmake is generated
            get_target_property(_libdeflate_binary_dir libdeflate_static BINARY_DIR)
            if(_libdeflate_binary_dir)
                # find_package(libdeflate CONFIG) with libdeflate_DIR set will look for:
                # - libdeflate_DIR/libdeflate-config.cmake
                # - libdeflate_DIR/libdeflate/libdeflate-config.cmake
                # The config file is generated in the build directory by configure_package_config_file
                # So we set libdeflate_DIR to the build directory where the config file exists
                set(libdeflate_DIR "${_libdeflate_binary_dir}" CACHE PATH "Path to libdeflate config" FORCE)
                message(STATUS "Setting libdeflate_DIR to ${libdeflate_DIR} for OpenEXR to find Core's libdeflate")
                # Don't force internal deflate, let OpenEXR find it via find_package
                set(OPENEXR_FORCE_INTERNAL_DEFLATE  OFF     CACHE BOOL "" FORCE)
            else()
                message(WARNING "Could not determine libdeflate build directory, OpenEXR will use internal vendored libdeflate")
                set(OPENEXR_FORCE_INTERNAL_DEFLATE  ON      CACHE BOOL "" FORCE)
            endif()
        else()
            message(STATUS "libdeflate::libdeflate_static not found, OpenEXR will use internal vendored libdeflate")
            set(OPENEXR_FORCE_INTERNAL_DEFLATE  ON      CACHE BOOL "" FORCE)
        endif()

        # Visera build OpenJPH + Imath as sibling subprojects
        set(OPENEXR_IS_SUBPROJECT           ON      CACHE BOOL "" FORCE)
        set(OPENEXR_FORCE_INTERNAL_IMATH    ON      CACHE BOOL "" FORCE)
        set(OPENEXR_FORCE_INTERNAL_OPENJPH  ON      CACHE BOOL "" FORCE)
        set(EXR_OPENJPH_LIB                 openjph                    )

        set(OPENEXR_BUILD_TOOLS             OFF     CACHE BOOL "" FORCE)
        set(OPENEXR_BUILD_EXAMPLES          OFF     CACHE BOOL "" FORCE)
        set(OPENEXR_INSTALL                 OFF     CACHE BOOL "" FORCE)
        set(OPENEXR_INSTALL_TOOLS           OFF     CACHE BOOL "" FORCE)
        set(OPENEXR_INSTALL_DEVELOPER_TOOLS OFF     CACHE BOOL "" FORCE)

        add_subdirectory(${VISERA_ASSETHUB_EXTERNAL_DIR}/OpenEXR)
        set_property(TARGET OpenEXR     PROPERTY CXX_STANDARD 20)
        set_property(TARGET OpenEXRCore PROPERTY CXX_STANDARD 20)
        set_target_properties(OpenEXR PROPERTIES FOLDER "${VISERA_ASSETHUB_EXTERNAL_DIR}/OpenEXR")
    endif()

    target_link_libraries(${in_target} PRIVATE OpenEXR)
endmacro()