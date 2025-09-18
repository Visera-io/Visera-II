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
        Import(const FPath& I_Path) -> TArray<FByte> = 0;

    public:
        virtual ~IShaderImporter() = default;
    };

}