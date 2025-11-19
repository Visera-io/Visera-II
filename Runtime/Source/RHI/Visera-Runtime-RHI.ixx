module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
       import Visera.Runtime.RHI.SPIRV;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.Media.Image;
       import Visera.Runtime.Event;
       import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FRHI : public IGlobalSingleton<FRHI>
    {
    public:
        inline void
        BeginFrame()  const { Driver->BeginFrame(); GEvent->OnBeginFrame.Broadcast(); }
        inline void
        EndFrame()    const { GEvent->OnEndFrame.Broadcast(); Driver->EndFrame(); }
        inline void
        Present()     const { Driver->Present(); }

        [[nodiscard]] inline TSharedRef<RHI::FCommandBuffer>
        GetDrawCommands() { return Driver->GetCurrentFrame().DrawCalls; }
        [[nodiscard]] inline TSharedPtr<RHI::FBuffer>
        CreateStagingBuffer(UInt64 I_Size);
        [[nodiscard]] inline TSharedPtr<RHI::FImage>
        CreateTexture2D(TSharedRef<FImage> I_Image);
        // [[nodiscard]] inline TSharedPtr<RHI::FImage>
        // CreatePipelineLayout(TSharedRef<FImage> I_Image);
        [[nodiscard]] inline TSharedPtr<RHI::FRenderPipeline>
        CreateRenderPipeline(const FString&                   I_Name,
                             TSharedRef<RHI::FPipelineLayout> I_PipelineLayout,
                             TSharedRef<RHI::FSPIRVShader>    I_VertexShader,
                             TSharedRef<RHI::FSPIRVShader>    I_FragmentShader)
        {
            VISERA_ASSERT(I_VertexShader->IsVertexShader());
            VISERA_ASSERT(I_FragmentShader->IsFragmentShader());

            LOG_DEBUG("Creating a Vulkan Render Pass (name:{}).", I_Name);
            return Driver->CreateRenderPipeline(I_PipelineLayout,
                   Driver->CreateShaderModule(I_VertexShader),
                   Driver->CreateShaderModule(I_FragmentShader));
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
        TUniquePtr<RHI::FVulkanDriver> Driver;

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

        Status = EStatus::Bootstrapped;
    }

    void FRHI::
    Terminate()
    {
        LOG_TRACE("Terminating RHI.");
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

    TSharedPtr<RHI::FImage> FRHI::
    CreateTexture2D(TSharedRef<FImage> I_Image)
    {
        LOG_DEBUG("(WIP) Creating a Texture2D (extent:[{},{}]).",
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
        auto Texture2D = Driver->CreateImage(
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
            Cmd->ConvertImageLayout(Texture2D,
                RHI::EImageLayout::eTransferDstOptimal,
                RHI::EPipelineStage::eTopOfPipe,
                RHI::EAccess::eNone,
                RHI::EPipelineStage::eTransfer,
                RHI::EAccess::eTransferWrite);

            Cmd->CopyBufferToImage(StagingBuffer, Texture2D);

            Cmd->ConvertImageLayout(Texture2D,
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

        return Texture2D;
    }
}
