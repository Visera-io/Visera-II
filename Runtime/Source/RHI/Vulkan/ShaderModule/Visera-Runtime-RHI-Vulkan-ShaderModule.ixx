module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.ShaderModule;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.ShaderModule;
import Visera.Runtime.AssetHub.Shader;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanShaderModule : public IShaderModule
    {
    public:
        [[nodiscard]] const void*
        GetHandle() const override { return &Handle; }

    private:
        vk::raii::ShaderModule Handle {nullptr};

    public:
        FVulkanShaderModule() = delete;
        FVulkanShaderModule(const vk::raii::Device& I_Device,
                            TSharedPtr<FShader>     I_Shader);
        ~FVulkanShaderModule() override;
    };

    FVulkanShaderModule::
    FVulkanShaderModule(const vk::raii::Device& I_Device,
                        TSharedPtr<FShader>     I_Shader)
    : IShaderModule{I_Shader}
    {
        auto CreateInfo = vk::ShaderModuleCreateInfo{}
            .setPCode    (reinterpret_cast<const uint32_t*>(Shader->GetData()))
            .setCodeSize (Shader->GetSize() * sizeof(FByte))
        ;
        auto Result = I_Device.createShaderModule(CreateInfo);
        if (!Result)
        {
            LOG_FATAL("Failed to create a shader module from {}!", I_Shader->GetName());
        }
        else
        { Handle = std::move(*Result); }
    }

    FVulkanShaderModule::
    ~FVulkanShaderModule()
    {

    }
}
