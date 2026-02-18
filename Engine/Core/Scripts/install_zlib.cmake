if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_zlib in_target)
    message(STATUS "\nLinking ZLib (ZLIB::ZLIB)")
    
    if(NOT TARGET zlib)    
        option(ZLIB_BUILD_EXAMPLES "Enable Zlib Examples" OFF)
        set(ZLIB_INSTALL           OFF    CACHE BOOL " " FORCE)
        set(ZLIB_BUILD_TESTING     OFF    CACHE BOOL " " FORCE)
        set(ZLIB_BUILD_STATIC      ON     CACHE BOOL " " FORCE)
        set(ZLIB_BUILD_SHARED      OFF    CACHE BOOL " " FORCE)
        add_subdirectory(${VISERA_CORE_EXTERNAL_DIR}/ZLib)

        # For LibPNG set_property(TARGET zlib PROPERTY FOLDER "dependencies")
        set(ZLIB_LIBRARY ZLIB::ZLIBSTATIC)
        set(ZLIB_INCLUDE_DIR "${VISERA_CORE_EXTERNAL_DIR}/ZLib" CACHE STRING " " FORCE)
        if(NOT TARGET ZLIB::ZLIB)
            add_library(ZLIB::ZLIB ALIAS zlibstatic)
        endif()

        #target_include_directories(zlib PUBLIC "${VISERA_CORE_EXTERNAL_DIR}/ZLib")
        #set_target_properties(zlib PROPERTIES FOLDER "Visera/Core/External/ZLib")
        set_target_properties(zlibstatic PROPERTIES FOLDER "Visera/Core/External/ZLib")
    endif()

    target_link_libraries(${in_target} PRIVATE "$<BUILD_INTERFACE:ZLIB::ZLIBSTATIC>")
endmacro()