#pragma once
#include <Visera-Runtime.hpp>
#if defined(VISERA_RUNTIME_API)
#undef VISERA_RUNTIME_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
  #if defined(VISERA_SHADER_BUILD_STATIC)
    #define VISERA_SHADER_API
  #elif defined(VISERA_SHADER_BUILD_SHARED) || defined(VISERA_MONOLITHIC_MODE)
    #define VISERA_SHADER_API __declspec(dllexport)
  #else
    #define VISERA_SHADER_API __declspec(dllimport)
  #endif
#else
  #define VISERA_SHADER_API __attribute__((visibility("default")))
#endif