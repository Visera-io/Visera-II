module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.VertexBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Buffer;

export namespace Visera
{
    using FRHIVertexBuffer = FVulkanBuffer;
}