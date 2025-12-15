module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Vulkan.ShaderModule;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;
import Visera.Core.Types.Array;
import vulkan_hpp;

namespace Visera
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
        FVulkanShaderModule(const vk::raii::Device& I_Device,
                            const TArray<FByte>&    I_SPIRVShader);
        ~FVulkanShaderModule() {};
    };

    FVulkanShaderModule::
    FVulkanShaderModule(const vk::raii::Device& I_Device,
                        const TArray<FByte>&    I_SPIRVShader)
    {
        VISERA_ASSERT((I_SPIRVShader.size() % 4) == 0);

        const auto CreateInfo = vk::ShaderModuleCreateInfo{}
            .setPCode    (reinterpret_cast<const UInt32*>(I_SPIRVShader.data()))
            .setCodeSize (I_SPIRVShader.size() * sizeof(FByte))
        ;
        auto Result = I_Device.createShaderModule(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create a Vulkan Shader Module!"); }
        else
        { Handle = std::move(*Result); }
    }
}
