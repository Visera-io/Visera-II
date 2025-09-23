module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.VertexBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IVertexBuffer
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

    public:
        IVertexBuffer()                                = default;
        virtual ~IVertexBuffer()                       = default;
        IVertexBuffer(const IVertexBuffer&)            = delete;
        IVertexBuffer& operator=(const IVertexBuffer&) = delete;
    };
}
