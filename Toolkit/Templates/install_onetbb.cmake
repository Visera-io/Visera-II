if(NOT VISERA_GLOBAL_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package.")
endif()

macro(link_onetbb in_target)
    message(STATUS "\nLinking OneTBB (TBB::tbb)")
    
    if(NOT TARGET TBB::tbb)
        set(BUILD_SHARED_LIBS   ON)
        set(TBB_TEST            OFF CACHE BOOL "" FORCE)
        set(TBB_EXAMPLES        OFF CACHE BOOL "" FORCE)
        add_subdirectory(${VISERA_GLOBAL_EXTERNAL_DIR}/OneTBB)
        set_target_properties(tbb PROPERTIES FOLDER "Visera/Runtime/External/OneTBB")
        set_target_properties(tbbmalloc PROPERTIES FOLDER "Visera/Runtime/External/OneTBB")
        set_target_properties(tbbmalloc_proxy PROPERTIES FOLDER "Visera/Runtime/External/OneTBB")
    endif()
    target_link_libraries(${in_target} PRIVATE TBB::tbb)

    add_custom_command(
        TARGET ${in_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:TBB::tbb>
        ${VISERA_APP_FRAMEWORK_DIR}
    )
endmacro()