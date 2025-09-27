module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Shader;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Runtime.AssetHub.Shader.Common;
       import Visera.Runtime.AssetHub.Asset;
       import Visera.Runtime.AssetHub.Shader.Compiler;
       import Visera.Runtime.AssetHub.Shader.Slang;
       import Visera.Core.Log;

export namespace Visera
{
    class VISERA_RUNTIME_API FShader : public IAsset
    {
    public:
        [[nodiscard]] const FByte*
        GetData() const override { return Data.data(); }
        [[nodiscard]] UInt64
        GetSize() const override { return Data.size(); }
        [[nodiscard]] EShaderLanguage
        GetLanguage() const { return Importer->GetLanguage(); }
        [[nodiscard]] EShaderStage
        GetStage() const { return Importer->GetShaderStage(); }
        [[nodiscard]] FStringView
        GetEntryPoint() const { return EntryPoint; }

    private:
        const FString   EntryPoint;
        TArray<FByte>   Data;
        TUniquePtr<IShaderCompiler> Importer;

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
            Importer = MakeUnique<FSlangShaderCompiler>(); //[TODO]: Shared Session
            Data = Importer->Compile(GetPath(), EntryPoint);
        }
        else
        {
            LOG_ERROR("Failed to load the shader \"{}\""
                      "-- unsupported extension {}!", GetPath(), Extension);
            return;
        }

        LOG_DEBUG("Loaded the shader \"{}\" (stage:{}, entry_point:{}, size:{}).",
                  GetPath(), Importer->GetShaderStage(), EntryPoint, Data.size());
    }
}
