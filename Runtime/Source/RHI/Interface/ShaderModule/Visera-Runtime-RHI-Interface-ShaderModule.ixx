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

        [[nodiscard]] TWeakPtr<FShader>
        GetShader() const { return Shader; }

    private:
        TSharedPtr<FShader> Shader;

    public:
        IShaderModule()                                = default;
        IShaderModule(TSharedPtr<FShader> I_Shader) : Shader(std::move(I_Shader)) {};
        virtual ~IShaderModule()                       = default;
        IShaderModule(const IShaderModule&)            = delete;
        IShaderModule& operator=(const IShaderModule&) = delete;
    };
}
