module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.ShaderModule;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.AssetHub.Shader;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanShaderModule
    {
    public:
        [[nodiscard]] inline const char*
        GetEntryPoint() const { return "main"/*Shader->GetEntryPoint().data()*/; }
        [[nodiscard]] inline const auto&
        GetName() const { return Shader->GetName(); }
        [[nodiscard]] inline const vk::raii::ShaderModule&
        GetHandle() const { return Handle; }

        [[nodiscard]] inline Bool
        IsVertexShader() const { return Shader->GetStage() == EShaderStage::Vertex; }
        [[nodiscard]] inline Bool
        IsFragmentShader() const { return Shader->GetStage() == EShaderStage::Fragment; }

    private:
        vk::raii::ShaderModule Handle {nullptr};
        TSharedPtr<FShader>    Shader;

    public:
        FVulkanShaderModule() = delete;
        FVulkanShaderModule(const vk::raii::Device& I_Device,
                            TSharedPtr<FShader>     I_Shader);
        ~FVulkanShaderModule();
    };

    FVulkanShaderModule::
    FVulkanShaderModule(const vk::raii::Device& I_Device,
                        TSharedPtr<FShader>     I_Shader)
    : Shader{ std::move(I_Shader) }
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
