module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.RenderPipeline;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Pipeline.Render;

export namespace Visera
{
    using FRHIRenderPipeline = FVulkanRenderPipeline;
}