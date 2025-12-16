module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Vulkan.Pipeline.Render;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.RenderTarget;
import Visera.Runtime.RHI.Vulkan.Pipeline.Cache;
import Visera.Runtime.RHI.Vulkan.Pipeline.Layout;
import Visera.Runtime.RHI.Vulkan.ShaderModule;
import Visera.Runtime.Log;
import Visera.Core.Types.Array;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkanRenderPipeline
    {
    public:
        [[nodiscard]] inline const vk::raii::Pipeline&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline TSharedRef<FVulkanPipelineLayout>
        GetLayout() const { return Layout; }
        [[nodiscard]] inline const vk::Rect2D&
        GetRenderArea() const { return CurrentRenderingInfo.renderArea; }
        [[nodiscard]] inline TSharedRef<FVulkanRenderTarget>
        GetColorRT() const { return CurrentColorRT; }
        inline FVulkanRenderPipeline*
        SetColorRT(TSharedRef<FVulkanRenderTarget> I_ColorRT);
        [[nodiscard]] inline TSharedRef<FVulkanRenderTarget>
        GetDepthRT() const { return CurrentDepthRT; }
        inline FVulkanRenderPipeline*
        SetDepthRT(TSharedRef<FVulkanRenderTarget> I_DepthRT);
        [[nodiscard]] inline TSharedRef<FVulkanRenderTarget>
        GetStencilRT() const { return CurrentStencilRT; }
        inline FVulkanRenderPipeline*
        SetStencilRT(TSharedRef<FVulkanRenderTarget> I_StencilRT);
        inline FVulkanRenderPipeline*
        SetRenderArea(const vk::Rect2D& I_RenderArea) { CurrentRenderingInfo.setRenderArea(I_RenderArea); return this; }
        [[nodiscard]] inline const vk::RenderingInfo&
        GetRenderingInfo() const { return CurrentRenderingInfo; }

        struct
        {
            TArray<vk::VertexInputAttributeDescription>
            VertexAttributes;
            TArray<vk::VertexInputBindingDescription>
            VertexBindings;
            vk::PipelineInputAssemblyStateCreateInfo
            InputAssembly{};
            vk::PipelineViewportStateCreateInfo
            ViewportState{};
            vk::PipelineRasterizationStateCreateInfo
            Rasterizer{};
            vk::PipelineMultisampleStateCreateInfo
            Multisampling{};
            vk::PipelineColorBlendAttachmentState
            ColorBlendAttachment{};
            vk::Format
            ColorRTFormat    {vk::Format::eR8G8B8A8Srgb};
            vk::Format
            DepthRTFormat    {vk::Format::eUndefined};
            vk::Format
            StencilRTFormat  {vk::Format::eUndefined};
        }Settings;

    private:
        vk::raii::Pipeline                Handle {nullptr};
        TSharedPtr<FVulkanPipelineLayout> Layout;

        TSharedPtr<FVulkanShaderModule> VertexShader;
        TSharedPtr<FVulkanShaderModule> FragmentShader;

        vk::RenderingInfo               CurrentRenderingInfo;
        TSharedPtr<FVulkanRenderTarget> CurrentColorRT;
        TSharedPtr<FVulkanRenderTarget> CurrentDepthRT;
        TSharedPtr<FVulkanRenderTarget> CurrentStencilRT;

        enum : UInt8 { MAX_DYNAMIC_STATE = 2 };
        static inline vk::DynamicState  DynamicStates[MAX_DYNAMIC_STATE]
        {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
        };
        vk::PipelineDynamicStateCreateInfo
        DynamicStateCreateInfo{};

    public:
        FVulkanRenderPipeline() = delete;
        FVulkanRenderPipeline(TSharedRef<FVulkanPipelineLayout> I_PipelineLayout,
                              TSharedRef<FVulkanShaderModule>   I_VertexShader,
                              TSharedRef<FVulkanShaderModule>   I_FragmentShader)
        :Layout         (I_PipelineLayout),
         VertexShader   (I_VertexShader),
         FragmentShader (I_FragmentShader)
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
        }

        void Create(const vk::raii::Device&          I_Device,
                    TUniqueRef<FVulkanPipelineCache> I_PipelineCache)
        {
            CurrentRenderingInfo
                .setLayerCount(1)
            ;
            vk::PipelineShaderStageCreateInfo ShaderStageCreateInfos[2]{};
            ShaderStageCreateInfos[0]
                .setStage  (vk::ShaderStageFlagBits::eVertex)
                .setPName  (VertexShader->GetEntryPoint())
                .setModule (VertexShader->GetHandle())
            ;
            ShaderStageCreateInfos[1]
                .setStage  (vk::ShaderStageFlagBits::eFragment)
                .setPName  (FragmentShader->GetEntryPoint())
                .setModule (FragmentShader->GetHandle())
            ;
            DynamicStateCreateInfo
                .setDynamicStateCount (MAX_DYNAMIC_STATE)
                .setPDynamicStates    (DynamicStates)
            ;
            auto VertexInputInfo = vk::PipelineVertexInputStateCreateInfo{}
                .setVertexAttributeDescriptionCount (Settings.VertexAttributes.size())
                .setPVertexAttributeDescriptions    (Settings.VertexAttributes.data())
                .setVertexBindingDescriptionCount   (Settings.VertexBindings.size())
                .setPVertexBindingDescriptions      (Settings.VertexBindings.data())
            ;
            auto ColorBlending = vk::PipelineColorBlendStateCreateInfo{}
                .setLogicOpEnable   (vk::False)
                .setLogicOp         (vk::LogicOp::eCopy)
                .setAttachmentCount (1)
                .setPAttachments    (&Settings.ColorBlendAttachment)
            ;
            auto PipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo{}
                .setPColorAttachmentFormats (&Settings.ColorRTFormat)
                .setColorAttachmentCount    (1)
                .setDepthAttachmentFormat   (Settings.DepthRTFormat)
                .setStencilAttachmentFormat (Settings.StencilRTFormat)
            ;
            auto CreateInfo = vk::GraphicsPipelineCreateInfo{}
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
                .setLayout              (Layout->GetHandle())
                .setBasePipelineHandle  (nullptr)
                .setBasePipelineIndex   (-1)
                .setRenderPass          (nullptr) // Using Dynamic Rendering.
            ;
            auto Result = I_Device.createGraphicsPipeline(I_PipelineCache->GetHandle(), CreateInfo);
            if (!Result.has_value())
            { LOG_FATAL("Failed to create the pipeline!"); }
            else
            { Handle = std::move(*Result); }

            // Clear Shader Modules
            VertexShader.reset();
            FragmentShader.reset();
        }
    };
    
    FVulkanRenderPipeline* FVulkanRenderPipeline::
    SetColorRT(TSharedRef<FVulkanRenderTarget> I_ColorRT)
    {
        VISERA_ASSERT(I_ColorRT && I_ColorRT->GetImage());
        static vk::RenderingAttachmentInfo ColorInfo{};

        CurrentColorRT = I_ColorRT;
        ColorInfo = CurrentColorRT->GetAttachmentInfo();
        CurrentRenderingInfo
        .setColorAttachmentCount (1)
        .setPColorAttachments    (&ColorInfo);
        return this;
    }

    FVulkanRenderPipeline* FVulkanRenderPipeline::
    SetDepthRT(TSharedRef<FVulkanRenderTarget> I_DepthRT)
    {
        VISERA_ASSERT(I_DepthRT && I_DepthRT->GetImage());
        static vk::RenderingAttachmentInfo DepthInfo{};

        CurrentDepthRT = I_DepthRT;
        DepthInfo = CurrentDepthRT->GetAttachmentInfo();
        CurrentRenderingInfo
        .setPDepthAttachment(&DepthInfo);
        return this;
    }

    FVulkanRenderPipeline* FVulkanRenderPipeline::
    SetStencilRT(TSharedRef<FVulkanRenderTarget> I_StencilRT)
    {
        VISERA_ASSERT(I_StencilRT && I_StencilRT->GetImage());
        static vk::RenderingAttachmentInfo StencilInfo{};

        CurrentStencilRT = I_StencilRT;
        StencilInfo = CurrentStencilRT->GetAttachmentInfo();
        CurrentRenderingInfo
        .setPStencilAttachment(&StencilInfo);
        return this;
    }
}