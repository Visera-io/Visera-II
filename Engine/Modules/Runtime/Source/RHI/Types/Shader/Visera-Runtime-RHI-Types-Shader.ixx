module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.Shader;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Vulkan.ShaderModule;
import Visera.Runtime.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FRHIShader
    {
    public:
        [[nodiscard]] inline ERHIShaderStages
        GetStage()      const { return Stage; }
        [[nodiscard]] inline FStringView
        GetEntryPoint() const { return "main"; }
        [[nodiscard]] inline TSharedRef<FVulkanShaderModule>
        GetShaderModule() const { return ShaderModule; }

        [[nodiscard]] inline Bool
        IsVertexShader()    const { return Stage == ERHIShaderStages::Vertex; }
        [[nodiscard]] inline Bool
        IsFragmentShader()  const { return Stage == ERHIShaderStages::Fragment; }

    private:
        ERHIShaderStages                Stage        {ERHIShaderStages::Undefined};
        TSharedPtr<FVulkanShaderModule> ShaderModule;

    public:
        FRHIShader() = delete;
        FRHIShader(ERHIShaderStages                I_Stage,
                   TSharedPtr<FVulkanShaderModule> I_ShaderModule);
        ~FRHIShader() = default;
    };

    FRHIShader::
    FRHIShader(ERHIShaderStages                I_Stage,
               TSharedPtr<FVulkanShaderModule> I_ShaderModule)
    : Stage         (I_Stage),
      ShaderModule  (I_ShaderModule)
    {
        VISERA_ASSERT(Stage != ERHIShaderStages::Undefined);
    }
}