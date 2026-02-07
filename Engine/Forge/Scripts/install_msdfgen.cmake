if(NOT VISERA_FORGE_EXTERNAL_DIR)
    message(FATAL_ERROR "Please include 'install.cmake' before installing any package!")
endif()

macro(link_msdfgen in_target)
    message(STATUS "Linking MSDFGen (MSDFGen)")

    if(NOT TARGET MSDFGen)
        file(GLOB MSDFGEN_CORE_SOURCES "${VISERA_FORGE_EXTERNAL_DIR}/MSDFGen/core/*.cpp")
        add_library(MSDFGen STATIC
            "${VISERA_FORGE_EXTERNAL_DIR}/MSDFGen/msdfgen.h"
            ${MSDFGEN_CORE_SOURCES}
        )

        target_include_directories(MSDFGen PUBLIC "${VISERA_FORGE_EXTERNAL_DIR}/MSDFGen")
        target_compile_definitions(MSDFGen PUBLIC MSDFGEN_PUBLIC=)

        set_target_properties(MSDFGen PROPERTIES FOLDER "Visera/Forge/External/MSDFGen")
    endif()

    target_link_libraries(${in_target} PRIVATE MSDFGen)
endmacro()
