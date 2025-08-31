module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Runtime.RHI.VMA;
import Visera.Core.Log;

namespace Visera
{
    class VISERA_RUNTIME_API FRHI : public IGlobalSingleton
    {
    public:
        using EFormat        = vk::Format;

        [[nodiscard]] inline auto
        GetSurfaceCapabilities() const { return Vulkan->GPU.Handle.getSurfaceCapabilitiesKHR(Vulkan->Surface); };

        [[nodiscard]] inline auto
        GetSurfaceFormats() const { return Vulkan->GPU.Handle.getSurfaceFormatsKHR(Vulkan->Surface); };

        [[nodiscard]] inline auto
        GetPresentModes() const { return Vulkan->GPU.Handle.getSurfacePresentModesKHR(Vulkan->Surface); };

        [[nodiscard]] inline const TUniquePtr<FVulkan>&
        GetAPI() const { return Vulkan; };

    private:

        TUniquePtr<FVulkan> Vulkan;

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
        LOG_DEBUG("Terminating RHI.");
        Vulkan.reset();

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
