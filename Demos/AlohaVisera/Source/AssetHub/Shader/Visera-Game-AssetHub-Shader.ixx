module;
#include <Visera-Game.hpp>
export module Visera.Game.AssetHub.Shader;
#define VISERA_MODULE_NAME "Game.AssetHub"
export import Visera.Game.AssetHub.Shader.Common;
       import Visera.Game.AssetHub.Shader.Slang;
       import Visera.Game.AssetHub.Asset;
       import Visera.RHI;
       import Visera.Runtime.Log;
       import Visera.Core.Types.Array;

namespace Visera
{
    export class VISERA_ENGINE_API FShader : public IAsset
    {
    public:
        [[nodiscard]] const FByte*
        GetData() const override { return SPIRVCode.data(); }
        [[nodiscard]] UInt64
        GetSize() const override { return SPIRVCode.size(); }
        [[nodiscard]] inline EShaderLanguage
        GetLanguage() const { return Language; }
        [[nodiscard]] inline FStringView
        GetEntryPoint() const { return EntryPoint; }
        [[nodiscard]] inline const TArray<FByte>&
        GetSPIRVCode() const { VISERA_ASSERT(GetSize() != 0); return SPIRVCode; }

    private:
        static inline TUniquePtr<FSlangCompiler> SlangCompiler;
        const FString          EntryPoint;
        EShaderLanguage        Language;
        TArray<FByte>          SPIRVCode;

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
        
        if (Extension == FPath(".slang"))
        {
            if (!SlangCompiler) { SlangCompiler = MakeUnique<FSlangCompiler>(); }

            Language  = EShaderLanguage::Slang;
            SPIRVCode = SlangCompiler->Compile(GetPath(), EntryPoint);
        }
        else
        {
            LOG_ERROR("Failed to load the shader \"{}\""
                      "-- unsupported extension {}!", GetPath(), Extension);
            return;
        }
    }
}
