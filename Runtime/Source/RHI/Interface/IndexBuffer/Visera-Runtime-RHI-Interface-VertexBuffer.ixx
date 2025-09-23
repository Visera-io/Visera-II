module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.IndexBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IIndexBuffer
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

    public:
        IIndexBuffer()                               = default;
        virtual ~IIndexBuffer()                      = default;
        IIndexBuffer(const IIndexBuffer&)            = delete;
        IIndexBuffer& operator=(const IIndexBuffer&) = delete;
    };
}
