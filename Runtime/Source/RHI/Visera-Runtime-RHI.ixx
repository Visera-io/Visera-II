module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Types;
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

        void
        BeginFrame()  const;
        void
        EndFrame()    const;
        inline void
        Present()     const { Driver->Present(); }

        //[TODO]: Remove these test APIs
        [[nodiscard]] inline TSharedRef<FCommandBuffer>
        GetDrawCommands() { return Driver->GetCurrentFrame().DrawCalls; }
        [[nodiscard]] inline TSharedRef<FDescriptorSet>
        GetDefaultDecriptorSet() { return DefaultDescriptorSet; }
        [[nodiscard]] inline TSharedRef<FDescriptorSet>
        GetDefaultDecriptorSet2() { return DefaultDescriptorSet2; }

        [[nodiscard]] inline TSharedPtr<FBuffer>
        CreateStagingBuffer(UInt64 I_Size);
        [[nodiscard]] inline TSharedPtr<FBuffer>
        CreateVertexBuffer(UInt64 I_Size);
        [[nodiscard]] inline TSharedPtr<FStaticTexture2D>
        CreateTexture2D(TSharedRef<FImage> I_Image, Bool bLinearSampler = True);
        [[nodiscard]] inline TSharedPtr<FRenderPipeline>
        CreateRenderPipeline(const FString&              I_Name,
                             TSharedRef<FSPIRVShader>    I_VertexShader,
                             TSharedRef<FSPIRVShader>    I_FragmentShader,
                             TSharedPtr<FPipelineLayout> I_PipelineLayout = {})
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
                 {1920, 1080}});
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
        TUniquePtr<FVulkanDriver>   Driver;
        TSharedPtr<FDescriptorSetLayout> DefaultDescriptorSetLayout;
        TSharedPtr<FPipelineLayout>      DefaultPipelineLayout;
        TSharedPtr<FDescriptorSet>       DefaultDescriptorSet;
        TSharedPtr<FDescriptorSet>       DefaultDescriptorSet2;
        TSharedPtr<FSampler>             DefaultLinearSampler;
        TSharedPtr<FSampler>             DefaultNearestSampler;

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

        DefaultDescriptorSetLayout = Driver->CreateDescriptorSetLayout({
            FDescriptorSetBinding{}
            .setStageFlags      (EShaderStage::eFragment)
            .setBinding         (0)
            .setDescriptorType  (EDescriptorType::eCombinedImageSampler)
            .setDescriptorCount (1)
        });
        DefaultDescriptorSet = Driver->CreateDescriptorSet(DefaultDescriptorSetLayout);
        DefaultDescriptorSet2 = Driver->CreateDescriptorSet(DefaultDescriptorSetLayout);
        DefaultPipelineLayout = Driver->CreatePipelineLayout(
        {DefaultDescriptorSetLayout->GetHandle()},
{
        FPushConstant{}
        .setStageFlags(EShaderStage::eAll)
        .setOffset(0)
        .setSize(sizeof(FDefaultPushConstant))
        });

        DefaultLinearSampler = Driver->CreateImageSampler(
            EFilter::eLinear,
            ESamplerAddressMode::eRepeat);
        DefaultNearestSampler = Driver->CreateImageSampler(
            EFilter::eNearest,
            ESamplerAddressMode::eRepeat);

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
        Driver.reset();

        Status = EStatus::Terminated;
    }

    FRHI::
    ~FRHI()
    {
        if (IsBootstrapped())
        { LOG_WARN("RHI must be terminated properly!"); }
    }


    TSharedPtr<FBuffer> FRHI::
    CreateStagingBuffer(UInt64 I_Size)
    {
        LOG_DEBUG("Creating a staging buffer ({} bytes).", I_Size);
        return Driver->CreateBuffer(
            I_Size,
            EBufferUsage::eTransferSrc,
            EMemoryPoolFlag::eHostAccessAllowTransferInstead |
            EMemoryPoolFlag::eHostAccessSequentialWrite);
    }

    TSharedPtr<FBuffer> FRHI::
    CreateVertexBuffer(UInt64 I_Size)
    {
        return Driver->CreateBuffer(
            I_Size,
            EBufferUsage::eVertexBuffer,
            EMemoryPoolFlag::eHostAccessAllowTransferInstead |
            EMemoryPoolFlag::eHostAccessSequentialWrite |
            EMemoryPoolFlag::eMapped);
    }

    TSharedPtr<FStaticTexture2D> FRHI::
    CreateTexture2D(TSharedRef<FImage> I_Image, Bool bLinearSampler)
    {
        LOG_DEBUG("(WIP) Creating a RHIImage (extent:[{},{}]).",
                  I_Image->GetWidth(), I_Image->GetHeight());
        EFormat Format = EFormat::eUndefined;
        switch (I_Image->GetColorFormat())
        {
        case EColorFormat::RGB:
            Format = I_Image->IsSRGB()?
                     EFormat::eR8G8B8Srgb
                     :
                     EFormat::eR8G8B8Unorm;
            break;
        case EColorFormat::RGBA:
            Format = I_Image->IsSRGB()?
                     EFormat::eR8G8B8A8Srgb
                     :
                     EFormat::eR8G8B8A8Unorm;
            break;
        case EColorFormat::BGR:
            Format = I_Image->IsSRGB()?
                     EFormat::eB8G8R8Srgb
                     :
                     EFormat::eB8G8R8Unorm;
            break;
        case EColorFormat::BGRA:
            Format = I_Image->IsSRGB()?
                     EFormat::eB8G8R8A8Srgb
                     :
                     EFormat::eB8G8R8A8Unorm;
            break;
        default:
            LOG_FATAL("Unsupported color format \"({})\"!",
                      static_cast<Int8>(I_Image->GetColorFormat()));
        }
        auto RHIImage = Driver->CreateImage(
            EImageType::e2D,
            {I_Image->GetWidth(), I_Image->GetHeight(), 1},
            Format,
            EImageUsage::eTransferDst | EImageUsage::eSampled
        );

        auto Cmd = Driver->CreateCommandBuffer(EQueue::eGraphics);
        auto Fence = Driver->CreateFence(False, "Convert Texture Layout");
        auto StagingBuffer = CreateStagingBuffer(I_Image->GetSize());
        StagingBuffer->Write(I_Image->GetData(), I_Image->GetSize());
        Cmd->Begin();
        {
            Cmd->ConvertImageLayout(RHIImage,
                EImageLayout::eTransferDstOptimal,
                EPipelineStage::eTopOfPipe,
                EAccess::eNone,
                EPipelineStage::eTransfer,
                EAccess::eTransferWrite);

            Cmd->CopyBufferToImage(StagingBuffer, RHIImage);

            Cmd->ConvertImageLayout(RHIImage,
                EImageLayout::eShaderReadOnlyOptimal,
                EPipelineStage::eTransfer,
                EAccess::eTransferWrite,
                EPipelineStage::eFragmentShader,
                EAccess::eShaderRead);
        }
        Cmd->End();
        Driver->Submit(Cmd, nullptr, nullptr, Fence);
        if (!Fence->Wait())
        { LOG_FATAL("Failed to wait on the fence (desc:{})!", Fence->GetDescription()); }

        auto RHIImageView = Driver->CreateImageView(
            RHIImage,
            EImageViewType::e2D,
            EImageAspect::eColor);
        return MakeShared<FStaticTexture2D>(
            RHIImageView,
            bLinearSampler? DefaultLinearSampler : DefaultNearestSampler);
    }

    void FRHI::
    BeginFrame() const
    {
        Driver->BeginFrame();
    }

    void FRHI::
    EndFrame() const
    {
        Driver->EndFrame();
    }
}
