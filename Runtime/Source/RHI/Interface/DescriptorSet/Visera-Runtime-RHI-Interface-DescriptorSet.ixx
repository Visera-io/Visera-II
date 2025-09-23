module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.DescriptorSet;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IDescriptorSet
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

    public:
        IDescriptorSet()                                 = default;
        virtual ~IDescriptorSet()                        = default;
        IDescriptorSet(const IDescriptorSet&)            = delete;
        IDescriptorSet& operator=(const IDescriptorSet&) = delete;
    };
}
