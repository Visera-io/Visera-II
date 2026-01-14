#pragma once
#include <Visera-Global.hpp>
#if defined(VISERA_GLOBAL_API)
#undef VISERA_GLOBAL_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
  #if defined(VISERA_RHI_BUILD_STATIC)
    #define VISERA_RHI_API
  #elif defined(VISERA_RHI_BUILD_SHARED) || defined(VISERA_MONOLITHIC_MODE)
    #define VISERA_RHI_API __declspec(dllexport)
  #else
    #define VISERA_RHI_API __declspec(dllimport)
  #endif
#else
  #define VISERA_RHI_API __attribute__((visibility("default")))
#endif

#define VISERA_MAX_PUSH_CONSTANT_SIZE 32
// Use this Macro inside the renderpass class;
#define VISERA_PUSH_CONSTANT(...)                                           \
struct alignas(16) FPushConstantRange { __VA_ARGS__ };                      \
static_assert(sizeof(FPushConstantRange) <= VISERA_MAX_PUSH_CONSTANT_SIZE,  \
"Push constant exceeds VISERA_MAX_PUSH_CONSTANT_SIZE");                     \
static_assert((sizeof(FPushConstantRange) % 4) == 0,                        \
"Push constant size should be multiple of 4")