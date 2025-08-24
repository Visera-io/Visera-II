module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Runtime.RHI.Volk;
import Visera.Core.Log;

namespace Visera
{
    class VISERA_RUNTIME_API FRHI
    {
        enum EStatues { Disabled, Bootstrapped, Terminated  };
    public:

    private:
        TUniquePtr<FVolk>   Volk;
        TUniquePtr<FVulkan> Vulkan;

    public:
        [[nodiscard]] inline Bool
        IsBootstrapped() const { return Statue == EStatues::Bootstrapped; }
        [[nodiscard]] inline Bool
        IsTerminated() const   { return Statue == EStatues::Terminated; }

        void inline
        Bootstrap();
        void inline
        Terminate();

    private:
        mutable EStatues Statue = EStatues::Disabled;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FRHI>
    GRHI = MakeUnique<FRHI>();

    void FRHI::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping RHI");
        try
        {
            Volk   = MakeUnique<FVolk>();
            Vulkan = MakeUnique<FVulkan>();
        }
        catch (const SRuntimeError& Error)
        {
            LOG_FATAL("{}", Error.what());
        }

        Statue = EStatues::Bootstrapped;
    }

    void FRHI::
    Terminate()
    {
        LOG_DEBUG("Terminating RHI");
        Vulkan.reset();
        Volk.reset();

        Statue = EStatues::Terminated;
    }

}
