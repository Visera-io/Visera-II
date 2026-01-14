#pragma once
#include <Visera-Global.hpp>
#if defined(VISERA_PHYSICS2D_API)
#undef VISERA_PHYSICS2D_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
  #if defined(VISERA_PHYSICS2D_BUILD_STATIC)
    #define VISERA_PHYSICS2D_API
  #elif defined(VISERA_PHYSICS2D_BUILD_SHARED) || defined(VISERA_MONOLITHIC_MODE)
    #define VISERA_PHYSICS2D_API __declspec(dllexport)
  #else
    #define VISERA_PHYSICS2D_API __declspec(dllimport)
  #endif
#else
  #define VISERA_PHYSICS2D_API __attribute__((visibility("default")))
#endif