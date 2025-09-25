module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Drivers.Vulkan.CommandBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.CommandBuffer;
import Visera.Runtime.RHI.Interface.RenderPass;
import Visera.Runtime.RHI.Drivers.Vulkan.RenderPass;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanCommandBuffer : public ICommandBuffer
    {
    public:
        void
        Begin() override;
        void
        ReachRenderPass(const TUniquePtr<IRenderPass>& I_RenderPass) override;
        void
        Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
             UInt32 I_FirstVertex, UInt32 I_FirstInstance) const override;
        void
        LeaveRenderPass(const TUniquePtr<IRenderPass>& I_RenderPass) override;
        void
        End() override;
        void
        Submit(const void* I_Queue) override;

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
    Begin()
    {
        VISERA_ASSERT(IsIdle() || IsSubmitted());

        Handle.begin({});

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer::
    ReachRenderPass(const TUniquePtr<IRenderPass>& I_RenderPass)
    {
        VISERA_ASSERT(IsRecording());
        auto Pipeline = static_cast<const vk::raii::Pipeline*>(I_RenderPass->GetPipeline());
        Handle.bindPipeline(vk::PipelineBindPoint::eGraphics,
                         *Pipeline);
        Status = EStatus::InsideRenderPass;
    }

    void FVulkanCommandBuffer::
    Draw(UInt32 I_VertexCount, UInt32 I_InstanceCount,
         UInt32 I_FirstVertex, UInt32 I_FirstInstance) const
    {
        VISERA_ASSERT(IsInsideRenderPass());
        Handle.draw(I_VertexCount,
            I_InstanceCount,
            I_FirstVertex,
            I_FirstInstance);
    }

    void FVulkanCommandBuffer::
    LeaveRenderPass(const TUniquePtr<IRenderPass>& I_RenderPass)
    {
        VISERA_ASSERT(IsInsideRenderPass());

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer::
    End()
    {
        VISERA_ASSERT(IsRecording());
        Handle.end();

        Status = EStatus::Recording;
    }

    void FVulkanCommandBuffer::
    Submit(const void* I_Queue)
    {
        VISERA_ASSERT(!IsSubmitted());


        Status = EStatus::Submitted;
    }
}
