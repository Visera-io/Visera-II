module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Vulkan.Pipeline.Render;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Attachment;
import Visera.Runtime.RHI.Vulkan.Pipeline.Cache;
import Visera.Runtime.RHI.Vulkan.Pipeline.Layout;
import Visera.Runtime.RHI.Vulkan.ShaderModule;
import Visera.Core.Log;
import Visera.Core.Containers.Array;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkanRenderPipeline
    {
    public:
        [[nodiscard]] inline const vk::raii::Pipeline&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline FVulkanPipelineLayout*
        GetLayout() { return &Layout; }
        [[nodiscard]] inline const FVulkanPipelineLayout*
        GetLayout() const { return &Layout; }
        [[nodiscard]] inline const vk::Rect2D&
        GetRenderArea() const { return CurrentRenderingInfo.renderArea; }
        /** Number of color attachments currently set (0 until SetColorAttachments is called). */
        [[nodiscard]] inline UInt32
        GetColorAttachmentCount() const { return ColorAttachmentCount; }
        inline FVulkanRenderPipeline*
        SetColorAttachments(FVulkanColorAttachment* const* I_ColorAttachments, UInt32 I_Count);
        [[nodiscard]] inline FVulkanDepthStencilAttachment*
        GetDepthStencilAttachment() const { return CurrentDepthStencilAttachment; }
        inline FVulkanRenderPipeline*
        SetDepthStencilAttachment(FVulkanDepthStencilAttachment* I_DepthStencilAttachment);
        /** Clears depth/stencil attachment so VkRenderingInfo no longer has pDepthAttachment/pStencilAttachment. */
        inline FVulkanRenderPipeline*
        ClearDepthStencilAttachment();
        inline FVulkanRenderPipeline*
        SetRenderArea(const vk::Rect2D& I_RenderArea) { CurrentRenderingInfo.setRenderArea(I_RenderArea); return this; }
        [[nodiscard]] inline const vk::RenderingInfo&
        GetRenderingInfo() const { return CurrentRenderingInfo; }

        struct FSettings
        {
            TInlineArray<vk::VertexInputAttributeDescription, 16> VertexAttributes;
            TInlineArray<vk::VertexInputBindingDescription,    8> VertexBindings;
            vk::PipelineInputAssemblyStateCreateInfo
            InputAssembly;
            vk::PipelineViewportStateCreateInfo
            ViewportState;
            vk::PipelineRasterizationStateCreateInfo
            Rasterizer;
            vk::PipelineMultisampleStateCreateInfo
            Multisampling;
            vk::PipelineColorBlendAttachmentState
            ColorBlendAttachment;
            /** One per color attachment; length must match PipelineRenderingCreateInfo. */
            TInlineArray<vk::Format, kMaxColorAttachments> ColorRTFormats;
            vk::Format
            DepthRTFormat    {vk::Format::eUndefined};
            vk::Format
            StencilRTFormat  {vk::Format::eUndefined};
        }Settings;

    private:
        vk::raii::Pipeline                Handle {nullptr};
        FVulkanPipelineLayout             Layout;

        FVulkanShaderModule               VertexShader;
        FVulkanShaderModule               FragmentShader;

        vk::RenderingInfo    CurrentRenderingInfo;
        /** Per-instance attachment infos so VkRenderingInfo stays valid until submit (no static/data-race). */
        vk::RenderingAttachmentInfo ColorAttachmentInfos[kMaxColorAttachments];
        vk::RenderingAttachmentInfo DepthAttachmentInfo{};
        vk::RenderingAttachmentInfo StencilAttachmentInfo{};

        UInt32                          ColorAttachmentCount          {0};
        FVulkanDepthStencilAttachment*  CurrentDepthStencilAttachment {nullptr};

        enum : UInt8 { MAX_DYNAMIC_STATE = 2 };
        static inline vk::DynamicState  DynamicStates[MAX_DYNAMIC_STATE]
        {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
        };
        vk::PipelineDynamicStateCreateInfo
        DynamicStateCreateInfo{};

    public:
        FVulkanRenderPipeline() = default;
        FVulkanRenderPipeline(FVulkanPipelineLayout&& I_PipelineLayout,
                              FVulkanShaderModule&&   I_VertexShader,
                              FVulkanShaderModule&&   I_FragmentShader)
        : Layout         (std::move(I_PipelineLayout)),
          VertexShader   (std::move(I_VertexShader)),
          FragmentShader (std::move(I_FragmentShader))
        {
            /* Use Create(...) */
            Settings.InputAssembly
                .setTopology (vk::PrimitiveTopology::eTriangleList)
            ;
            Settings.ViewportState
                .setViewportCount   (1)
                .setPViewports      (nullptr) // Dynamic Viewport
                .setScissorCount    (1)
                .setPScissors       (nullptr) // Dynamic Scissor
            ;
            Settings.Rasterizer
                .setDepthClampEnable        (vk::False)
                .setRasterizerDiscardEnable (vk::False)
                .setPolygonMode             (vk::PolygonMode::eFill)
                .setCullMode                (vk::CullModeFlagBits::eBack)
                .setFrontFace               (vk::FrontFace::eClockwise)
                .setDepthBiasEnable         (vk::False)
                .setDepthBiasSlopeFactor    (1.0)
                .setLineWidth               (1.0)
            ;
            Settings.Multisampling
                .setRasterizationSamples    (vk::SampleCountFlagBits::e1)
                .setSampleShadingEnable     (vk::False)
            ;
            Settings.ColorBlendAttachment
                .setColorWriteMask      (vk::ColorComponentFlagBits::eR |
                                         vk::ColorComponentFlagBits::eG |
                                         vk::ColorComponentFlagBits::eB |
                                         vk::ColorComponentFlagBits::eA)
                .setBlendEnable         (vk::True)
                .setSrcColorBlendFactor (vk::BlendFactor::eSrcAlpha)
                .setDstColorBlendFactor (vk::BlendFactor::eOneMinusSrcAlpha)
                .setColorBlendOp        (vk::BlendOp::eAdd)
                .setSrcAlphaBlendFactor (vk::BlendFactor::eOne)
                .setDstAlphaBlendFactor (vk::BlendFactor::eZero)
                .setAlphaBlendOp        (vk::BlendOp::eAdd)
            ;
            CurrentRenderingInfo.setPDepthAttachment(nullptr).setPStencilAttachment(nullptr);
        }

        void Create(const vk::raii::Device& I_Device,
                    FVulkanPipelineCache*   I_PipelineCache)
        {
            CurrentRenderingInfo
                .setLayerCount(1)
                .setPDepthAttachment(nullptr)
                .setPStencilAttachment(nullptr)
            ;
            vk::PipelineShaderStageCreateInfo ShaderStageCreateInfos[2]{};
            ShaderStageCreateInfos[0]
                .setStage  (vk::ShaderStageFlagBits::eVertex)
                .setPName  (VertexShader.GetEntryPoint())
                .setModule (VertexShader.GetHandle())
            ;
            ShaderStageCreateInfos[1]
                .setStage  (vk::ShaderStageFlagBits::eFragment)
                .setPName  (FragmentShader.GetEntryPoint())
                .setModule (FragmentShader.GetHandle())
            ;
            DynamicStateCreateInfo
                .setDynamicStateCount (MAX_DYNAMIC_STATE)
                .setPDynamicStates    (DynamicStates)
            ;
            const auto VertexInputInfo = vk::PipelineVertexInputStateCreateInfo{}
                .setVertexAttributeDescriptionCount (Settings.VertexAttributes.GetSize())
                .setPVertexAttributeDescriptions    (Settings.VertexAttributes.Data())
                .setVertexBindingDescriptionCount   (Settings.VertexBindings.GetSize())
                .setPVertexBindingDescriptions      (Settings.VertexBindings.Data())
            ;
            const UInt32 ColorAttachmentCount = Settings.ColorRTFormats.GetSize();
            VISERA_ASSERT(ColorAttachmentCount > 0 && ColorAttachmentCount <= kMaxColorAttachments);
            TInlineArray<vk::PipelineColorBlendAttachmentState, kMaxColorAttachments> BlendAttachments;
            for (UInt32 i = 0; i < ColorAttachmentCount; ++i)
            { BlendAttachments.PushBack(Settings.ColorBlendAttachment); }
            const auto ColorBlending = vk::PipelineColorBlendStateCreateInfo{}
                .setLogicOpEnable   (vk::False)
                .setLogicOp         (vk::LogicOp::eCopy)
                .setAttachmentCount (ColorAttachmentCount)
                .setPAttachments    (BlendAttachments.Data())
            ;
            const auto PipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo{}
                .setColorAttachmentCount    (ColorAttachmentCount)
                .setPColorAttachmentFormats (Settings.ColorRTFormats.Data())
                .setDepthAttachmentFormat   (Settings.DepthRTFormat)
                .setStencilAttachmentFormat (Settings.StencilRTFormat)
            ;
            const auto CreateInfo = vk::GraphicsPipelineCreateInfo{}
                .setPNext               (&PipelineRenderingCreateInfo)
                .setStageCount          (2)
                .setPStages             (ShaderStageCreateInfos)
                .setPVertexInputState   (&VertexInputInfo)
                .setPInputAssemblyState (&Settings.InputAssembly)
                .setPViewportState      (&Settings.ViewportState)
                .setPRasterizationState (&Settings.Rasterizer)
                .setPMultisampleState   (&Settings.Multisampling)
                .setPColorBlendState    (&ColorBlending)
                .setPDynamicState       (&DynamicStateCreateInfo)
                .setLayout              (Layout.GetHandle())
                .setBasePipelineHandle  (nullptr)
                .setBasePipelineIndex   (-1)
                .setRenderPass          (nullptr) // Using Dynamic Rendering.
            ;
            auto Result = I_Device.createGraphicsPipeline(I_PipelineCache->GetHandle(), CreateInfo);
            if (!Result.has_value())
            { LOG_FATAL("Failed to create the graphics pipeline!"); }
            else
            { Handle = std::move(*Result); }

            // Shader modules are stored as value types, no need to reset
        }
    };
    
    FVulkanRenderPipeline* FVulkanRenderPipeline::
    SetColorAttachments(FVulkanColorAttachment* const* I_ColorAttachments, UInt32 I_Count)
    {
        VISERA_ASSERT(I_ColorAttachments != nullptr && I_Count > 0 && I_Count <= kMaxColorAttachments);
        for (UInt32 i = 0; i < I_Count; ++i)
        {
            VISERA_ASSERT(I_ColorAttachments[i] != nullptr);
            ColorAttachmentInfos[i] = I_ColorAttachments[i]->GetAttachmentInfo(vk::ImageLayout::eColorAttachmentOptimal);
        }
        ColorAttachmentCount = I_Count;
        CurrentRenderingInfo
            .setColorAttachmentCount(I_Count)
            .setPColorAttachments(ColorAttachmentInfos);
        return this;
    }

    FVulkanRenderPipeline* FVulkanRenderPipeline::
    SetDepthStencilAttachment(FVulkanDepthStencilAttachment* I_DepthStencilAttachment)
    {
        if (I_DepthStencilAttachment == nullptr)
        { return ClearDepthStencilAttachment(); }
        CurrentDepthStencilAttachment = I_DepthStencilAttachment;
        const auto Layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        DepthAttachmentInfo   = CurrentDepthStencilAttachment->GetDepthAttachmentInfo(Layout);
        StencilAttachmentInfo = CurrentDepthStencilAttachment->GetStencilAttachmentInfo(Layout);
        CurrentRenderingInfo
            .setPDepthAttachment  (&DepthAttachmentInfo)
            .setPStencilAttachment(&StencilAttachmentInfo);
        return this;
    }

    FVulkanRenderPipeline* FVulkanRenderPipeline::
    ClearDepthStencilAttachment()
    {
        CurrentDepthStencilAttachment = nullptr;
        CurrentRenderingInfo
            .setPDepthAttachment  (nullptr)
            .setPStencilAttachment(nullptr);
        return this;
    }
}