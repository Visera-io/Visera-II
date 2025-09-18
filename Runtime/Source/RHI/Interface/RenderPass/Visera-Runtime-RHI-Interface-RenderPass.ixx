module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.RenderPass;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IRenderPass
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

    public:
        IRenderPass()                              = default;
        virtual ~IRenderPass()                     = default;
        IRenderPass(const IRenderPass&)            = delete;
        IRenderPass& operator=(const IRenderPass&) = delete;
    };
}
