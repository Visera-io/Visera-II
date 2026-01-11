#pragma once
#include <Visera-Global.hpp>
#if defined(VISERA_GLOBAL_API)
#undef VISERA_GLOBAL_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
    #define VISERA_ENGINE_API __declspec(dllexport)
#else
    #define VISERA_ENGINE_API __attribute__((visibility("default")))
#endif

namespace Visera
{

}