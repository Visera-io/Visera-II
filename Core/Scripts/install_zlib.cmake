if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_zlib in_target)
    message(STATUS "\nLinking ZLib (ZLIB::ZLIB)")
    
    if(NOT TARGET zlib)    
        option(ZLIB_BUILD_EXAMPLES "Enable Zlib Examples" OFF)

        set(ZLIB_BUILD_STATIC_LIBS ON     CACHE BOOL " " FORCE)
        set(ZLIB_BUILD_SHARED_LIBS OFF    CACHE BOOL " " FORCE)
        set(BUILD_SHARED_LIBS OFF)
        add_subdirectory(${VISERA_CORE_EXTERNAL_DIR}/ZLib)

        # For LibPNG set_property(TARGET zlib PROPERTY FOLDER "dependencies")
        if(VISERA_MONOLITHIC_MODE)
            set(ZLIB_LIBRARY zlib)
        else()
            set(ZLIB_LIBRARY zlib PARENT_SCOPE)
        endif()
        set(ZLIB_INCLUDE_DIR "${VISERA_CORE_EXTERNAL_DIR}/ZLib" CACHE BOOL " " FORCE)

        # libpng expects zlib to be a modern CMake package, let's make an alias for it
        add_library(ZLIB::ZLIB ALIAS zlibstatic)
        target_include_directories(zlib PUBLIC "${VISERA_CORE_EXTERNAL_DIR}/ZLib")
        set_target_properties(zlib PROPERTIES FOLDER "Visera/Core/External/ZLib")
        set_target_properties(zlibstatic PROPERTIES FOLDER "Visera/Core/External/ZLib")
    endif()

    target_link_libraries(${in_target} PUBLIC ZLIB::ZLIB)
endmacro()