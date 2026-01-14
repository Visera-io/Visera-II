#pragma once
#include <Visera-RHI.hpp>
#if defined(VISERA_RHI_API)
#undef VISERA_RHI_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
  #if defined(VISERA_GRAPHICS_BUILD_STATIC)
    #define VISERA_GRAPHICS_API
  #elif defined(VISERA_GRAPHICS_BUILD_SHARED) || defined(VISERA_MONOLITHIC_MODE)
    #define VISERA_GRAPHICS_API __declspec(dllexport)
  #else
    #define VISERA_GRAPHICS_API __declspec(dllimport)
  #endif
#else
  #define VISERA_GRAPHICS_API __attribute__((visibility("default")))
#endif