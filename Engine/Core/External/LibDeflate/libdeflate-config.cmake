# Minimal libdeflate config for in-tree use when OpenEXR is built as a subproject
# and libdeflate is provided by Visera-Core via add_subdirectory.
# We do NOT include libdeflate-targets.cmake because the target already exists
# in the project (libdeflate::libdeflate_static from Core's LibDeflate).

set(libdeflate_FOUND TRUE)
set(libdeflate_VERSION "1.25")
