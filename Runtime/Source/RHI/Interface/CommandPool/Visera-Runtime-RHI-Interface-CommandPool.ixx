module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.CommandPool;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API ICommandPool
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

    public:
        ICommandPool()                               = default;
        virtual ~ICommandPool()                      = default;
        ICommandPool(const ICommandPool&)            = delete;
        ICommandPool& operator=(const ICommandPool&) = delete;
    };
}
