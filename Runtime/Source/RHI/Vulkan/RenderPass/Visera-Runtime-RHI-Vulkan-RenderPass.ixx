module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.RenderPass;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.PipelineCache;
import Visera.Runtime.RHI.Vulkan.ShaderModule;
import Visera.Runtime.RHI.Vulkan.RenderTarget;
import Visera.Core.Types.Text;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanRenderPass
    {
    public:
        static constexpr auto ColorRTFormat = EVulkanFormat::eB8G8R8A8Srgb; //[TODO]: RGBA
        static constexpr auto DepthRTFormat = EVulkanFormat::eD32Sfloat;

        [[nodiscard]] inline const vk::raii::RenderPass&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline const vk::raii::Pipeline&
        GetPipeline() const { return Pipeline; }
        [[nodiscard]] inline const vk::raii::PipelineLayout&
        GetPipelineLayout() const { return PipelineLayout; }
        [[nodiscard]] inline const FVulkanRect2D&
        GetRenderArea() const { return CurrentRenderArea; }
        inline FVulkanRenderPass*
        SetColorRT(TSharedRef<FVulkanRenderTarget> I_ColorRT);
        inline FVulkanRenderPass*
        SetDepthRT(TSharedRef<FVulkanRenderTarget> I_DepthRT);
        inline FVulkanRenderPass*
        SetRenderArea(const FVulkanRect2D& I_RenderArea) { CurrentRenderArea = I_RenderArea; return this; }
        [[nodiscard]] inline auto
        GetRenderingInfo() const;

    private:
        FText                     Name;
        vk::raii::RenderPass      Handle         {nullptr};
        vk::raii::PipelineLayout  PipelineLayout {nullptr};
        vk::raii::Pipeline        Pipeline       {nullptr};

        TSharedPtr<FVulkanRenderTarget> CurrentColorRT;
        TSharedPtr<FVulkanRenderTarget> CurrentDepthRT;
        FVulkanRect2D                   CurrentRenderArea{};

        TSharedPtr<FVulkanShaderModule> VertexShader;
        TSharedPtr<FVulkanShaderModule> FragmentShader;

        enum : UInt8 { MAX_DYNAMIC_STATE = 2 };
        static inline vk::DynamicState
        DynamicStates[MAX_DYNAMIC_STATE]
        {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor,
        };
        Bool         bColorBlendEnabled { False };
    public:
        FVulkanRenderPass() = delete;
        FVulkanRenderPass(const FText&                     I_Name,
                          const vk::raii::Device&          I_Device,
                          TSharedPtr<FVulkanShaderModule>  I_VertexShader,
                          TSharedPtr<FVulkanShaderModule>  I_FragmentShader,
                          TUniqueRef<FVulkanPipelineCache> I_PipelineCache);
    };

    FVulkanRenderPass::
    FVulkanRenderPass(const FText&                     I_Name,
                      const vk::raii::Device&          I_Device,
                      TSharedPtr<FVulkanShaderModule>  I_VertexShader,
                      TSharedPtr<FVulkanShaderModule>  I_FragmentShader,
                      TUniqueRef<FVulkanPipelineCache> I_PipelineCache)
    : Name{I_Name},
      VertexShader   (std::move(I_VertexShader)),
      FragmentShader (std::move(I_FragmentShader))
    {
        VISERA_ASSERT(VertexShader->IsVertexShader());
        VISERA_ASSERT(FragmentShader->IsFragmentShader());

        vk::PipelineShaderStageCreateInfo ShaderStageCreateInfos[2]{};
        ShaderStageCreateInfos[0]
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setPName(VertexShader->GetEntryPoint())
            .setModule(VertexShader->GetHandle())
        ;
        ShaderStageCreateInfos[1]
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setPName(FragmentShader->GetEntryPoint())
            .setModule(FragmentShader->GetHandle())
        ;
        auto DynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo{}
            .setDynamicStateCount   (MAX_DYNAMIC_STATE)
            .setPDynamicStates      (DynamicStates)
        ;
        auto VertexInputInfo = vk::PipelineVertexInputStateCreateInfo{}
            ;
        auto InputAssembly = vk::PipelineInputAssemblyStateCreateInfo{}
            .setTopology(vk::PrimitiveTopology::eTriangleList)
        ;
        auto ViewportState = vk::PipelineViewportStateCreateInfo{}
            .setViewportCount   (1)
            .setPViewports      (nullptr) // Dynamic Viewport
            .setScissorCount    (1)
            .setPScissors       (nullptr) // Dynamic Scissor
        ;
        auto Rasterizer = vk::PipelineRasterizationStateCreateInfo{}
            .setDepthClampEnable        (vk::False)
            .setRasterizerDiscardEnable (vk::False)
            .setPolygonMode             (vk::PolygonMode::eFill)
            .setCullMode                (vk::CullModeFlagBits::eBack)
            .setFrontFace               (vk::FrontFace::eClockwise)
            .setDepthBiasEnable         (vk::False)
            .setDepthBiasSlopeFactor    (1.0)
            .setLineWidth               (1.0)
        ;
        auto Multisampling = vk::PipelineMultisampleStateCreateInfo{}
            .setRasterizationSamples    (vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable     (vk::False)
        ;
        auto ColorBlendAttachment = vk::PipelineColorBlendAttachmentState{}
            .setColorWriteMask      (vk::ColorComponentFlagBits::eR |
                                     vk::ColorComponentFlagBits::eG |
                                     vk::ColorComponentFlagBits::eB |
                                     vk::ColorComponentFlagBits::eA)
            .setBlendEnable         (vk::False)
            .setBlendEnable         (bColorBlendEnabled? vk::True : vk::False)
            .setSrcColorBlendFactor (vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor (vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp        (vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor (vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor (vk::BlendFactor::eZero)
            .setAlphaBlendOp        (vk::BlendOp::eAdd)
        ;
        auto ColorBlending = vk::PipelineColorBlendStateCreateInfo{}
            .setLogicOpEnable   (vk::False)
            .setLogicOp         (vk::LogicOp::eCopy)
            .setAttachmentCount (1)
            .setPAttachments    (&ColorBlendAttachment)
        ;
        struct alignas(16) FTestPushConstantRange
        {
            Float Time{0};
            Float CursorX{0}, CursorY{0};
            Float OffsetX{0}, OffsetY{0};
        };
        auto PushConstantRange = vk::PushConstantRange{}
            .setStageFlags  (EVulkanShaderStage::eFragment)
            .setOffset      (0)
            .setSize        (sizeof(FTestPushConstantRange))
        ;
        auto PipelineLayoutInfo = vk::PipelineLayoutCreateInfo{}
            .setSetLayoutCount          (0)
            .setPSetLayouts             (nullptr)
            .setPushConstantRangeCount  (1)
            .setPPushConstantRanges     (&PushConstantRange)
        ;
        // Create Pipeline Layout
        {
            auto Result = I_Device.createPipelineLayout(PipelineLayoutInfo);
            if (!Result.has_value())
            {
                LOG_FATAL("Failed to create the pipeline layout!");
            }
            else
            { PipelineLayout = std::move(*Result); }
        }

        auto PipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo{}
            .setColorAttachmentCount    (1)
            .setPColorAttachmentFormats (&ColorRTFormat)
        ;
        auto PipelineCreateInfo = vk::GraphicsPipelineCreateInfo{}
            .setPNext               (&PipelineRenderingCreateInfo)
            .setStageCount          (2)
            .setPStages             (ShaderStageCreateInfos)
            .setPVertexInputState   (&VertexInputInfo)
            .setPInputAssemblyState (&InputAssembly)
            .setPViewportState      (&ViewportState)
            .setPRasterizationState (&Rasterizer)
            .setPMultisampleState   (&Multisampling)
            .setPColorBlendState    (&ColorBlending)
            .setPDynamicState       (&DynamicStateCreateInfo)
            .setLayout              (PipelineLayout)
            .setBasePipelineHandle  (nullptr)
            .setBasePipelineIndex   (-1)
            .setRenderPass          (nullptr) // Uusing Dynamic Rendering.
        ;
        // Create Pipeline
        {
            auto Result = I_Device.createGraphicsPipeline(I_PipelineCache->GetHandle(), PipelineCreateInfo);
            if (!Result.has_value())
            {
                LOG_FATAL("Failed to create the pipeline!");
            }
            else
            { Pipeline = std::move(*Result); }
        }
    }

    FVulkanRenderPass* FVulkanRenderPass::
    SetColorRT(TSharedRef<FVulkanRenderTarget> I_ColorRT)
    {
        VISERA_ASSERT(I_ColorRT && I_ColorRT->GetImage());
        VISERA_ASSERT(I_ColorRT->GetFormat() == ColorRTFormat);
        CurrentColorRT = I_ColorRT;
        return this;
    }

    FVulkanRenderPass* FVulkanRenderPass::
    SetDepthRT(TSharedRef<FVulkanRenderTarget> I_DepthRT)
    {
        VISERA_ASSERT(I_DepthRT && I_DepthRT->GetImage());
        VISERA_ASSERT(I_DepthRT->GetFormat() == DepthRTFormat);
        CurrentDepthRT = I_DepthRT;
        return this;
    }

    auto FVulkanRenderPass::
    GetRenderingInfo() const
    {
        VISERA_ASSERT(CurrentColorRT && CurrentColorRT->GetImage());

        static vk::RenderingAttachmentInfo AttachmentInfo{};
        AttachmentInfo = CurrentColorRT->GetAttachmentInfo();

        auto RenderingInfo = vk::RenderingInfo{}
            .setRenderArea          (CurrentRenderArea)
            .setLayerCount          (1)
            .setColorAttachmentCount(1)
            .setPColorAttachments   (&AttachmentInfo)
        ;
        return RenderingInfo;
    }
}
