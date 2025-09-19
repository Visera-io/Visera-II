module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Shader.Importer;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Runtime.AssetHub.Shader.Common;
export import Visera.Core.Types.Path;

namespace Visera
{
    export class VISERA_RUNTIME_API IShaderImporter
    {
    public:
        [[nodiscard]] virtual auto
        Import(const FPath& I_Path, FStringView I_EntryPoint) -> TArray<FByte> = 0;
        [[nodiscard]] inline EShaderLanguage
        GetLanguage() const { return Language; };
        [[nodiscard]] inline EShaderStage
        GetShaderStage() const { return ShaderStage; };

    protected:
        EShaderLanguage Language   { EShaderLanguage::Slang };
        EShaderStage    ShaderStage { EShaderStage::Unknown   };

    public:
        virtual ~IShaderImporter() = default;
    };
}