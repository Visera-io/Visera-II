#pragma once
#include <Visera-Runtime.hpp>
#if defined(VISERA_RUNTIME_API)
#undef VISERA_RUNTIME_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
    #define VISERA_ENGINE_API __declspec(dllexport)
#else
    #define VISERA_ENGINE_API __attribute__((visibility("default")))
#endif

namespace Visera
{

}