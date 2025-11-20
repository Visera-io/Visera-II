module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
export import Visera.Runtime.RHI.Texture;
       import Visera.Runtime.RHI.SPIRV;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.Media.Image;
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

        inline void
        BeginFrame()  const { Driver->BeginFrame(); }
        inline void
        EndFrame()    const { Driver->EndFrame(); }
        inline void
        Present()     const { Driver->Present(); }

        //[TODO]: Remove these test APIs
        [[nodiscard]] inline TSharedRef<RHI::FCommandBuffer>
        GetDrawCommands() { return Driver->GetCurrentFrame().DrawCalls; }
        [[nodiscard]] inline TSharedRef<RHI::FDescriptorSet>
        GetDefaultDecriptorSet() { return DefaultDescriptorSet; }

        [[nodiscard]] inline TSharedPtr<RHI::FBuffer>
        CreateStagingBuffer(UInt64 I_Size);
        [[nodiscard]] inline TSharedPtr<RHI::FStaticTexture2D>
        CreateTexture2D(TSharedRef<FImage> I_Image, Bool bLinearSampler = True);
        [[nodiscard]] inline TSharedPtr<RHI::FRenderPipeline>
        CreateRenderPipeline(const FString&                   I_Name,
                             TSharedRef<RHI::FSPIRVShader>    I_VertexShader,
                             TSharedRef<RHI::FSPIRVShader>    I_FragmentShader,
                             TSharedPtr<RHI::FPipelineLayout> I_PipelineLayout = {})
        {
            VISERA_ASSERT(I_VertexShader->IsVertexShader());
            VISERA_ASSERT(I_FragmentShader->IsFragmentShader());

            LOG_DEBUG("Creating a Vulkan Render Pass (name:{}).", I_Name);
            auto RenderPipeline = Driver->CreateRenderPipeline(
                I_PipelineLayout? I_PipelineLayout : DefaultPipelineLayout,
                   Driver->CreateShaderModule(I_VertexShader),
                   Driver->CreateShaderModule(I_FragmentShader));
            RenderPipeline->SetRenderArea({
                 {0,0},
                 {1920, 1080}});
            return RenderPipeline;
        }

        // Low-level API
        [[nodiscard]] inline const TUniquePtr<RHI::FVulkanDriver>&
        GetDriver(DEBUG_ONLY_FIELD(const std::source_location& I_Location = std::source_location::current()))  const
        {
            DEBUG_ONLY_FIELD(LOG_WARN("\"{}\" line:{} \"{}\" accessed the RHI driver.",
                             I_Location.file_name(),
                             I_Location.line(),
                             I_Location.function_name()));
            return Driver;
        };

    private:
        TUniquePtr<RHI::FVulkanDriver>   Driver;
        TSharedPtr<RHI::FDescriptorSetLayout> DefaultDescriptorSetLayout;
        TSharedPtr<RHI::FPipelineLayout>      DefaultPipelineLayout;
        TSharedPtr<RHI::FDescriptorSet>       DefaultDescriptorSet;
        TSharedPtr<RHI::FSampler>             DefaultLinearSampler;
        TSharedPtr<RHI::FSampler>             DefaultNearestSampler;

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
        Driver = MakeUnique<RHI::FVulkanDriver>();

        DefaultDescriptorSetLayout = Driver->CreateDescriptorSetLayout({
            RHI::FDescriptorSetBinding{}
            .setStageFlags      (RHI::EShaderStage::eFragment)
            .setBinding         (0)
            .setDescriptorType  (RHI::EDescriptorType::eCombinedImageSampler)
            .setDescriptorCount (1)
        });
        DefaultDescriptorSet = Driver->CreateDescriptorSet(DefaultDescriptorSetLayout);
        DefaultPipelineLayout = Driver->CreatePipelineLayout(
        {DefaultDescriptorSetLayout->GetHandle()},
{
        RHI::FPushConstant{}
        .setStageFlags(RHI::EShaderStage::eFragment)
        .setOffset(0)
        .setSize(sizeof(FDefaultPushConstant))
        });

        DefaultLinearSampler = Driver->CreateImageSampler(
            RHI::EFilter::eLinear,
            RHI::ESamplerAddressMode::eRepeat);
        DefaultNearestSampler = Driver->CreateImageSampler(
            RHI::EFilter::eNearest,
            RHI::ESamplerAddressMode::eRepeat);

        Status = EStatus::Bootstrapped;
    }

    void FRHI::
    Terminate()
    {
        LOG_TRACE("Terminating RHI.");
        DefaultNearestSampler.reset();
        DefaultLinearSampler.reset();
        DefaultPipelineLayout.reset();
        DefaultDescriptorSet.reset();
        DefaultDescriptorSetLayout.reset();
        Driver.reset();

        Status = EStatus::Terminated;
    }

    FRHI::
    ~FRHI()
    {
        if (IsBootstrapped())
        { LOG_WARN("RHI must be terminated properly!"); }
    }


    TSharedPtr<RHI::FBuffer> FRHI::
    CreateStagingBuffer(UInt64 I_Size)
    {
        LOG_DEBUG("Creating a staging buffer ({} bytes).", I_Size);
        return Driver->CreateBuffer(
            I_Size,
            RHI::EBufferUsage::eTransferSrc,
            RHI::EMemoryPoolFlag::eHostAccessAllowTransferInstead |
            RHI::EMemoryPoolFlag::eHostAccessSequentialWrite);
    }

    TSharedPtr<RHI::FStaticTexture2D> FRHI::
    CreateTexture2D(TSharedRef<FImage> I_Image, Bool bLinearSampler)
    {
        LOG_DEBUG("(WIP) Creating a RHIImage (extent:[{},{}]).",
                  I_Image->GetWidth(), I_Image->GetHeight());
        RHI::EFormat Format = RHI::EFormat::eUndefined;
        switch (I_Image->GetColorFormat())
        {
        case EColorFormat::RGB:
            Format = I_Image->IsSRGB()?
                     RHI::EFormat::eR8G8B8Srgb
                     :
                     RHI::EFormat::eR8G8B8Unorm;
            break;
        case EColorFormat::RGBA:
            Format = I_Image->IsSRGB()?
                     RHI::EFormat::eR8G8B8A8Srgb
                     :
                     RHI::EFormat::eR8G8B8A8Unorm;
            break;
        case EColorFormat::BGR:
            Format = I_Image->IsSRGB()?
                     RHI::EFormat::eB8G8R8Srgb
                     :
                     RHI::EFormat::eB8G8R8Unorm;
            break;
        case EColorFormat::BGRA:
            Format = I_Image->IsSRGB()?
                     RHI::EFormat::eB8G8R8A8Srgb
                     :
                     RHI::EFormat::eB8G8R8A8Unorm;
            break;
        default:
            LOG_FATAL("Unsupported color format \"({})\"!",
                      static_cast<Int8>(I_Image->GetColorFormat()));
        }
        auto RHIImage = Driver->CreateImage(
            RHI::EImageType::e2D,
            {I_Image->GetWidth(), I_Image->GetHeight(), 1},
            Format,
            RHI::EImageUsage::eTransferDst | RHI::EImageUsage::eSampled
        );

        auto Cmd = Driver->CreateCommandBuffer(RHI::EQueue::eGraphics);
        auto Fence = Driver->CreateFence(False, "Convert Texture Layout");
        auto StagingBuffer = CreateStagingBuffer(I_Image->GetSize());
        StagingBuffer->Write(I_Image->GetData(), I_Image->GetSize());
        Cmd->Begin();
        {
            Cmd->ConvertImageLayout(RHIImage,
                RHI::EImageLayout::eTransferDstOptimal,
                RHI::EPipelineStage::eTopOfPipe,
                RHI::EAccess::eNone,
                RHI::EPipelineStage::eTransfer,
                RHI::EAccess::eTransferWrite);

            Cmd->CopyBufferToImage(StagingBuffer, RHIImage);

            Cmd->ConvertImageLayout(RHIImage,
                RHI::EImageLayout::eShaderReadOnlyOptimal,
                RHI::EPipelineStage::eTransfer,
                RHI::EAccess::eTransferWrite,
                RHI::EPipelineStage::eFragmentShader,
                RHI::EAccess::eShaderRead);
        }
        Cmd->End();
        Driver->Submit(Cmd, nullptr, nullptr, Fence);
        if (!Fence->Wait())
        { LOG_FATAL("Failed to wait on the fence (desc:{})!", Fence->GetDescription()); }

        auto RHIImageView = Driver->CreateImageView(
            RHIImage,
            RHI::EImageViewType::e2D,
            RHI::EImageAspect::eColor);
        return MakeShared<RHI::FStaticTexture2D>(
            RHIImageView,
            bLinearSampler? DefaultLinearSampler : DefaultNearestSampler);
    }
}
