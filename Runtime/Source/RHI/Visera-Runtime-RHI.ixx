module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Core.Log;

namespace Visera
{
    export namespace RHI
    {

    }

    class VISERA_RUNTIME_API FRHI : public IGlobalSingleton
    {
    public:
        [[nodiscard]] inline const auto&
        GetDriver()   const { return GVulkan; };

    public:
        FRHI() : IGlobalSingleton("RHI") {}
        ~FRHI() override;
        void inline
        Bootstrap() override;
        void inline
        Terminate() override;

    };

    export inline VISERA_RUNTIME_API TUniquePtr<FRHI>
    GRHI = MakeUnique<FRHI>();

    void FRHI::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping RHI.");

        try
        {
            GVulkan->Bootstrap();
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
        LOG_DEBUG("Terminating RHI.");
        GVulkan->Terminate();
        Statue = EStatues::Terminated;
    }

    FRHI::
    ~FRHI()
    {
        if (IsBootstrapped())
        {
            std::cerr << "[FATAL] RHI must be terminated properly!\n";
            std::exit(EXIT_FAILURE);
        }
    }

}
