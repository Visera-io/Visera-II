module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.VertexBuffer;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Vulkan.Buffer;

export namespace Visera
{
    using FRHIVertexBuffer = FVulkanBuffer;
}