if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package!")
endif()

macro(link_imath in_target)
    message(STATUS "\nLinking Imath (Imath::Imath)")

    if(NOT TARGET Imath::Imath)
        set(BUILD_SHARED_LIBS           OFF)
        set(IMATH_BUILD_TESTS           OFF)
        set(IMATH_BUILD_TOOLS           OFF)
        set(IMATH_ENABLE_LOGGING        OFF)
        set(IMATH_INSTALL               OFF)
        set(IMATH_INSTALL_PKG_CONFIG    OFF)
        add_subdirectory("${VISERA_CORE_EXTERNAL_DIR}/Imath")
        # Imath is a subproject with its own CXX.dd; on macOS with -g, dyndep lists .o.dSYM as output
        # but no rule produces it. Use -g0 for this target only so the subproject build succeeds.
        if(APPLE)
            target_compile_options(Imath PRIVATE $<$<CONFIG:Debug>:-g0> $<$<CONFIG:Develop>:-g0>)
        endif()
        set_target_properties(Imath PROPERTIES FOLDER "Visera/Core/External/Imath")
    endif()

    target_link_libraries(${in_target} PRIVATE "$<BUILD_INTERFACE:Imath::Imath>")
    target_include_directories(${in_target} PRIVATE "${VISERA_CORE_EXTERNAL_DIR}/Imath/src")
endmacro()