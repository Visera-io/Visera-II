module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.RenderPipeline;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Vulkan.Pipeline.Render;

export namespace Visera
{
    using FRHIRenderPipeline = FVulkanRenderPipeline;
}