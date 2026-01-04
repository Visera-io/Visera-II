module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.Pipeline.Render;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.RenderTarget;
import Visera.RHI.Vulkan.Pipeline.Cache;
import Visera.RHI.Vulkan.Pipeline.Layout;
import Visera.RHI.Vulkan.ShaderModule;
import Visera.Runtime.Log;
import Visera.Core.Types.Array;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanRenderPipeline
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
        [[nodiscard]] inline FVulkanRenderTarget*
        GetColorRT() const { return CurrentColorRT; }
        inline FVulkanRenderPipeline*
        SetColorRT(FVulkanRenderTarget* I_ColorRT);
        [[nodiscard]] inline FVulkanRenderTarget*
        GetDepthRT() const { return CurrentDepthRT; }
        inline FVulkanRenderPipeline*
        SetDepthRT(FVulkanRenderTarget* I_DepthRT);
        [[nodiscard]] inline FVulkanRenderTarget*
        GetStencilRT() const { return CurrentStencilRT; }
        inline FVulkanRenderPipeline*
        SetStencilRT(FVulkanRenderTarget* I_StencilRT);
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
            ColorRTFormat    {vk::Format::eR16G16B16A16Sfloat};
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

        vk::RenderingInfo               CurrentRenderingInfo;
        FVulkanRenderTarget* CurrentColorRT {nullptr};
        FVulkanRenderTarget* CurrentDepthRT {nullptr};
        FVulkanRenderTarget* CurrentStencilRT {nullptr};

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
                .setVertexAttributeDescriptionCount (Settings.VertexAttributes.size())
                .setPVertexAttributeDescriptions    (Settings.VertexAttributes.data())
                .setVertexBindingDescriptionCount   (Settings.VertexBindings.size())
                .setPVertexBindingDescriptions      (Settings.VertexBindings.data())
            ;
            const auto ColorBlending = vk::PipelineColorBlendStateCreateInfo{}
                .setLogicOpEnable   (vk::False)
                .setLogicOp         (vk::LogicOp::eCopy)
                .setAttachmentCount (1)
                .setPAttachments    (&Settings.ColorBlendAttachment)
            ;
            const auto PipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo{}
                .setPColorAttachmentFormats (&Settings.ColorRTFormat)
                .setColorAttachmentCount    (1)
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
    SetColorRT(FVulkanRenderTarget* I_ColorRT)
    {
        VISERA_ASSERT(I_ColorRT != nullptr);
        static vk::RenderingAttachmentInfo ColorInfo{};

        CurrentColorRT = I_ColorRT;
        ColorInfo = CurrentColorRT->GetAttachmentInfo();
        CurrentRenderingInfo
        .setColorAttachmentCount (1)
        .setPColorAttachments    (&ColorInfo);
        return this;
    }

    FVulkanRenderPipeline* FVulkanRenderPipeline::
    SetDepthRT(FVulkanRenderTarget* I_DepthRT)
    {
        VISERA_ASSERT(I_DepthRT != nullptr);
        static vk::RenderingAttachmentInfo DepthInfo{};

        CurrentDepthRT = I_DepthRT;
        DepthInfo = CurrentDepthRT->GetAttachmentInfo();
        CurrentRenderingInfo
        .setPDepthAttachment(&DepthInfo);
        return this;
    }

    FVulkanRenderPipeline* FVulkanRenderPipeline::
    SetStencilRT(FVulkanRenderTarget* I_StencilRT)
    {
        VISERA_ASSERT(I_StencilRT != nullptr);
        static vk::RenderingAttachmentInfo StencilInfo{};

        CurrentStencilRT = I_StencilRT;
        StencilInfo = CurrentStencilRT->GetAttachmentInfo();
        CurrentRenderingInfo
        .setPStencilAttachment(&StencilInfo);
        return this;
    }
}