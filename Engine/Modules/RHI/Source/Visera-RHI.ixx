module;
#include <Visera-RHI.hpp>
export module Visera.RHI;
#define VISERA_MODULE_NAME "RHI"
export import Visera.RHI.Common;
export import Visera.RHI.Types;
       import Visera.RHI.Vulkan;
       import Visera.Assets.Image;
       import Visera.Runtime.Log;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Array;
       import Visera.Core.Delegate;
       import Visera.Runtime.Global;
       import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FRHI : public IGlobalSingleton<FRHI>
    {
    public:
        TMulticastDelegate<>
        OnBeginFrame;
        TMulticastDelegate<>
        OnEndFrame;
        TUnicastDelegate<void(TSharedRef<FRHICommandBuffer>, TSharedRef<FRHIImageView>)>
        DebugUIDrawCalls;

        void
        BeginFrame();
        void
        EndFrame();
        inline void
        Present() { if (!Driver->Present()) { LOG_ERROR("Failed to present!"); } }

        //[TODO]: Remove these test APIs
        [[nodiscard]] inline TSharedRef<FVulkanCommandBuffer>
        GetDrawCommands() { return Driver->GetCurrentFrame().DrawCalls; }

        [[nodiscard]] inline TSharedPtr<FRHIShader>
        CreateShader(ERHIShaderStages I_ShaderStage, const TArray<FByte>& I_SPIRVShaderCode);
        [[nodiscard]] inline TSharedPtr<FVulkanDescriptorSet>
        CreateDescriptorSet(const FRHIDescriptorSetLayout& I_SetLayout);
        [[nodiscard]] inline TSharedPtr<FVulkanBuffer>
        CreateStagingBuffer(UInt64 I_Size);
        [[nodiscard]] inline TSharedPtr<FVulkanBuffer>
        CreateVertexBuffer(UInt64 I_Size);
        [[nodiscard]] inline TSharedPtr<FVulkanBuffer>
        CreateMappedVertexBuffer(UInt64 I_Size);
        [[nodiscard]] TSharedPtr<FRHIRenderPipeline>
        CreateRenderPipeline(const FString&                 I_Name,
                             const FRHIRenderPipelineDesc& I_PipelineDesc);
        [[nodiscard]] TSharedPtr<FRHIComputePipeline>
        CreateComputePipeline(const FString&                I_Name,
                              const FRHIComputePipelineDesc& I_PipelineDesc);

        // Low-level API
        [[nodiscard]] inline const TUniquePtr<FVulkanDriver>&
        GetDriver(DEBUG_ONLY_FIELD(const std::source_location& I_Location = std::source_location::current()))  const
        {
            DEBUG_ONLY_FIELD(LOG_WARN("\"{}\" line:{} \"{}\" accessed the RHI driver.",
                             I_Location.file_name(),
                             I_Location.line(),
                             I_Location.function_name()));
            return Driver;
        };

    private:
        TUniquePtr<FVulkanDriver> Driver;

        TMap<UInt64, TSharedPtr<FVulkanDescriptorSetLayout>>
        DescriptorSetLayoutPool;
        TMap<UInt64, TSharedPtr<FVulkanPipelineLayout>>
        PipelineLayoutPool;

        TSharedPtr<FVulkanSampler> DefaultLinearSampler;
        TSharedPtr<FVulkanSampler> DefaultNearestSampler;

    public:
        FRHI() : IGlobalSingleton("RHI.Vulkan") {}
        void inline
        Bootstrap() override;
        void inline
        Terminate() override;

    private:
        [[nodiscard]] TSharedRef<FVulkanDescriptorSetLayout> inline
        GetDescriptorSetLayoutFromPool(const FRHIDescriptorSetLayout& I_DescriptorSetLayout)
        {
            auto Hash = I_DescriptorSetLayout.Hash();
            auto& Layout = DescriptorSetLayoutPool[Hash];
            if (!Layout)
            {
                LOG_DEBUG("Creating a new descriptor set layout (hash:{}).", Hash);
                auto VulkanSetBindings = TArray<vk::DescriptorSetLayoutBinding>();
                for (const auto& Binding : I_DescriptorSetLayout.GetBindings())
                {
                    VulkanSetBindings.push_back(vk::DescriptorSetLayoutBinding{}
                        .setBinding         (Binding.Binding)
                        .setDescriptorType  (TypeCast(Binding.Type))
                        .setDescriptorCount (Binding.Count)
                        .setStageFlags      (TypeCast(Binding.ShaderStages))
                        //.setPImmutableSamplers()
                    );
                }
                Layout = Driver->CreateDescriptorSetLayout(VulkanSetBindings);
            }
            return Layout;
        }
    };

    export inline VISERA_RHI_API TUniquePtr<FRHI>
    GRHI = MakeUnique<FRHI>();

    void FRHI::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping RHI.");
        Driver = MakeUnique<FVulkanDriver>();

        DefaultLinearSampler = Driver->CreateImageSampler(
            vk::Filter::eLinear,
            vk::SamplerAddressMode::eRepeat);
        DefaultNearestSampler = Driver->CreateImageSampler(
            vk::Filter::eNearest,
            vk::SamplerAddressMode::eRepeat);

        Status = EStatus::Bootstrapped;
    }

    void FRHI::
    Terminate()
    {
        LOG_TRACE("Terminating RHI.");
        DefaultNearestSampler.reset();
        DefaultLinearSampler.reset();

        DescriptorSetLayoutPool.clear();
        PipelineLayoutPool.clear();
        Driver.reset();

        Status = EStatus::Terminated;
    }

    TSharedPtr<FRHIRenderPipeline> FRHI::
    CreateRenderPipeline(const FString&                 I_Name,
                         const FRHIRenderPipelineDesc&  I_PipelineDesc)
    {
        LOG_DEBUG("Creating the render pipeline (name: {}).", I_Name);
        UInt64 PipelineLayoutHash = I_PipelineDesc.Layout.GetPipelineLayoutHash();
        auto& PipelineLayout = PipelineLayoutPool[PipelineLayoutHash];
        if (!PipelineLayout)
        {
            LOG_DEBUG("Creating a new pipeline layout for the pipeline \"{}\".", I_Name);
            TArray<vk::DescriptorSetLayout> DescriptorSetLayouts;
            for (const auto& DescriptorSetLayout : I_PipelineDesc.Layout.GetDescriptorLayouts())
            {
                auto& SetLayout = GetDescriptorSetLayoutFromPool(DescriptorSetLayout);
                DescriptorSetLayouts.emplace_back(SetLayout->GetHandle());
            }
            TArray<vk::PushConstantRange> PushConstantRanges;
            for (const auto& PushConstantRange : I_PipelineDesc.Layout.GetPushConstantRanges())
            {
                PushConstantRanges.emplace_back(vk::PushConstantRange{}
                    .setSize(PushConstantRange.Size)
                    .setOffset(PushConstantRange.Offset)
                    .setStageFlags(TypeCast(PushConstantRange.Stages)));
            }
            PipelineLayout = Driver->CreatePipelineLayout(DescriptorSetLayouts,
                                                          PushConstantRanges);
        }
        auto RenderPipeline = Driver->CreateRenderPipeline(PipelineLayout,
            I_PipelineDesc.VertexShader->GetShaderModule(),
            I_PipelineDesc.FragmentShader->GetShaderModule());
        VISERA_ASSERT(RenderPipeline != nullptr);

        if (I_PipelineDesc.VertexAssembly.Topology == ERHIPrimitiveTopology::LineStrip)
        {
            LOG_WARN("Testing Line Renderer");
            RenderPipeline->Settings.InputAssembly
            .setTopology(TypeCast(I_PipelineDesc.VertexAssembly.Topology));
            RenderPipeline->Settings.VertexAttributes = {
                vk::VertexInputAttributeDescription{}
                    .setBinding   (0)
                    .setLocation  (0)
                    .setFormat    (TypeCast(ERHIFormat::Vector2F))
                    .setOffset    (0)
            };
            RenderPipeline->Settings.Rasterizer.polygonMode = vk::PolygonMode::eLine;
            RenderPipeline->Settings.VertexBindings = {
                vk::VertexInputBindingDescription{}
                    .setBinding     (0)
                    .setStride      (sizeof(float) * 2)
                    .setInputRate   (vk::VertexInputRate::eVertex)
            };
        }
        else
        {
            RenderPipeline->Settings.VertexAttributes.emplace_back()
                .setLocation (0)
                .setBinding  (0)
                .setFormat   (vk::Format::eR32G32B32A32Sfloat)
                .setOffset   (0)
            ;
            RenderPipeline->Settings.VertexBindings.emplace_back()
                .setBinding     (0)
                .setStride      (sizeof(float) * 4)
                .setInputRate   (vk::VertexInputRate::eVertex)
            ;
        }
        RenderPipeline->Create(Driver->GetNativeDevice(), Driver->GetPipelineCache());

        RenderPipeline->SetRenderArea({
                {0,0},
                {1920, 1080}
                });
        return RenderPipeline;
    }

    TSharedPtr<FRHIComputePipeline> FRHI::
    CreateComputePipeline(const FString&                I_Name,
                         const FRHIComputePipelineDesc& I_PipelineDesc)
    {
        LOG_DEBUG("Creating the compute pipeline (name: {}).", I_Name);
        UInt64 PipelineLayoutHash = I_PipelineDesc.Layout.GetPipelineLayoutHash();
        auto& PipelineLayout = PipelineLayoutPool[PipelineLayoutHash];
        if (!PipelineLayout)
        {
            LOG_DEBUG("Creating a new pipeline layout for the compute pipeline \"{}\".", I_Name);
            TArray<vk::DescriptorSetLayout> DescriptorSetLayouts;
            for (const auto& DescriptorSetLayout : I_PipelineDesc.Layout.GetDescriptorLayouts())
            {
                auto& SetLayout = GetDescriptorSetLayoutFromPool(DescriptorSetLayout);
                DescriptorSetLayouts.emplace_back(SetLayout->GetHandle());
            }
            TArray<vk::PushConstantRange> PushConstantRanges;
            for (const auto& PushConstantRange : I_PipelineDesc.Layout.GetPushConstantRanges())
            {
                PushConstantRanges.emplace_back(vk::PushConstantRange{}
                    .setSize        (PushConstantRange.Size)
                    .setOffset      (PushConstantRange.Offset)
                    .setStageFlags  (TypeCast(PushConstantRange.Stages)));
            }
            PipelineLayout = Driver->CreatePipelineLayout(DescriptorSetLayouts,
                                                          PushConstantRanges);
        }
        auto ComputePipeline = Driver->CreateComputePipeline(PipelineLayout,
                                                             I_PipelineDesc.ComputeShader->GetShaderModule());
        VISERA_ASSERT(ComputePipeline != nullptr);
        return ComputePipeline;
    }

    TSharedPtr<FVulkanBuffer> FRHI::
    CreateStagingBuffer(UInt64 I_Size)
    {
        LOG_DEBUG("Creating a staging buffer ({} bytes).", I_Size);
        return Driver->CreateBuffer(
            I_Size,
            vk::BufferUsageFlagBits::eTransferSrc,
            static_cast<EVulkanMemoryPoolFlags>(
            EVulkanMemoryPoolFlags::HostAccessAllowTransferInstead |
            EVulkanMemoryPoolFlags::HostAccessSequentialWrite));
    }

    TSharedPtr<FVulkanBuffer> FRHI::
    CreateVertexBuffer(UInt64 I_Size)
    {
        return Driver->CreateBuffer(
            I_Size,
            vk::BufferUsageFlagBits::eVertexBuffer,
            static_cast<EVulkanMemoryPoolFlags>(
            EVulkanMemoryPoolFlags::HostAccessAllowTransferInstead |
            EVulkanMemoryPoolFlags::HostAccessSequentialWrite));
    }

    TSharedPtr<FVulkanBuffer> FRHI::
    CreateMappedVertexBuffer(UInt64 I_Size)
    {
        return Driver->CreateBuffer(
            I_Size,
            vk::BufferUsageFlagBits::eVertexBuffer,
            static_cast<EVulkanMemoryPoolFlags>(
            EVulkanMemoryPoolFlags::HostAccessAllowTransferInstead |
            EVulkanMemoryPoolFlags::HostAccessSequentialWrite |
            EVulkanMemoryPoolFlags::Mapped));
    }

    TSharedPtr<FVulkanDescriptorSet> FRHI::
    CreateDescriptorSet(const FRHIDescriptorSetLayout& I_SetLayout)
    {
        auto& Layout = GetDescriptorSetLayoutFromPool(I_SetLayout);
        return Driver->CreateDescriptorSet(Layout);
    }

    TSharedPtr<FRHIShader> FRHI::
    CreateShader(ERHIShaderStages I_ShaderStage, const TArray<FByte>& I_SPIRVShaderCode)
    {
        LOG_DEBUG("Creating a new shader (stage:{}, size:{}).",
                  I_ShaderStage, I_SPIRVShaderCode.size());

        auto ShaderModule = Driver->CreateShaderModule(I_SPIRVShaderCode);
        if (!ShaderModule)
        {
            LOG_ERROR("Failed to create the shader (stage:{}, size:{})!",
                      I_ShaderStage, I_SPIRVShaderCode.size());
            return {};
        }
        return MakeShared<FRHIShader>(I_ShaderStage, ShaderModule);
    }

    void FRHI::
    BeginFrame()
    {
        while (!Driver->BeginFrame())
        {
            LOG_FATAL("Failed to begin new frame!");
        }
        OnBeginFrame.Broadcast();
    }

    void FRHI::
    EndFrame()
    {
        OnEndFrame.Broadcast();

        auto& CurrentFrame = Driver->GetCurrentFrame();
        DebugUIDrawCalls.Invoke(CurrentFrame.DrawCalls, CurrentFrame.ColorRT->GetImageView());
        Driver->EndFrame();
    }
}
