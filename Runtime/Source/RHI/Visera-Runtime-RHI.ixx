module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
export import Visera.Runtime.RHI.Types;
       import Visera.Runtime.RHI.SPIRV;
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

        [[nodiscard]] inline TSharedPtr<FVulkanDescriptorSet>
        CreateDescriptorSet(const FRHIDescriptorSetLayout& I_SetLayout);
        [[nodiscard]] inline TSharedPtr<FVulkanBuffer>
        CreateStagingBuffer(UInt64 I_Size);
        [[nodiscard]] inline TSharedPtr<FVulkanBuffer>
        CreateVertexBuffer(UInt64 I_Size);
        [[nodiscard]] inline TSharedPtr<FRHIStaticTexture>
        CreateTexture2D(TSharedRef<FImage> I_Image, ERHISamplerType I_SamplerType);
        [[nodiscard]] inline TSharedPtr<FVulkanRenderPipeline>
        CreateRenderPipeline(const FString&                    I_Name,
                             TSharedRef<FSPIRVShader>          I_VertexShader,
                             TSharedRef<FSPIRVShader>          I_FragmentShader,
                             TSharedPtr<FVulkanPipelineLayout> I_PipelineLayout = {})
        {
            VISERA_ASSERT(I_VertexShader->IsVertexShader());
            VISERA_ASSERT(I_FragmentShader->IsFragmentShader());

            LOG_DEBUG("Creating a Vulkan Render Pass (name:{}).", I_Name);
            auto RenderPipeline = Driver->CreateRenderPipeline(
                I_PipelineLayout? I_PipelineLayout : DefaultPipelineLayout,
                   Driver->CreateShaderModule(I_VertexShader->GetShaderCode()),
                   Driver->CreateShaderModule(I_FragmentShader->GetShaderCode()));
            RenderPipeline->SetRenderArea({
                {0,0},
                {1920, 1080}
                });
            return RenderPipeline;
        }

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

        TSharedPtr<FVulkanDescriptorSetLayout> DefaultDescriptorSetLayout;
        TSharedPtr<FVulkanPipelineLayout>      DefaultPipelineLayout;
        TSharedPtr<FVulkanDescriptorSet>       DefaultDescriptorSet;
        TSharedPtr<FVulkanDescriptorSet>       DefaultDescriptorSet2;
        TSharedPtr<FVulkanSampler>             DefaultLinearSampler;
        TSharedPtr<FVulkanSampler>             DefaultNearestSampler;

    public:
        FRHI() : IGlobalSingleton("RHI") {}
        ~FRHI() override;
        void inline
        Bootstrap() override;
        void inline
        Terminate() override;
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
        Driver.reset();

        Status = EStatus::Terminated;
    }

    FRHI::
    ~FRHI()
    {
        if (IsBootstrapped())
        { LOG_WARN("RHI must be terminated properly!"); }
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
        auto Hash = I_SetLayout.GetOrderedHash();
        auto& Layout = DescriptorSetLayoutPool[Hash];
        if (!Layout)
        {
            LOG_DEBUG("Creating a new descriptor set layout (hash:{}).", Hash);
            UInt32 BindingCount = I_SetLayout.GetBindingCount();
            auto VulkanSetBindings = TArray<vk::DescriptorSetLayoutBinding>(BindingCount);
            for (UInt32 Idx = 0; Idx < BindingCount; Idx++)
            {
                auto& Binding = I_SetLayout.GetBinding(Idx);
                VulkanSetBindings[Idx] = vk::DescriptorSetLayoutBinding{}
                    .setBinding         (Binding.Index)
                    .setDescriptorType  (TypeCast(Binding.Type))
                    .setDescriptorCount (Binding.Count)
                    .setStageFlags      (TypeCast(Binding.ShaderStages))
                    //.setPImmutableSamplers()
                ;
            }
            Layout = Driver->CreateDescriptorSetLayout(VulkanSetBindings);
        }
        return Driver->CreateDescriptorSet(Layout);
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
