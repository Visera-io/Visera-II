module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Driver.Interface.Fence;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IFence
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

    public:
        IFence() = default;
        virtual ~IFence() = default;
        IFence(const IFence&) = delete;
        IFence& operator=(const IFence&) = delete;
    };
}
