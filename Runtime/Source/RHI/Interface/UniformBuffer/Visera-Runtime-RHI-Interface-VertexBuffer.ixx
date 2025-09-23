module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.UniformBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IUniformBuffer
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

    public:
        IUniformBuffer()                                 = default;
        virtual ~IUniformBuffer()                        = default;
        IUniformBuffer(const IUniformBuffer&)            = delete;
        IUniformBuffer& operator=(const IUniformBuffer&) = delete;
    };
}
