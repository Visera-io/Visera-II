module;
#include <Visera-Runtime.hpp>
#include "Visera-Runtime-RHI-Vulkan-Loader.inl"
export module Visera.Runtime.RHI.Vulkan.Loader;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera
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
        vk::detail::DynamicLoader DynamicLoader;

        mutable Bool bLoadedInstance = False;
        mutable Bool bLoadedDevice   = False;
    };

    FVulkanLoader::
    FVulkanLoader()
    {

    }

    FVulkanLoader::
    ~FVulkanLoader()
    {
        if (!bLoadedInstance)
        { LOG_ERROR("Forgot to load VkInstance!"); }
        if (!bLoadedDevice)
        { LOG_ERROR("Forgot to load VkDevice!"); }
    }

    void FVulkanLoader::
    Load(const vk::Instance& I_Instance) const
    {
        VISERA_ASSERT(I_Instance != nullptr);

        auto Func_vkGetInstanceProcAddr = DynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>
            ("vkGetInstanceProcAddr");
        if (!Func_vkGetInstanceProcAddr)
        { LOG_FATAL("Failed to get the 'vkGetInstanceProcAddr'!"); }

        VULKAN_HPP_DEFAULT_DISPATCHER.init(I_Instance, Func_vkGetInstanceProcAddr);
        bLoadedInstance = True;
    }

    void FVulkanLoader::
    Load(const vk::Device& I_Device) const
    {
        VISERA_ASSERT(I_Device != nullptr);
        VISERA_ASSERT(bLoadedInstance);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(I_Device);
        bLoadedDevice = True;
    }
}
