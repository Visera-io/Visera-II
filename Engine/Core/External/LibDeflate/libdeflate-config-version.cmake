# Version file for find_package(libdeflate CONFIG) compatibility.
# Used when OpenEXR finds libdeflate via this in-tree config.
set(PACKAGE_VERSION "1.25")
if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
  set(PACKAGE_VERSION_EXACT TRUE)
endif()
if(NOT PACKAGE_FIND_VERSION VERSION_GREATER PACKAGE_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
