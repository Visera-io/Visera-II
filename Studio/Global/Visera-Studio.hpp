#pragma once
#include <Visera-Runtime.hpp>
#if defined(VISERA_RUNTIME_API)
#undef VISERA_RUNTIME_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
  #if defined(VISERA_STUDIO_BUILD_STATIC)
    #define VISERA_STUDIO_API
  #elif defined(VISERA_STUDIO_BUILD_SHARED)
    #define VISERA_STUDIO_API __declspec(dllexport)
  #else
    #define VISERA_STUDIO_API __declspec(dllimport)
  #endif
#else
  #define VISERA_STUDIO_API __attribute__((visibility("default")))
#endif