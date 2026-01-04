module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.CommandBuffer;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Vulkan.CommandBuffer;

export namespace Visera
{
    class VISERA_RHI_API FRHICommandBuffer final
    {
    public:
        using EStatus = FVulkanCommandBuffer::EStatus;
        [[nodiscard]] inline EStatus
        GetStatus() const { return Self.GetStatus(); }

    private:
        FVulkanCommandBuffer Self;

    public:
    };
}