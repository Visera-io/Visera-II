if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please set VISERA_RUNTIME_EXTERNAL_DIR (e.g. from Runtime/CMakeLists.txt)")
endif()

macro(link_openjph in_target)
    message(STATUS "\nLinking OpenJPH (OpenJPH)")

    if(NOT TARGET openjph)
        set(BUILD_SHARED_LIBS           OFF CACHE BOOL "" FORCE)
        set(OJPH_BUILD_EXECUTABLES      OFF CACHE BOOL "" FORCE)
        set(OJPH_ENABLE_TIFF_SUPPORT    OFF CACHE BOOL "" FORCE)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/OpenJPH)
        set_target_properties(openjph PROPERTIES FOLDER "Visera/Runtime/External/OpenJPH")
    endif()

    target_link_libraries(${in_target} PRIVATE "$<BUILD_INTERFACE:openjph>")
endmacro()