module;
#include <Visera-RHI.hpp>

#include "vulkan/vulkan_core.h"
export module Visera.RHI.Vulkan.Loader;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.Global.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanLoader
    {
    public:
        void inline
        Load(const vk::Instance& I_Instance) const;
        void inline
        Load(const vk::Device& I_Device)     const;

        FVulkanLoader();
        ~FVulkanLoader();

    private:
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
        vk::detail::defaultDispatchLoaderDynamic.init(vkGetInstanceProcAddr);
        vk::detail::defaultDispatchLoaderDynamic.init(I_Instance);
        bLoadedInstance = True;
    }

    void FVulkanLoader::
    Load(const vk::Device& I_Device) const
    {
        VISERA_ASSERT(I_Device != nullptr);
        VISERA_ASSERT(bLoadedInstance);
        vk::detail::defaultDispatchLoaderDynamic.init(I_Device);
        bLoadedDevice = True;
    }
}
