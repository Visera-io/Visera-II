if(NOT VISERA_CORE_EXTERNAL_DIR)
    message(FATAL_ERROR "please include 'install.cmake' before installing any package.")
endif()

macro(link_eigen in_target)
    message(STATUS "\nLinking Eigen3 (Eigen)")

    if(NOT TARGET Eigen)
        file(GLOB_RECURSE EIGEN_MODULES ${VISERA_CORE_EXTERNAL_DIR}/Eigen)
        add_library(Eigen INTERFACE)
        target_include_directories(Eigen INTERFACE ${VISERA_CORE_EXTERNAL_DIR}/Eigen)
    endif()

    target_link_libraries(${in_target} PUBLIC Eigen)
endmacro()