module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.GFX.SwapChain;
#define VISERA_MODULE_NAME "Runtime.GFX"
import Visera.Runtime.RHI;

export namespace Visera
{
    class VISERA_RUNTIME_API FSwapChain
    {
    public:
        FSwapChain();
        ~FSwapChain();

    private:
        vk::raii::SwapchainKHR Handle {nullptr};
    };

    FSwapChain::
    FSwapChain()
    {
        const auto SurfaceCapabilities = GRHI->GetSurfaceCapabilities();
        const auto SurfaceFormats = GRHI->GetSurfaceFormats();
        const auto PresentModes = GRHI->GetPresentModes();
    }

    FSwapChain::
    ~FSwapChain()
    {

    }


}
