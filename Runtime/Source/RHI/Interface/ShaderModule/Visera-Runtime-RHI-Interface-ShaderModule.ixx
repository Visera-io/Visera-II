module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.ShaderModule;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.AssetHub.Shader;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IShaderModule
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;
        [[nodiscard]] const FByte*
        GetData() const { return Shader->GetData(); }
        [[nodiscard]] UInt64
        GetSize() const { return Shader->GetSize(); }
        [[nodiscard]] const char*
        GetEntryPoint() const { VISERA_ASSERT(EShaderLanguage::Slang == Shader->GetLanguage()); return "main"; }
        [[nodiscard]] const auto&
        GetName() const { return Shader->GetName(); }

        [[nodiscard]] Bool
        IsVertexShader() const { return Shader->GetStage() == EShaderStage::Vertex; }
        [[nodiscard]] Bool
        IsFragmentShader() const { return Shader->GetStage() == EShaderStage::Fragment; }
        [[nodiscard]] Bool
        IsValid() const { return Shader && !Shader->IsEmpty(); }

    protected:
        TSharedPtr<FShader> Shader;

    public:
        IShaderModule()                                = delete;
        IShaderModule(TSharedPtr<FShader> I_Shader) : Shader(std::move(I_Shader)) {};
        virtual ~IShaderModule()                       = default;
        IShaderModule(const IShaderModule&)            = delete;
        IShaderModule& operator=(const IShaderModule&) = delete;
    };
}
