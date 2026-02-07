#pragma once
#include <Visera-Global.hpp>
#if defined(VISERA_GLOBAL_API)
#undef VISERA_GLOBAL_API
#endif

#define VISERA_FORGE_API
//
// #if defined(VISERA_ON_WINDOWS_SYSTEM)
//   #if defined(VISERA_FORGE_BUILD_STATIC)
//     #define VISERA_FORGE_API
//   #elif defined(VISERA_FORGE_BUILD_SHARED) || defined(VISERA_MONOLITHIC_MODE)
//     #define VISERA_FORGE_API __declspec(dllexport)
//   #else
//     #define VISERA_FORGE_API __declspec(dllimport)
//   #endif
// #else
//   #define VISERA_FORGE_API __attribute__((visibility("default")))
// #endif