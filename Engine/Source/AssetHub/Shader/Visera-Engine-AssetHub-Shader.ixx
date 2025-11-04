module;
#include <Visera-Engine.hpp>
export module Visera.Engine.AssetHub.Shader;
#define VISERA_MODULE_NAME "Engine.AssetHub"
export import Visera.Engine.AssetHub.Shader.Common;
       import Visera.Engine.AssetHub.Shader.Slang;
       import Visera.Engine.AssetHub.Asset;
       import Visera.Runtime.RHI;
       import Visera.Core.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FShader : public IAsset
    {
    public:
        [[nodiscard]] const FByte*
        GetData() const override { return SPIRVCode->GetShaderCode().data(); }
        [[nodiscard]] UInt64
        GetSize() const override { return SPIRVCode->GetSize(); }
        [[nodiscard]] inline EShaderLanguage
        GetLanguage() const { return Language; }
        [[nodiscard]] inline EShaderStage
        GetStage() const { return ShaderStage; }
        [[nodiscard]] inline FStringView
        GetEntryPoint() const { return EntryPoint; }
        [[nodiscard]] inline TSharedRef<RHI::FSPIRVShader>
        GetSPIRVCode() const { VISERA_ASSERT(SPIRVCode && "Compiled"); return SPIRVCode; }

    private:
        static inline TUniquePtr<FSlangCompiler> SlangCompiler;
        const FString                   EntryPoint;
        EShaderLanguage                 Language;
        EShaderStage                    ShaderStage;
        TSharedPtr<RHI::FSPIRVShader>   SPIRVCode;

    public:
        FShader() = delete;
        FShader(const FName& I_Name, const FPath& I_Path, const FString& I_EntryPoint);
    };

    FShader::
    FShader(const FName& I_Name, const FPath& I_Path, const FString& I_EntryPoint)
    : IAsset(EType::Shader, I_Name, I_Path),
      EntryPoint{I_EntryPoint}
    {
        const FPath Extension = GetPath().GetExtension();
        
        if (Extension == PATH(".slang"))
        {
            if (!SlangCompiler) { SlangCompiler = MakeUnique<FSlangCompiler>(); }
            SPIRVCode = SlangCompiler->Compile(GetPath(), EntryPoint);
            VISERA_ASSERT(SPIRVCode);
        }
        else
        {
            LOG_ERROR("Failed to load the shader \"{}\""
                      "-- unsupported extension {}!", GetPath(), Extension);
            return;
        }

        switch (SPIRVCode->GetStage())
        {
        case RHI::FSPIRVShader::EStage::Vertex:   ShaderStage = EShaderStage::Vertex;   break;
        case RHI::FSPIRVShader::EStage::Fragment: ShaderStage = EShaderStage::Fragment; break;
        default: LOG_FATAL("Unknown shader stage!");
        }
    }
}
