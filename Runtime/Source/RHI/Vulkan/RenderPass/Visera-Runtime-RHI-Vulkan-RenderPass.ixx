module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.RenderPass;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.PipelineCache;
import Visera.Runtime.RHI.Vulkan.ShaderModule;
import Visera.Runtime.RHI.Vulkan.RenderTarget;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanRenderPass
    {
    public:
        [[nodiscard]] inline const vk::raii::RenderPass&
        GetHandle() const { return Handle; }
        [[nodiscard]] inline const vk::raii::Pipeline&
        GetPipeline() const { return Pipeline; }
        [[nodiscard]] inline const FVulkanRect2D&
        GetRenderArea() const { return CurrentRenderArea; }
        inline void
        SetRenderArea(const FVulkanRect2D& I_RenderArea) { CurrentRenderArea = I_RenderArea; }
        [[nodiscard]] inline auto
        GetRenderingInfo() const;

    private:
        vk::raii::RenderPass      Handle         {nullptr};
        vk::raii::PipelineLayout  PipelineLayout {nullptr};
        vk::raii::Pipeline        Pipeline       {nullptr};

        TArray<FVulkanRenderTarget>     RenderTargets;
        UInt8                           CurrentRenderTargetIndex{0};
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

        vk::Viewport Viewport {};
        vk::Rect2D   Scissor  {};
        Bool         bColorBlendEnabled { False };
    public:
        FVulkanRenderPass() = delete;
        FVulkanRenderPass(const vk::raii::Device&          I_Device,
                          TSharedPtr<FVulkanShaderModule>  I_VertexShader,
                          TSharedPtr<FVulkanShaderModule>  I_FragmentShader,
                          TUniqueRef<FVulkanPipelineCache> I_PipelineCache);
    };

    FVulkanRenderPass::
    FVulkanRenderPass(const vk::raii::Device&          I_Device,
                      TSharedPtr<FVulkanShaderModule>  I_VertexShader,
                      TSharedPtr<FVulkanShaderModule>  I_FragmentShader,
                      TUniqueRef<FVulkanPipelineCache> I_PipelineCache)
    : VertexShader   (std::move(I_VertexShader)),
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
        Viewport = vk::Viewport
        {  0.0f, 0.0f,
            900,
            600,
            0.0f, 1.0f
        };
        Scissor = vk::Rect2D
        {
            vk::Offset2D{ 0, 0 },
            {900, 600}
        };
        auto ViewportState = vk::PipelineViewportStateCreateInfo{}
            .setViewportCount   (1)
            .setPViewports      (&Viewport)
            .setScissorCount    (1)
            .setPScissors       (&Scissor)
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
        auto PipelineLayoutInfo = vk::PipelineLayoutCreateInfo{}
            .setSetLayoutCount          (0)
            .setPushConstantRangeCount  (0)
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

        auto RTFormat = vk::Format::eR8G8B8A8Unorm; //[TODO]: Remove
        auto PipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo{}
            .setColorAttachmentCount    (1)
            .setPColorAttachmentFormats (&RTFormat)
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

    auto FVulkanRenderPass::
    GetRenderingInfo() const
    {
        VISERA_ASSERT(CurrentRenderTargetIndex < RenderTargets.size());

        auto AttachmentInfo = RenderTargets[CurrentRenderTargetIndex]
                              .GetAttachmentInfo();

        auto RenderingInfo = vk::RenderingInfo{}
            .setRenderArea          (CurrentRenderArea)
            .setLayerCount          (1)
            .setColorAttachmentCount(1)
            .setColorAttachments    (AttachmentInfo)
        ;
        return RenderingInfo;
    }
}
