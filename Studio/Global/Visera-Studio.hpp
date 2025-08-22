#pragma once
#include <Visera-Engine.hpp>
#if defined(VISERA_ENGINE_API)
#undef VISERA_ENGINE_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
    #if defined(VISERA_STUDIO_BUILD_SHARED)
        #define VISERA_STUDIO_API __declspec(dllexport)
    #else
        #define VISERA_STUDIO_API __declspec(dllimport)
    #endif
#else
    #define VISERA_STUDIO_API
#endif

namespace Visera
{

}