module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.Sampler;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API ISampler
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

    public:
        ISampler()                           = default;
        virtual ~ISampler()                  = default;
        ISampler(const ISampler&)            = delete;
        ISampler& operator=(const ISampler&) = delete;
    };
}
