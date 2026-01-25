module;
#include <Visera-RHI.hpp>
export module Visera.RHI;
#define VISERA_MODULE_NAME "RHI"
export import Visera.RHI.Common;
export import Visera.RHI.Resource;
export import Visera.RHI.CommandList;
       import Visera.RHI.Vulkan;
       import Visera.RHI.Registry;
       import Visera.Core.Types.Array;
       import Visera.Core.Delegate;
       import Visera.Global.Service;
       import Visera.Global.Log;
       import vulkan_hpp;

export namespace Visera
{
    using FRHIDrawCalls     = FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>;
    using FRHITransferCalls = FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>;

    class VISERA_RHI_API FRHI : public IGlobalService
    {
    public:
        TMulticastDelegate<>
        OnBeginFrame;
        TMulticastDelegate<>
        OnEndFrame;

        [[nodiscard]] FRHITextureHandle
        CreateTexture(FRHITextureCreateDesc&& I_TextureDesc);
        void
        DestroyTexture(FRHITextureHandle I_TextureHandle, Bool I_bTransient = False);
        [[nodiscard]] FRHIBufferHandle
        CreateBuffer(FRHIBufferCreateDesc&& I_BufferDesc,
                     const FByte*           I_InitialData       = nullptr,
                     UInt64                 I_InitialDataSize   = 0);
        void
        DestroyBuffer(FRHIBufferHandle I_BufferHandle, Bool I_bTransient = False);
        [[nodiscard]] FRHISamplerHandle
        CreateSampler(FRHISamplerCreateDesc&& I_SamplerDesc);
        void
        DestroySampler(FRHISamplerHandle I_SamplerHandle, Bool I_bTransient = False);

        [[nodiscard]] Bool
        BeginFrame();
        void
        EndFrame();
        void
        Submit(const FRHICommandList& I_CommandList);
        void
        Present();

        // Low-level API
        [[nodiscard]] inline const TUniquePtr<FVulkanDriver>&
        GetDriver(DEBUG_ONLY_FIELD(const std::source_location& I_Location = std::source_location::current()))  const
        {
            DEBUG_ONLY_FIELD(LOG_WARN("\"{}\" line:{} \"{}\" accessed the RHI driver.",
                             I_Location.file_name(),
                             I_Location.line(),
                             I_Location.function_name()));
            return Driver;
        }

    private:
        TUniquePtr<FVulkanDriver>   Driver;
        TUniquePtr<FRHIRegistry>    Registry;

        FVulkanCommandPool<EVulkanQueueFamily::Graphics>
        GraphicsCommandPool;
        FVulkanCommandPool<EVulkanQueueFamily::Transfer>
        TransferCommandPool;
        //[TODO]: Remove
        vk::raii::DescriptorPool
        GlobalDescriptorPool {nullptr};
        FVulkanDescriptorSetLayout
        GlobalDescriptorSetLayout;

        struct FFrame
        {
            FVulkanFence      Fence;
#if !defined(VISERA_OFFSCREEN_MODE)
            FVulkanSemaphore  SwapChainReadySemaphore;
#endif
            FVulkanSemaphore  RenderFinishedSemaphore;
            FRHIDrawCalls     DrawCalls;

            FVulkanSemaphore  TransferFinishedSemaphore;
            FRHITransferCalls TransferCalls;
        };
        TArray<FFrame> InFlightFrames;
        UInt8          FrameIndex = 0;

    public:
        FRHI() : IGlobalService(EName::RHI)
        {
            Dependencies =
            {
                EName::Platform,
#if !defined(VISERA_OFFSCREEN_MODE)
                EName::Window,
#endif
            };

            if (!OnBootstrap.TryBind([this]
            {
                Driver   = MakeUnique<FVulkanDriver>();
                if (Driver->GetDevice().GraphicsQueueFamilyIndex
                    !=
                    Driver->GetDevice().TransferQueueFamilyIndex)
                { LOG_WARN("NOT support \"Queue Family Ownership Transfer\"!"); }

                Registry = MakeUnique<FRHIRegistry>(Driver);

                // Command Pools
                GraphicsCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Graphics>
                (
                    False
                );
                TransferCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Transfer>
                (
                    False
                );

                // Descriptor Pools
                TArray<vk::DescriptorPoolSize> PoolSizes(2);
                PoolSizes[0] = vk::DescriptorPoolSize{}
                    .setType(vk::DescriptorType::eUniformBuffer)
                    .setDescriptorCount(1000);
                PoolSizes[1] = vk::DescriptorPoolSize{}
                    .setType(vk::DescriptorType::eCombinedImageSampler)
                    .setDescriptorCount(1000);

                auto DescritorPoolCreateInfo = vk::DescriptorPoolCreateInfo{}
                    .setMaxSets(Driver->GetGPU().Properties.limits.maxBoundDescriptorSets)
                    .setPoolSizeCount(PoolSizes.GetSize())
                    .setPPoolSizes(PoolSizes.Data())
                ;
                auto Result = Driver->GetDevice().Context.createDescriptorPool(DescritorPoolCreateInfo);
                if (Result.has_value())
                {
                    GlobalDescriptorPool = std::move(*Result);
                }
                else { LOG_FATAL("Failed to create the global descriptor pool!"); }

                TArray<vk::DescriptorSetLayoutBinding> Bindings(2);
                Bindings[0] = vk::DescriptorSetLayoutBinding{}
                    .setBinding(0)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setDescriptorCount(1)
                    .setStageFlags(vk::ShaderStageFlagBits::eAll)
                ;
                Bindings[1] = vk::DescriptorSetLayoutBinding{}
                    .setBinding(1)
                    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                    .setDescriptorCount(1)
                    .setStageFlags(vk::ShaderStageFlagBits::eAll)
                ;
                GlobalDescriptorSetLayout = Driver->CreateDescriptorSetLayout(Bindings);

#if !defined(VISERA_OFFSCREEN_MODE)
                auto Cmd = GraphicsCommandPool.CreateCommandBuffer(True);
                Cmd.Begin();
                {
                    for (auto& Image : Driver->GetSwapChain().Images)
                    Cmd.ConvertImageLayout(&Image,
                        vk::ImageLayout::ePresentSrcKHR,
                        EVulkanGraphicsStage::TopOfPipe,
                        EVulkanGraphicsAccess::None,
                        EVulkanGraphicsStage::BottomOfPipe,
                        EVulkanGraphicsAccess::None);
                }
                Cmd.End();
                auto Fence = Driver->CreateFence(False);

                Driver->Submit(&Cmd, nullptr, nullptr, &Fence);
                if (!Fence.Wait())
                {
                    LOG_FATAL("Failed to init RHI SwapChain!");
                }

                InFlightFrames.Resize(Driver->GetSwapChain().Images.GetSize());
                for (auto& Frame : InFlightFrames)
                {
                    Frame.Fence = Driver->CreateFence(True);

                    Frame.SwapChainReadySemaphore = Driver->CreateSemaphore();

                    Frame.RenderFinishedSemaphore = Driver->CreateSemaphore();
                    Frame.DrawCalls = GraphicsCommandPool.CreateCommandBuffer(True);

                    Frame.TransferFinishedSemaphore = Driver->CreateSemaphore();
                    Frame.TransferCalls = TransferCommandPool.CreateCommandBuffer(True);
                }
#else
                InFlightFrames.Resize(1);
                for (auto& Frame : InFlightFrames)
                {
                    Frame.Fence = Driver->CreateFence(True);

                    Frame.RenderFinishedSemaphore = Driver->CreateSemaphore();
                    Frame.DrawCalls = GraphicsCommandPool.CreateCommandBuffer(True);

                    Frame.TransferFinishedSemaphore = Driver->CreateSemaphore();
                    Frame.TransferCalls = TransferCommandPool.CreateCommandBuffer(True);
                }
#endif
                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                Driver->WaitIdle();
                InFlightFrames.Clear();
                GlobalDescriptorSetLayout = {};
                GlobalDescriptorPool.clear();
                GraphicsCommandPool = {};
                TransferCommandPool = {};
                Registry.reset();
                Driver.reset();
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    private:
        // Map a single vk::ImageLayout to the stage/access for barrier src or dst.
        static void MapGraphicsLayoutToBarrier(
            vk::ImageLayout         I_Layout,
            EVulkanGraphicsStage*  IO_Stage,
            EVulkanGraphicsAccess* IO_Access);

        // Infer src/dst from old and new layout (map both, then use as barrier).
        void InferGraphicsBarrier(
            vk::ImageLayout        I_OldLayout,
            vk::ImageLayout        I_NewLayout,
            EVulkanGraphicsStage*  IO_SrcStage,
            EVulkanGraphicsAccess* IO_SrcAccess,
            EVulkanGraphicsStage*  IO_DstStage,
            EVulkanGraphicsAccess* IO_DstAccess) const;

        static void MapTransferLayoutToBarrier(
            vk::ImageLayout         I_Layout,
            EVulkanTransferStage*  IO_Stage,
            EVulkanTransferAccess* IO_Access);

        void InferTransferBarrier(
            vk::ImageLayout        I_OldLayout,
            vk::ImageLayout        I_NewLayout,
            EVulkanTransferStage*  IO_SrcStage,
            EVulkanTransferAccess* IO_SrcAccess,
            EVulkanTransferStage*  IO_DstStage,
            EVulkanTransferAccess* IO_DstAccess) const;
    };

    Bool FRHI::
    BeginFrame()
    {
        FrameIndex = (FrameIndex + 1) % InFlightFrames.GetSize();
        auto& CurrentFrame = InFlightFrames[FrameIndex];

        if (!CurrentFrame.Fence.Wait()) { return False; }

        Registry->CollectGarbage(FrameIndex);
#if !defined(VISERA_OFFSCREEN_MODE)
        if (Driver->WaitNextFrame(&CurrentFrame.SwapChainReadySemaphore))
        {
            if (!CurrentFrame.Fence.Reset())
            {
                LOG_ERROR("Failed to reset the Fence!");
                return False;
            }
        }
        else
        {
            LOG_ERROR("Failed to begin new frame!");
            return False;
        }
#endif
        CurrentFrame.TransferCalls.Reset();
        CurrentFrame.TransferCalls.Begin();
        CurrentFrame.DrawCalls.Reset();
        CurrentFrame.DrawCalls.Begin();
        OnBeginFrame.Broadcast();

        return True;
    }

    void FRHI::
    EndFrame()
    {
        auto& CurrentFrame = InFlightFrames[FrameIndex];
        OnEndFrame.Broadcast();

        CurrentFrame.TransferCalls.End();
        CurrentFrame.DrawCalls.End();

        Driver->Submit(&CurrentFrame.TransferCalls,
#if !defined(VISERA_OFFSCREEN_MODE)
        &CurrentFrame.SwapChainReadySemaphore,
#else
        nullptr,
#endif
        &CurrentFrame.TransferFinishedSemaphore,
        nullptr);

        Driver->Submit(&CurrentFrame.DrawCalls,
        &CurrentFrame.TransferFinishedSemaphore,
        &CurrentFrame.RenderFinishedSemaphore,
        &CurrentFrame.Fence);

        Registry->ClearGarbage();
    }

    void FRHI::
    Submit(const FRHICommandList& I_CommandList)
    {
        auto& Frame = InFlightFrames[FrameIndex];
        VISERA_ASSERT(Frame.DrawCalls.IsRecording());
        VISERA_ASSERT(Frame.TransferCalls.IsRecording());

        for (auto Command : I_CommandList)
        {
            if (Command.PayloadPtrAligned == nullptr) { continue; }

            switch (Command.Type)
            {
            case ECommandType::ConvertImageLayout:
                {
                    const auto* Payload = reinterpret_cast<const FRHICommandList::FConvertImageLayout*>(Command.PayloadPtrAligned);

                    auto* Texture = Registry->GetTexture(Payload->Image);
                    VISERA_ASSERT(Texture);

                    auto* Img = Texture->GetVulkanImage();
                    const auto OldLayout = Img->GetLayout();
                    const auto NewLayout = TypeCast(Payload->NewLayout);

                    EVulkanGraphicsStage  SrcStage{},  DstStage{};
                    EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
                    InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);

                    Frame.DrawCalls.ConvertImageLayout(Img, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
                    break;
                }
            case ECommandType::ClearColorImage:
                {
                    const auto* Payload = reinterpret_cast<const FRHICommandList::FClearColorImage*>(Command.PayloadPtrAligned);

                    auto* Texture = Registry->GetTexture(Payload->Image);
                    VISERA_ASSERT(Texture);

                    Frame.DrawCalls.ClearColorImage(Texture->GetVulkanImage(),{
                        Payload->ClearColor.R,
                        Payload->ClearColor.G,
                        Payload->ClearColor.B,
                        Payload->ClearColor.A,
                    });
                    break;
                }
            case ECommandType::BlitImage:
                {
                    const auto* Payload = reinterpret_cast<const FRHICommandList::FBlitImage*>(Command.PayloadPtrAligned);

                    auto* SrcTexture = Registry->GetTexture(Payload->SrcImage);
                    auto* DstTexture = Registry->GetTexture(Payload->DstImage);
                    VISERA_ASSERT(SrcTexture && DstTexture);

                    Frame.DrawCalls.BlitImage(
                        SrcTexture->GetVulkanImage(),
                        DstTexture->GetVulkanImage(),
                        TypeCast(Payload->Filter));
                    break;
                }
            case ECommandType::BlitToSwapChain:
                {
#if !defined(VISERA_OFFSCREEN_MODE)
                    const auto* Payload = reinterpret_cast<const FRHICommandList::FBlitToSwapChain*>(Command.PayloadPtrAligned);

                    auto* Texture = Registry->GetTexture(Payload->Image);
                    VISERA_ASSERT(Texture);
                    auto* SwapChainImage = Driver->GetSwapChain().GetCurrentImage();
                    {
                        const auto OldLayout = SwapChainImage->GetLayout();
                        const auto NewLayout = TypeCast(ERHIImageLayout::TransferDst);
                        EVulkanGraphicsStage  SrcStage{},  DstStage{};
                        EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
                        InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
                        Frame.DrawCalls.ConvertImageLayout(SwapChainImage, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
                    }
                    Frame.DrawCalls.BlitImage(
                        Texture->GetVulkanImage(),
                        SwapChainImage,
                        TypeCast(Payload->Filter));
                    {
                        const auto OldLayout = SwapChainImage->GetLayout();
                        const auto NewLayout = TypeCast(ERHIImageLayout::Present);
                        EVulkanGraphicsStage  SrcStage{},  DstStage{};
                        EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
                        InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
                        Frame.DrawCalls.ConvertImageLayout(SwapChainImage, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
                    }
#endif
                    break;
                }
            case ECommandType::CopyBufferToImage:
                {
                    const auto* Payload = reinterpret_cast<const FRHICommandList::FCopyBufferToImage*>(Command.PayloadPtrAligned);
                    auto* Buffer  = Registry->GetBuffer(Payload->Buffer);
                    auto* Texture = Registry->GetTexture(Payload->Image);
                    VISERA_ASSERT(Buffer && Texture);
                    auto* VulkanBuffer = Buffer->GetVulkanBuffer();
                    auto* VulkanImage  = Texture->GetVulkanImage();
                    const auto OldLayout = VulkanImage->GetLayout();
                    const auto TransferDst = TypeCast(ERHIImageLayout::TransferDst);
                    {
                        EVulkanTransferStage  SrcStage{}, DstStage{};
                        EVulkanTransferAccess SrcAccess{}, DstAccess{};
                        InferTransferBarrier(OldLayout, TransferDst, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
                        Frame.TransferCalls.ConvertImageLayout(VulkanImage, TransferDst, SrcStage, SrcAccess, DstStage, DstAccess);
                    }
                    Frame.TransferCalls.CopyBufferToImage(VulkanBuffer, VulkanImage);
                    {
                        EVulkanTransferStage  SrcStage{}, DstStage{};
                        EVulkanTransferAccess SrcAccess{}, DstAccess{};
                        InferTransferBarrier(TransferDst, OldLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
                        Frame.TransferCalls.ConvertImageLayout(VulkanImage, OldLayout, SrcStage, SrcAccess, DstStage, DstAccess);
                    }
                    break;
                }
            case ECommandType::WriteBuffer:
                {
                    const auto* Payload = reinterpret_cast<const FRHICommandList::FWriteBuffer*>(Command.PayloadPtrAligned);
                    auto Buffer = Registry->GetBuffer(Payload->Buffer);
                    VISERA_ASSERT(Buffer && Payload->Data);
                    auto* VulkanBuffer = Buffer->GetVulkanBuffer();
                    if (VulkanBuffer->IsHostWritable())
                    {
                        VulkanBuffer->Write(Payload->Data, Payload->Size);
                    }
                    else
                    {
                        // Copy By Staging Buffer
                        VISERA_UNIMPLEMENTED_API;
                    }
                    break;
                }
            default: LOG_ERROR("Unknown Command!"); break;
            }
        }
    }

    void FRHI::
    Present()
    {
        auto& CurrentFrame = InFlightFrames[FrameIndex];
        if (!Driver->Present(&CurrentFrame.RenderFinishedSemaphore))
        {
            LOG_ERROR("Failed to present!");
        }
    }

    FRHITextureHandle FRHI::
    CreateTexture(FRHITextureCreateDesc&& I_TextureDesc)
    {
        return Registry->Register(std::move(I_TextureDesc));
    }

    void FRHI::
    DestroyTexture(FRHITextureHandle I_TextureHandle, Bool I_bTransient)
    {
        UInt8 RetiredFrame = (FrameIndex + I_bTransient) % InFlightFrames.GetSize();
        Registry->Unregister(I_TextureHandle, RetiredFrame);
    }

    FRHIBufferHandle FRHI::
    CreateBuffer(FRHIBufferCreateDesc&& I_BufferDesc,
                const FByte*            I_InitialData,
                UInt64                  I_InitialDataSize)
    {
        auto Handle = Registry->Register(std::move(I_BufferDesc));
        if (I_InitialData)
        {
            auto Buffer = Registry->GetBuffer(Handle);
            Buffer->Write(I_InitialData, I_InitialDataSize);
        }
        return Handle;
    }

    void FRHI::
    DestroyBuffer(FRHIBufferHandle I_BufferHandle, Bool I_bTransient)
    {
        UInt8 RetiredFrame = (FrameIndex + I_bTransient) % InFlightFrames.GetSize();
        Registry->Unregister(I_BufferHandle, RetiredFrame);
    }

    FRHISamplerHandle FRHI::
    CreateSampler(FRHISamplerCreateDesc&& I_SamplerDesc)
    {
        return Registry->Register(std::move(I_SamplerDesc));
    }

    void FRHI::
    DestroySampler(FRHISamplerHandle I_SamplerHandle, Bool I_bTransient)
    {
        UInt8 RetiredFrame = (FrameIndex + I_bTransient) % InFlightFrames.GetSize();
        Registry->Unregister(I_SamplerHandle, RetiredFrame);
    }

    void FRHI::
    MapGraphicsLayoutToBarrier(
        vk::ImageLayout         I_Layout,
        EVulkanGraphicsStage*  IO_Stage,
        EVulkanGraphicsAccess* IO_Access)
    {
        VISERA_ASSERT(IO_Stage && IO_Access);

        switch (I_Layout)
        {
        case vk::ImageLayout::eUndefined:
            *IO_Stage  = EVulkanGraphicsStage::TopOfPipe;
            *IO_Access = EVulkanGraphicsAccess::None;
            return;

        case vk::ImageLayout::eTransferDstOptimal:
            *IO_Stage  = EVulkanGraphicsStage::Transfer;
            *IO_Access = EVulkanGraphicsAccess::TransferWrite;
            return;

        case vk::ImageLayout::eTransferSrcOptimal:
            *IO_Stage  = EVulkanGraphicsStage::Transfer;
            *IO_Access = EVulkanGraphicsAccess::TransferRead;
            return;

        case vk::ImageLayout::eColorAttachmentOptimal:
            *IO_Stage  = EVulkanGraphicsStage::ColorAttachmentOutput;
            *IO_Access = EVulkanGraphicsAccess::ColorAttachmentWrite;
            return;

        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            *IO_Stage  = EVulkanGraphicsStage::EarlyFragmentTests | EVulkanGraphicsStage::LateFragmentTests;
            *IO_Access = EVulkanGraphicsAccess::DepthStencilAttachmentWrite;
            return;

        case vk::ImageLayout::eShaderReadOnlyOptimal:
            *IO_Stage  = EVulkanGraphicsStage::FragmentShader;
            *IO_Access = EVulkanGraphicsAccess::ShaderSampledRead;
            return;

        case vk::ImageLayout::eGeneral:
            *IO_Stage  = EVulkanGraphicsStage::FragmentShader;
            *IO_Access = EVulkanGraphicsAccess::ShaderRead | EVulkanGraphicsAccess::ShaderWrite;
            return;

        case vk::ImageLayout::ePresentSrcKHR:
            // For src side: treat present as "no access".
            *IO_Stage  = EVulkanGraphicsStage::BottomOfPipe;
            *IO_Access = EVulkanGraphicsAccess::None;
            return;

        default:
            // Bring-up safe fallback.
            *IO_Stage  = EVulkanGraphicsStage::AllCommands;
            *IO_Access = EVulkanGraphicsAccess::MemoryRead | EVulkanGraphicsAccess::MemoryWrite;
            return;
        }
    }

    void FRHI::
    InferGraphicsBarrier(
        vk::ImageLayout        I_OldLayout,
        vk::ImageLayout        I_NewLayout,
        EVulkanGraphicsStage*  IO_SrcStage,
        EVulkanGraphicsAccess* IO_SrcAccess,
        EVulkanGraphicsStage*  IO_DstStage,
        EVulkanGraphicsAccess* IO_DstAccess) const
    {
        VISERA_ASSERT(IO_SrcStage && IO_SrcAccess && IO_DstStage && IO_DstAccess);

        MapGraphicsLayoutToBarrier(I_OldLayout, IO_SrcStage, IO_SrcAccess);
        MapGraphicsLayoutToBarrier(I_NewLayout, IO_DstStage, IO_DstAccess);

        // If src was read-only, it is still fine. If src was unknown, fallback already covered.
    }

    void FRHI::
    MapTransferLayoutToBarrier(
        vk::ImageLayout        I_Layout,
        EVulkanTransferStage*  IO_Stage,
        EVulkanTransferAccess* IO_Access)
    {
        VISERA_ASSERT(IO_Stage && IO_Access);

        switch (I_Layout)
        {
        case vk::ImageLayout::eUndefined:
            *IO_Stage  = EVulkanTransferStage::TopOfPipe;
            *IO_Access = EVulkanTransferAccess::None;
            return;

        case vk::ImageLayout::eTransferDstOptimal:
            *IO_Stage  = EVulkanTransferStage::Transfer;
            *IO_Access = EVulkanTransferAccess::TransferWrite;
            return;

        case vk::ImageLayout::eTransferSrcOptimal:
            *IO_Stage  = EVulkanTransferStage::Transfer;
            *IO_Access = EVulkanTransferAccess::TransferRead;
            return;

        case vk::ImageLayout::ePresentSrcKHR:
            *IO_Stage  = EVulkanTransferStage::BottomOfPipe;
            *IO_Access = EVulkanTransferAccess::None;
            return;

        default:
            // Transfer queue mainly cares about TransferSrc/Dst; other layouts fallback.
            *IO_Stage  = EVulkanTransferStage::AllCommands;
            *IO_Access = EVulkanTransferAccess::MemoryRead;
            return;
        }
    }

    void FRHI::
    InferTransferBarrier(
        vk::ImageLayout        I_OldLayout,
        vk::ImageLayout        I_NewLayout,
        EVulkanTransferStage*  IO_SrcStage,
        EVulkanTransferAccess* IO_SrcAccess,
        EVulkanTransferStage*  IO_DstStage,
        EVulkanTransferAccess* IO_DstAccess) const
    {
        VISERA_ASSERT(IO_SrcStage && IO_SrcAccess && IO_DstStage && IO_DstAccess);

        MapTransferLayoutToBarrier(I_OldLayout, IO_SrcStage, IO_SrcAccess);
        MapTransferLayoutToBarrier(I_NewLayout, IO_DstStage, IO_DstAccess);
    }
}
