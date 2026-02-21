#pragma once

#include <Visera-Core.hpp>
#if defined(VISERA_CORE_API)
#undef VISERA_CORE_API
#endif

// VISERA_RUNTIME_API: from CMake VISERA_RUNTIME_BUILD_STATIC/SHARED (PRIVATE for SHARED, PUBLIC for STATIC).
#if defined(VISERA_RUNTIME_BUILD_STATIC)
#  define VISERA_RUNTIME_API
#elif defined(VISERA_ON_WINDOWS_SYSTEM)
#  if defined(VISERA_RUNTIME_BUILD_SHARED)
#    define VISERA_RUNTIME_API __declspec(dllexport)
#  else
#    define VISERA_RUNTIME_API __declspec(dllimport)
#  endif
#else
#  define VISERA_RUNTIME_API __attribute__((visibility("default")))
#endif