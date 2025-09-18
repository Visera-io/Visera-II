module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Shader;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Interface;
import Visera.Runtime.AssetHub.Shader.Importer;
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

    private:
        TArray<FByte> Data;
        TUniquePtr<IShaderImporter> Importer;

    public:
        FShader() = delete;
        FShader(const FName& I_Name, const FPath& I_Path);
    };

    FShader::
    FShader(const FName& I_Name, const FPath& I_Path)
    : IAsset(EType::Shader, I_Name, I_Path)
    {
        const FPath Extension = GetPath().GetExtension();

        if (Extension == PATH(".slang"))
        {
            Importer = MakeUnique<FSlangShaderImporter>();
            Data = Importer->Import(GetPath());
        }
        else
        {
            LOG_ERROR("Failed to load the shader \"{}\""
                      "-- unsupported extension {}!", GetPath(), Extension);
            return;
        }

        LOG_DEBUG("Loaded the shader \"{}\".",
                  GetPath());
                  //, Importer->GetWidth(), Importer->GetHeight(), Importer->IsSRGB());
    }
}
