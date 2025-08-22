message(STATUS "\nInstalling Visera Engine...")

set(VISERA_ENGINE_SOURCE_DIR   "${PROJECT_SOURCE_DIR}/Source")
set(VISERA_ENGINE_EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External")
set(VISERA_ENGINE_GLOBAL_DIR   "${PROJECT_SOURCE_DIR}/Global")
set(VISERA_ENGINE_SCRIPTS_DIR  "${PROJECT_SOURCE_DIR}/Scripts")

add_library(${VISERA_ENGINE})
add_library(Visera::Engine ALIAS ${VISERA_ENGINE})

if(${VISERA_APP})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${VISERA_APP}>")
endif()

if(NOT TARGET Visera::Core)
    message(FATAL_ERROR "Visera-Core is not installed!")
endif()
target_link_libraries(${VISERA_ENGINE} PRIVATE Visera::Core)

if(NOT TARGET Visera::Runtime)
    message(FATAL_ERROR "Visera-Runtime is not installed!")
endif()
target_link_libraries(${VISERA_ENGINE} PRIVATE Visera::Runtime)

#
# << Install External Packages >>
#

list(APPEND CMAKE_MODULE_PATH ${VISERA_ENGINE_SCRIPTS_DIR})

#
#
#
file(GLOB_RECURSE VISERA_ENGINE_MODULES "${VISERA_ENGINE_SOURCE_DIR}/*.ixx")

target_include_directories(${VISERA_ENGINE}
                           PUBLIC
                           ${VISERA_ENGINE_GLOBAL_DIR})

target_sources(${VISERA_ENGINE}
               PUBLIC
               FILE_SET "visera_engine_modules" TYPE CXX_MODULES
               FILES ${VISERA_ENGINE_MODULES})