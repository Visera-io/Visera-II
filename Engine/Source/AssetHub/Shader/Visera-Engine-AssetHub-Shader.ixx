module;
#include <Visera-Engine.hpp>
export module Visera.Engine.AssetHub.Shader;
#define VISERA_MODULE_NAME "Engine.AssetHub"
export import Visera.Engine.AssetHub.Shader.Common;
       import Visera.Engine.AssetHub.Asset;
       import Visera.Runtime.RHI;
       import Visera.Core.Log;

export namespace Visera
{
    class VISERA_ENGINE_API FShader : public IAsset
    {
    public:
        [[nodiscard]] const FByte*
        GetData() const override { return Data.data(); }
        [[nodiscard]] UInt64
        GetSize() const override { return Data.size(); }
        [[nodiscard]] EShaderLanguage
        GetLanguage() const { return Language; }
        [[nodiscard]] EShaderStage
        GetStage() const { return ShaderStage; }
        [[nodiscard]] FStringView
        GetEntryPoint() const { return EntryPoint; }

    private:
        const FString   EntryPoint;
        TArray<FByte>   Data;
        EShaderLanguage Language;
        EShaderStage    ShaderStage;

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
            VISERA_UNIMPLEMENTED_API;
            //Importer = MakeUnique<FSlangShaderCompiler>(); //[TODO]: Shared Session
            //Data = Importer->Compile(GetPath(), EntryPoint);
        }
        else
        {
            LOG_ERROR("Failed to load the shader \"{}\""
                      "-- unsupported extension {}!", GetPath(), Extension);
            return;
        }
    }
}
