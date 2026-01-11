#pragma once
#include <Visera-Global.hpp>
#if defined(VISERA_GLOBAL_API)
#undef VISERA_GLOBAL_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
  #if defined(VISERA_AUDIO_BUILD_STATIC)
    #define VISERA_AUDIO_API
  #elif defined(VISERA_AUDIO_BUILD_SHARED) || defined(VISERA_MONOLITHIC_MODE)
    #define VISERA_AUDIO_API __declspec(dllexport)
  #else
    #define VISERA_AUDIO_API __declspec(dllimport)
  #endif
#else
  #define VISERA_AUDIO_API __attribute__((visibility("default")))
#endif