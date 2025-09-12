module;
#include <Visera-Runtime.hpp>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI.Driver.Vulkan.Loader;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera
{
    class VISERA_RUNTIME_API FVulkanLoader : public IGlobalSingleton
    {
    public:
        void inline
        Load(const vk::Instance& I_Instance) const;
        void inline
        Load(const vk::Device& I_Device)     const;

        void inline
        Bootstrap() override;
        void inline
        Terminate() override;

        FVulkanLoader() : IGlobalSingleton("VMA") {}
        ~FVulkanLoader() override;

    private:
        mutable UInt8 bLoadedInstance : 1 = False;
        mutable UInt8 bLoadedDevice   : 1 = False;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FVulkanLoader>
    GVulkanLoader = MakeUnique<FVulkanLoader>();

    void FVulkanLoader::
    Bootstrap()
    {
        if (IsBootstrapped()) { return; }

        LOG_DEBUG("Bootstrapping Vulkan Loader.");

        if (volkInitialize() != VK_SUCCESS)
        {
            throw SRuntimeError("Failed to initialize the Volk!");
        }

        Statue = EStatues::Bootstrapped;
    }

    void FVulkanLoader::
    Terminate()
    {
        if (!IsBootstrapped()) { return; }

        LOG_DEBUG("Terminating Vulkan Loader.");

        if (!bLoadedInstance)
        { LOG_WARN("Forgot to load VkInstance?"); }
        if (!bLoadedDevice)
        { LOG_WARN("Forgot to load VkDevice?"); }

        volkFinalize();

        Statue = EStatues::Terminated;
    }

    void FVulkanLoader::
    Load(const vk::Instance& I_Instance) const
    {
        VISERA_ASSERT(I_Instance != nullptr);
        volkLoadInstance(I_Instance);
        bLoadedInstance = True;
    }

    void FVulkanLoader::
    Load(const vk::Device& I_Device) const
    {
        VISERA_ASSERT(I_Device != nullptr);
        volkLoadDevice(I_Device);
        bLoadedDevice = True;
    }

    FVulkanLoader::
    ~FVulkanLoader()
    {

    }
}
