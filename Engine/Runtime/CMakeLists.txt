# Runtime: single CMake/install (aligned with Core). VISERA_RUNTIME_DIR set by Engine.
# install_visera_runtime(${VISERA}) is called by Engine/CMakeLists.txt.

set(VISERA_RUNTIME_EXTERNAL_DIR "${VISERA_RUNTIME_DIR}/External" CACHE STRING "")
include("CMake/install.cmake")
