if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_ankerl in_target)
    message(STATUS "\nLinking Ankerl (unordered_dense::unordered_dense)")

    if(NOT TARGET unordered_dense::unordered_dense)
        add_subdirectory(${VISERA_CORE_EXTERNAL_DIR}/Ankerl)
    endif()
    target_link_libraries(${in_target} PUBLIC unordered_dense::unordered_dense)
endmacro()