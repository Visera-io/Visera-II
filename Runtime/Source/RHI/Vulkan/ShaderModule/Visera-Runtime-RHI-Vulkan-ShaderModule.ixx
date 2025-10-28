module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.ShaderModule;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.SPIRV;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanShaderModule
    {
    public:
        [[nodiscard]] inline const char*
        GetEntryPoint() const { return "main"/*Shader->GetEntryPoint().data()*/; }
        [[nodiscard]] inline const vk::raii::ShaderModule&
        GetHandle() const { return Handle; }

    private:
        vk::raii::ShaderModule Handle {nullptr};

    public:
        FVulkanShaderModule() = delete;
        FVulkanShaderModule(const vk::raii::Device&  I_Device,
                            TSharedRef<FSPIRVShader> I_SPIRVShader);
        ~FVulkanShaderModule();
    };

    FVulkanShaderModule::
    FVulkanShaderModule(const vk::raii::Device&  I_Device,
                        TSharedRef<FSPIRVShader> I_SPIRVShader)
    {
        auto CreateInfo = vk::ShaderModuleCreateInfo{}
            .setPCode    (reinterpret_cast<const UInt32*>(I_SPIRVShader->GetShaderCode().data()))
            .setCodeSize (I_SPIRVShader->GetSize() * sizeof(FByte))
        ;
        auto Result = I_Device.createShaderModule(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create a Vulkan Shader Module!"); }
        else
        { Handle = std::move(*Result); }
    }

    FVulkanShaderModule::
    ~FVulkanShaderModule()
    {
        Handle.clear();
    }
}
