module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
export import Visera.Runtime.RHI.Types;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.Media.Image;
       import Visera.Core.Types.Map;
       import Visera.Core.Global;
       import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FRHI : public IGlobalSingleton<FRHI>
    {
    public:
        struct alignas(16) VISERA_RUNTIME_API FDefaultPushConstant
        {
            Float DeltaTime{0};
            Float Scale{1.0};
            Float OffsetX{0};
            Float OffsetY{0};
        };

        void
        BeginFrame();
        void
        EndFrame();
        inline void
        Present() { if (!Driver->Present()) { LOG_ERROR("Failed to present!"); } }

        //[TODO]: Remove these test APIs
        [[nodiscard]] inline TSharedRef<FVulkanCommandBuffer>
        GetDrawCommands() { return Driver->GetCurrentFrame().DrawCalls; }
        [[nodiscard]] inline TSharedRef<FVulkanDescriptorSet>
        GetDefaultDecriptorSet() { return DefaultDescriptorSet; }
        [[nodiscard]] inline TSharedRef<FVulkanDescriptorSet>
        GetDefaultDecriptorSet2() { return DefaultDescriptorSet2; }

        [[nodiscard]] inline TSharedPtr<FRHIShader>
        CreateShader(ERHIShaderStages I_ShaderStage, const TArray<FByte>& I_SPIRVShaderCode);
        [[nodiscard]] inline TSharedPtr<FVulkanDescriptorSet>
        CreateDescriptorSet(const FRHIDescriptorSetLayout& I_SetLayout);
        [[nodiscard]] inline TSharedPtr<FVulkanBuffer>
        CreateStagingBuffer(UInt64 I_Size);
        [[nodiscard]] inline TSharedPtr<FVulkanBuffer>
        CreateVertexBuffer(UInt64 I_Size);
        [[nodiscard]] inline TSharedPtr<FRHIStaticTexture>
        CreateTexture2D(TSharedRef<FImage> I_Image, ERHISamplerType I_SamplerType);
        [[nodiscard]] TSharedPtr<FRHIRenderPipeline>
        CreateRenderPipeline(const FString&                 I_Name,
                             const FRHIRenderPipelineState& I_PipelineState);

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

        TSharedPtr<FVulkanDescriptorSetLayout> DefaultDescriptorSetLayout;
        TSharedPtr<FVulkanPipelineLayout>      DefaultPipelineLayout;
        TSharedPtr<FVulkanDescriptorSet>       DefaultDescriptorSet;
        TSharedPtr<FVulkanDescriptorSet>       DefaultDescriptorSet2;
        TSharedPtr<FVulkanSampler>             DefaultLinearSampler;
        TSharedPtr<FVulkanSampler>             DefaultNearestSampler;

    public:
        FRHI() : IGlobalSingleton("RHI") {}
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

    export inline VISERA_RUNTIME_API TUniquePtr<FRHI>
    GRHI = MakeUnique<FRHI>();

    void FRHI::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping RHI.");
        Driver = MakeUnique<FVulkanDriver>();

        auto DescriptorSetLayout = FRHIDescriptorSetLayout{}
            .AddBinding(0, ERHIDescriptorType::CombinedImageSampler, 1, ERHIShaderStages::Fragment)
            .AddBinding(1, ERHIDescriptorType::CombinedImageSampler, 1, ERHIShaderStages::Fragment)
        ;

        DefaultDescriptorSet  = CreateDescriptorSet(DescriptorSetLayout);
        DefaultDescriptorSet2 = CreateDescriptorSet(DescriptorSetLayout);
        DefaultPipelineLayout = Driver->CreatePipelineLayout(
        {DefaultDescriptorSet->GetLayout()->GetHandle()},
{
        vk::PushConstantRange{}
            .setStageFlags(TypeCast(ERHIShaderStages::All))
            .setOffset(0)
            .setSize(sizeof(FDefaultPushConstant))
        });

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
        DefaultPipelineLayout.reset();
        DefaultDescriptorSet2.reset();
        DefaultDescriptorSet.reset();
        DefaultDescriptorSetLayout.reset();
        DescriptorSetLayoutPool.clear();
        PipelineLayoutPool.clear();
        Driver.reset();

        Status = EStatus::Terminated;
    }

    TSharedPtr<FRHIRenderPipeline> FRHI::
    CreateRenderPipeline(const FString&                 I_Name,
                         const FRHIRenderPipelineState& I_PipelineState)
    {
        LOG_DEBUG("Creating the render pipeline (name: {}).", I_Name);
        UInt64 PipelineLayoutHash = I_PipelineState.GetPipelineLayoutHash();
        auto& PipelineLayout = PipelineLayoutPool[PipelineLayoutHash];
        if (!PipelineLayout)
        {
            LOG_DEBUG("Creating a new pipeline layout for the pipeline \"{}\".", I_Name);
            TArray<vk::DescriptorSetLayout> DescriptorSetLayouts;
            for (const auto& DescriptorSetLayout : I_PipelineState.GetDescriptorLayouts())
            {
                auto& SetLayout = GetDescriptorSetLayoutFromPool(DescriptorSetLayout);
                DescriptorSetLayouts.emplace_back(SetLayout->GetHandle());
            }
            TArray<vk::PushConstantRange> PushConstantRanges;
            for (const auto& PushConstantRange : I_PipelineState.GetPushConstantRanges())
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
            I_PipelineState.VertexShader->GetShaderModule(),
            I_PipelineState.FragmentShader->GetShaderModule());
        RenderPipeline->SetRenderArea({
                {0,0},
                {1920, 1080}
                });
        return RenderPipeline;
    }

    TSharedPtr<FVulkanBuffer> FRHI::
    CreateStagingBuffer(UInt64 I_Size)
    {
        LOG_DEBUG("Creating a staging buffer ({} bytes).", I_Size);
        return Driver->CreateBuffer(
            I_Size,
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA::EMemoryPoolFlags::HostAccessAllowTransferInstead |
            VMA::EMemoryPoolFlags::HostAccessSequentialWrite);
    }

    TSharedPtr<FVulkanBuffer> FRHI::
    CreateVertexBuffer(UInt64 I_Size)
    {
        return Driver->CreateBuffer(
            I_Size,
            vk::BufferUsageFlagBits::eVertexBuffer,
            VMA::EMemoryPoolFlags::HostAccessAllowTransferInstead |
            VMA::EMemoryPoolFlags::HostAccessSequentialWrite);
    }

    TSharedPtr<FRHIStaticTexture> FRHI::
    CreateTexture2D(TSharedRef<FImage> I_Image, ERHISamplerType I_SamplerType)
    {
        LOG_DEBUG("(WIP) Creating a RHIImage (extent:[{},{}]).",
                  I_Image->GetWidth(), I_Image->GetHeight());
        ERHIFormat Format = ERHIFormat::Undefined;
        switch (I_Image->GetColorFormat())
        {
        case EColorFormat::RGB:
            Format = I_Image->IsSRGB()?
                     ERHIFormat::R8G8B8_sRGB
                     :
                     ERHIFormat::R8G8B8_UNorm;
            break;
        case EColorFormat::RGBA:
            Format = I_Image->IsSRGB()?
                     ERHIFormat::R8G8B8A8_sRGB
                     :
                     ERHIFormat::R8G8B8A8_UNorm;
            break;
        case EColorFormat::BGR:
            Format = I_Image->IsSRGB()?
                     ERHIFormat::B8G8R8_sRGB
                     :
                     ERHIFormat::B8G8R8_UNorm;
            break;
        case EColorFormat::BGRA:
            Format = I_Image->IsSRGB()?
                     ERHIFormat::B8G8R8A8_sRGB
                     :
                     ERHIFormat::B8G8R8A8_UNorm;
            break;
        default:
            LOG_FATAL("Unsupported color format \"({})\"!",
                      static_cast<Int8>(I_Image->GetColorFormat()));
        }
        auto RHIImage = Driver->CreateImage(
            vk::ImageType::e2D,
            {I_Image->GetWidth(), I_Image->GetHeight(), 1},
            TypeCast(Format),
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
        );

        auto Cmd = Driver->CreateCommandBuffer(vk::QueueFlagBits::eGraphics);
        auto Fence = Driver->CreateFence(False, "Convert Texture Layout");
        auto StagingBuffer = CreateStagingBuffer(I_Image->GetSize());
        StagingBuffer->Write(I_Image->GetData(), I_Image->GetSize());
        Cmd->Begin();
        {
            Cmd->ConvertImageLayout(RHIImage,
                vk::ImageLayout::eTransferDstOptimal,
                vk::PipelineStageFlagBits2::eTopOfPipe,
                vk::AccessFlagBits2::eNone,
                vk::PipelineStageFlagBits2::eTransfer,
                vk::AccessFlagBits2::eTransferWrite);

            Cmd->CopyBufferToImage(StagingBuffer, RHIImage);

            Cmd->ConvertImageLayout(RHIImage,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                vk::PipelineStageFlagBits2::eTransfer,
                vk::AccessFlagBits2::eTransferWrite,
                vk::PipelineStageFlagBits2::eFragmentShader,
                vk::AccessFlagBits2::eShaderRead);
        }
        Cmd->End();
        Driver->Submit(Cmd, nullptr, nullptr, Fence);
        if (!Fence->Wait())
        { LOG_FATAL("Failed to wait on the fence (desc:{})!", Fence->GetDescription()); }

        auto RHIImageView = Driver->CreateImageView(
            RHIImage,
            vk::ImageViewType::e2D,
            vk::ImageAspectFlagBits::eColor);
        return MakeShared<FRHIStaticTexture>(
            RHIImageView,
            I_SamplerType == ERHISamplerType::Linear? DefaultLinearSampler : DefaultNearestSampler);
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
    }

    void FRHI::
    EndFrame()
    {
        Driver->EndFrame();
    }
}
