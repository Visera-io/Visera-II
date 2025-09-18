module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Drivers.Vulkan.ShaderModule;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.ShaderModule;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanShaderModule : public IShaderModule
    {
    public:
        [[nodiscard]] const void*
        GetHandle() const override { return *Handle; }

    private:
        vk::raii::ShaderModule Handle {nullptr};

    public:
        FVulkanShaderModule();
        ~FVulkanShaderModule() override;
    };

    FVulkanShaderModule::
    FVulkanShaderModule()
    {

    }

    FVulkanShaderModule::
    ~FVulkanShaderModule()
    {

    }
}
