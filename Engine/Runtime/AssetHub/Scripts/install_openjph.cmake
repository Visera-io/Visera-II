if(NOT VISERA_ASSETHUB_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_openjph in_target)
    message(STATUS "\nLinking OpenJPH (OpenJPH)")
    
    if(NOT TARGET openjph)
        set(BUILD_SHARED_LIBS           OFF CACHE BOOL "" FORCE)
        set(OJPH_BUILD_EXECUTABLES      OFF CACHE BOOL "" FORCE)
        set(OJPH_ENABLE_TIFF_SUPPORT    OFF CACHE BOOL "" FORCE)
        add_subdirectory(${VISERA_ASSETHUB_EXTERNAL_DIR}/OpenJPH)
        set_target_properties(openjph PROPERTIES FOLDER "${VISERA_ASSETHUB_EXTERNAL_DIR}/OpenJPH")
    endif()

    target_link_libraries(${in_target} PRIVATE openjph)
endmacro()