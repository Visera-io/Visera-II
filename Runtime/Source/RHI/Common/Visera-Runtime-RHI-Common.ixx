module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Common;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Core.Log;

export namespace Visera
{
    namespace RHI
    {
        using EQueue            = EVulkanQueue;
        using EImageType        = EVulkanImageType;
        using EFormat           = EVulkanFormat;
        using EImageUsage       = EVulkanImageUsage;
        using EImageLayout      = EVulkanImageLayout;
        using EPipelineStage    = EVulkanPipelineStage;
        using EAccess           = EVulkanAccess;
        using EShaderStage      = EVulkanShaderStage;
        using EMemoryPoolFlag   = EVulkanMemoryPoolFlag;
        using EBufferUsage      = EVulkanBufferUsage;

        using FBuffer           = FVulkanBuffer;
    }
}