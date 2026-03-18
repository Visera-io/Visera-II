if(NOT VISERA_RUNTIME_EXTERNAL_DIR)
    message(FATAL_ERROR "please set VISERA_RUNTIME_EXTERNAL_DIR (e.g. from Runtime/CMakeLists.txt)")
endif()

macro(link_v8 in_target)
    message(STATUS "\nLinking V8 (ViseraEXT::V8)")

    if(NOT TARGET ViseraEXT::V8)
        add_subdirectory(${VISERA_RUNTIME_EXTERNAL_DIR}/V8)
    endif()

    target_link_libraries(${in_target} PRIVATE ViseraEXT::V8)
endmacro()
