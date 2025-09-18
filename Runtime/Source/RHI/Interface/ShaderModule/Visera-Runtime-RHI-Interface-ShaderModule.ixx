module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.ShaderModule;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IShaderModule
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

    public:
        IShaderModule()                                = default;
        virtual ~IShaderModule()                       = default;
        IShaderModule(const IShaderModule&)            = delete;
        IShaderModule& operator=(const IShaderModule&) = delete;
    };
}
