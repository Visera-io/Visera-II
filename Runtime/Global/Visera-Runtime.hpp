#pragma once
#include <Visera-Core.hpp>
#if defined(VISERA_CORE_API)
#undef VISERA_CORE_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
    #if defined(VISERA_RUNTIME_BUILD_SHARED)
        #define VISERA_RUNTIME_API __declspec(dllexport)
    #else
        #define VISERA_RUNTIME_API __declspec(dllimport)
    #endif
#else
    #define VISERA_RUNTIME_API
#endif

namespace Visera
{
    using SRuntimeError = std::runtime_error;

    class VISERA_RUNTIME_API IGlobalSingleton
    {
    public:
        enum EStatues { Disabled, Bootstrapped, Terminated  };

        [[nodiscard]] FStringView
        GetDebugName() const { return Name; }

        virtual void inline
        Bootstrap() = 0;
        virtual void inline
        Terminate() = 0;

        [[nodiscard]] inline Bool
        IsBootstrapped() const { return Statue == EStatues::Bootstrapped; }
        [[nodiscard]] inline Bool
        IsTerminated() const   { return Statue == EStatues::Terminated; }

        IGlobalSingleton() = delete;
        explicit IGlobalSingleton(const char* I_Name) : Name(I_Name) {}
        virtual ~IGlobalSingleton()
        {
            if (IsBootstrapped())
            {
                std::cerr << "[WARNING] " << Name << " was NOT terminated properly!\n";
            }
        }

    protected:
        const char* Name;
        mutable EStatues Statue = EStatues::Disabled;
    };

}