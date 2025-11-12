#pragma once
#include <Visera-Core.hpp>
#if defined(VISERA_CORE_API)
#undef VISERA_CORE_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
    #define VISERA_RUNTIME_API __declspec(dllexport)
#else
    #define VISERA_RUNTIME_API __attribute__((visibility("default")))
#endif

namespace Visera
{
    using SRuntimeError = std::runtime_error;
}