module;
#include <Visera-Runtime.hpp>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI.Vulkan.Loader;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanLoader
    {
    public:
        void inline
        Load(const vk::Instance& I_Instance) const;
        void inline
        Load(const vk::Device& I_Device)     const;

        FVulkanLoader();
        ~FVulkanLoader();

    private:
        mutable UInt8 bLoadedInstance : 1 = False;
        mutable UInt8 bLoadedDevice   : 1 = False;
    };

    FVulkanLoader::
    FVulkanLoader()
    {
        LOG_TRACE("Creating the Vulkan Loader.");

        if (volkInitialize() != VK_SUCCESS)
        {
            throw SRuntimeError("Failed to initialize the Volk!");
        }
    }

    FVulkanLoader::
    ~FVulkanLoader()
    {
        LOG_TRACE("Destroying the Vulkan Loader.");

        if (!bLoadedInstance)
        { LOG_WARN("Forgot to load VkInstance?"); }
        if (!bLoadedDevice)
        { LOG_WARN("Forgot to load VkDevice?"); }

        volkFinalize();
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
}
