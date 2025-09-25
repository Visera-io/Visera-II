module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Drivers.Vulkan.CommandBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.CommandBuffer;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanCommandBuffer : public ICommandBuffer
    {
    public:
        void
        Begin() const override;
        void
        End() const override;
        void
        Submit(const void* I_Queue) const override;

        const void*
        GetHandle() const override { return &Handle; }

    private:
        vk::raii::CommandBuffer Handle {nullptr};
        vk::Viewport Viewport {};
        vk::Rect2D   Scissor  {};

    public:
        FVulkanCommandBuffer(const vk::raii::Device&      I_Device,
                             const vk::raii::CommandPool& I_Pool,
                             EType I_Type);
        ~FVulkanCommandBuffer() override {}
    };

    FVulkanCommandBuffer::
    FVulkanCommandBuffer(const vk::raii::Device&      I_Device,
                         const vk::raii::CommandPool& I_Pool,
                         EType I_Type)
    : ICommandBuffer{I_Type}
    {
        auto CreateInfo = vk::CommandBufferAllocateInfo{}
            .setCommandPool         (I_Pool)
            .setLevel               (vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount  (1)
        ;
        auto Results = I_Device.allocateCommandBuffers(CreateInfo);
        if (!Results)
        {  LOG_FATAL("Failed to create a new Vulkan Command Buffer (type:{})!", I_Type); }
        else
        { Handle = std::move(Results->front()); }
    }

    void FVulkanCommandBuffer::
    Begin() const
    {
        Handle.begin({});
    }

    void FVulkanCommandBuffer::
    End() const
    {
        Handle.end();
    }

    void FVulkanCommandBuffer::
    Submit(const void* I_Queue) const
    {

    }
}
