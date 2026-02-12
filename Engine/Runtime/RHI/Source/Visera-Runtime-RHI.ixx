module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
export import Visera.Runtime.RHI.Resource;
export import Visera.Runtime.RHI.CommandList;
export import Visera.Runtime.RHI.Registry;
export import Visera.Runtime.RHI.SwapChain;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.Window;
       import Visera.Runtime.Global;
       import Visera.Core.OS.Thread;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Map;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.String;
       import Visera.Core.Types.JSON;
       import Visera.Core.Log;
       import Visera.Platform;
       import vulkan_hpp;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHI : public IGlobalService
    {
    public:
        /// Submit command list for execution on FRHI thread (transfers ownership).
        void
        Execute(TUniquePtr<FRHICommandList> I_CommandList);
        /// Submit command list (makes a copy; prefer Execute(TUniquePtr) when possible).
        void
        Execute(FRHICommandList& I_CommandList);

        void
        CreateSwapChain(FWindow* I_Window);
        void
        DestroySwapChain(FWindow* I_Window);
        /// Return swap chain index for I_Window; used to set FRHICommandList::TargetSwapChain.
        [[nodiscard]] TOptional<FRHISwapChainID>
        GetSwapChainIndex(FWindow* I_Window) const;
        void
        WaitIdle() const { Driver->WaitIdle(); }

        // Resource creation
        [[nodiscard]] FRHITextureID
        CreateTexture(FRHITextureCreateInfo&& I_Desc);
        [[nodiscard]] FRHIBufferID
        CreateBuffer(FRHIBufferCreateInfo&& I_Desc);
        [[nodiscard]] FRHISamplerID
        CreateSampler(FRHISamplerCreateInfo&& I_Desc);
        [[nodiscard]] FRHIDescriptorSetID
        CreateDescriptorSet(FRHIDescriptorSetCreateInfo&& I_Desc);

        // Low-level API
        [[nodiscard]] inline FVulkanDriver*
        GetDriver()
        {
            DEBUG_ONLY_FIELD(LOG_WARN("Accessed the RHI driver."));
            return Driver;
        }

    private:
        struct VISERA_RUNTIME_API FRHIThread
        {
            FRHI*   Owner { nullptr };
            FThread Thread;
            FEvent  WakeEvent;

            TSPSCQueue<TUniquePtr<FRHICommandList>> CommandListQueue;

            void Start(FRHI* I_Owner);
            void Run();
            void Stop();
            void Enqueue(TUniquePtr<FRHICommandList> I_CommandList);
        };

        FRHIThread     RHIThread;

        FVulkanDriver* Driver   {nullptr};
        FRHIRegistry*  Registry {nullptr};
        Bool           bOffScreenMode {False};

        FVulkanGraphicsCommandPool GraphicsCommandPool;
        FVulkanTransferCommandPool TransferCommandPool;

        static constexpr UInt8        PrimarySwapChainIndex {0};
        TArray<FRHISwapChain>         SwapChains;
        TMap<FWindow*, UInt8>         WindowToSwapChainIndex;

    public:
        FRHI(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
        {
            Dependencies =
            {

            };

            bOffScreenMode = !I_Registry->Contains(EName::Window);
            
            if(!bOffScreenMode)
            {
                 Dependencies.Insert(EName::Window);
            }

            if (!OnBootstrap.TryBind([this]
            {
                TSharedPtr<FWindow> Window;
                if (!bOffScreenMode)
                {
                    if (auto WindowWeak = GetService<FWindow>(EName::Window); !WindowWeak.IsExpired())
                    {
                        Window = WindowWeak.Lock();
                    }
                    else
                    {
                        LOG_FATAL("Failed to get Window service!");
                        return False;
                    }

                }
                auto ExpectedGPU    = GetConfig().GetString(TJSONPath<"RHI.GPU", 2>::Get(), "");
                auto bVSync = GetConfig().GetBool(TJSONPath<"RHI.VSync", 2>::Get(), True);
                vk::PresentModeKHR PresentMode = bVSync?
                    vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
                UInt32 AppVersion = vk::makeVersion(
                    GetConfig().GetNumber(TJSONPath<"RHI.Version[0]", 3>::Get(), 1),
                    GetConfig().GetNumber(TJSONPath<"RHI.Version[1]", 3>::Get(), 0),
                    GetConfig().GetNumber(TJSONPath<"RHI.Version[2]", 3>::Get(), 0)
                );
                Driver = new FVulkanDriver({.Window = Window, .SwapChainPresentMode = PresentMode, .bOffScreenMode = bOffScreenMode, .ApplicationName = GetRuntimeName(), .ApplicationVersion = AppVersion, .GPUName = ExpectedGPU});

                if (Driver->GetDevice().GraphicsQueueFamilyIndex
                    !=
                    Driver->GetDevice().TransferQueueFamilyIndex)
                { LOG_WARN("({}) NOT support \"Queue Family Ownership Transfer\"!", GetRuntimeName()); }

                if (ExpectedGPU.IsEmpty())
                {
                    FString GPUName(Driver->GetGPU().Properties.deviceName.data());
                    SetConfig(TJSONPath<"RHI.GPU", 2>::Get(), GPUName);
                }

                Registry = new FRHIRegistry(Driver);

                GraphicsCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Graphics>(False);
                TransferCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Transfer>(False);

                if (bOffScreenMode)
                {
                    SwapChains.EmplaceBack();
                    SwapChains.Back().Initialize(Driver, &GraphicsCommandPool, &TransferCommandPool, nullptr);
                }
                else
                {
                    SwapChains.EmplaceBack();
                    SwapChains.Back().Initialize(Driver, &GraphicsCommandPool, &TransferCommandPool, Window.Get());
                    SwapChains.Back().SubscribeToResize(Driver, &GraphicsCommandPool, &TransferCommandPool, Window.Get(), GetRuntimeName());
                    WindowToSwapChainIndex.Insert(Window.Get(), PrimarySwapChainIndex);
                }

                RHIThread.Start(this);

                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                RHIThread.Stop();

                Driver->WaitIdle();
                for (auto& Ctx : SwapChains)
                { Ctx.UnsubscribeFromResize(); }
                SwapChains.Clear();
                WindowToSwapChainIndex.Clear();
                GraphicsCommandPool = {};
                TransferCommandPool = {};
                delete Registry;
                delete Driver;
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    private:
        template<typename T>
        [[nodiscard]] static const T&
        DecodePayload(const FRHICommandView& I_Cmd)
        {
            VISERA_ASSERT(I_Cmd.PayloadPtrAligned != nullptr);
            VISERA_ASSERT(I_Cmd.PayloadBytes == sizeof(T));
            return *reinterpret_cast<const T*>(I_Cmd.PayloadPtrAligned);
        }

        [[nodiscard]] FVulkanImage*
        GetVulkanImageChecked(FRHITextureHandle I_Handle) const
        {
            auto* Tex = Registry->Get(I_Handle);
            VISERA_ASSERT(Tex);
            return Tex->GetVulkanImage();
        }

        [[nodiscard]] FVulkanBuffer*
        GetVulkanBufferChecked(FRHIBufferHandle I_Handle) const
        {
            auto* Buf = Registry->Get(I_Handle);
            VISERA_ASSERT(Buf);
            return Buf->GetVulkanBuffer();
        }

        void ExecuteImmediate(FRHICommandList& I_CommandList);
        void ExecuteConvertImageLayout(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
        void ExecuteClearColorImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
        void ExecuteBlitImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
        void ExecuteBlitToSwapChain(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd, FWindow* I_TargetWindow);
        void ExecuteCopyBufferToImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
        void ExecuteWriteBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);

        // Map a single vk::ImageLayout to the stage/access for barrier src or dst.
        static void MapGraphicsLayoutToBarrier(
            vk::ImageLayout        I_Layout,
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
            vk::ImageLayout        I_Layout,
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

    void FRHI::
    CreateSwapChain(FWindow* I_Window)
    {
        if (bOffScreenMode) { return; }
        if (WindowToSwapChainIndex.Contains(I_Window)) { return; }
        Driver->CreateSwapChain(I_Window);
        SwapChains.EmplaceBack();
        SwapChains.Back().Initialize(Driver, &GraphicsCommandPool, &TransferCommandPool, I_Window);
        SwapChains.Back().SubscribeToResize(Driver, &GraphicsCommandPool, &TransferCommandPool, I_Window, GetRuntimeName());
        WindowToSwapChainIndex.Insert(I_Window, static_cast<UInt8>(SwapChains.GetSize() - 1));
    }

    void FRHI::
    DestroySwapChain(FWindow* I_Window)
    {
        if (bOffScreenMode) { return; }
        if (auto It = WindowToSwapChainIndex.Find(I_Window); It != WindowToSwapChainIndex.end())
        {
            UInt8 Idx = It->second;
            SwapChains[Idx].UnsubscribeFromResize();
            // Swap-and-pop to keep indices compact; update map for swapped window
            if (Idx != SwapChains.GetSize() - 1)
            {
                SwapChains[Idx] = std::move(SwapChains.Back());
                if (FWindow* Moved = SwapChains[Idx].Window)
                { WindowToSwapChainIndex[Moved] = Idx; }
            }
            SwapChains.PopBack();
            WindowToSwapChainIndex.Erase(I_Window);
        }
        Driver->DestroySwapChain(I_Window);
    }

    TOptional<FRHISwapChainID> FRHI::
    GetSwapChainIndex(FWindow* I_Window) const
    {
        if (auto It = WindowToSwapChainIndex.Find(I_Window); It != WindowToSwapChainIndex.end())
        { return TOptional<FRHISwapChainID>(It->second); }
        return NullOpt;
    }

    void FRHI::FRHIThread::
    Start(FRHI* I_Owner)
    {
        VISERA_ASSERT(I_Owner && !Owner);
        Owner = I_Owner;

        Thread.Start([this]
        {
            LOG_DEBUG("({}) RHI thread started.", Owner->GetRuntimeName());
            FPlatform::SetCurrentThreadName("RHI");
            try         { Run(); }
            catch (...) { LOG_FATAL("RHI thread crashed by exception."); }
        });
    }

    void FRHI::FRHIThread::
    Run()
    {
        while (!Thread.ShouldStop())
        {
            Bool bExecuted = False;
            do
            {
                if (auto R = CommandListQueue.Dequeue(); R.HasValue())
                {
                    bExecuted = True;
                    Owner->ExecuteImmediate(*std::move(R).GetValue());
                }
                else break;
            } while (True);

            if (!bExecuted) { WakeEvent.Wait(); }
        }

        if (Owner) { Owner->WaitIdle(); }
    }

    void FRHI::FRHIThread::
    Stop()
    {
        Thread.RequestStop();
        WakeEvent.Trigger();
        Thread.Join();
        auto RuntimeName = Owner ? Owner->GetRuntimeName() : FString("Unknown");
        Owner = nullptr;
        LOG_DEBUG("({}) RHI thread stopped.", RuntimeName);
    }

    void FRHI::FRHIThread::
    Enqueue(TUniquePtr<FRHICommandList> I_CommandList)
    {
        CommandListQueue.Enqueue(std::move(I_CommandList));
        WakeEvent.Trigger();
    }

    void FRHI::
    Execute(TUniquePtr<FRHICommandList> I_CommandList)
    {
        RHIThread.Enqueue(std::move(I_CommandList));
    }

    void FRHI::
    Execute(FRHICommandList& I_CommandList)
    {
        Execute(MakeUnique<FRHICommandList>(I_CommandList));
    }

    void FRHI::
    ExecuteImmediate(FRHICommandList& I_CommandList)
    {
        FRHISwapChainID Idx = I_CommandList.TargetSwapChain;
        if (Idx >= SwapChains.GetSize()) { return; }
        auto* Ctx = &SwapChains[Idx];
        FRHIInFlightFrame& Frame = Ctx->InFlightFrames[Ctx->FrameIndex];
        VISERA_ASSERT(Frame.DrawCalls.IsRecording());
        VISERA_ASSERT(Frame.TransferCalls.IsRecording());

        for (auto Command : I_CommandList)
        {
            if (Command.PayloadPtrAligned == nullptr) { continue; }

            switch (Command.Type)
            {
            case ECommandType::ConvertImageLayout:  ExecuteConvertImageLayout(Frame, Command); break;
            case ECommandType::ClearColorImage:     ExecuteClearColorImage(Frame, Command); break;
            case ECommandType::BlitImage:           ExecuteBlitImage(Frame, Command); break;
            case ECommandType::BlitToSwapChain:     ExecuteBlitToSwapChain(Frame, Command, Ctx->Window); break;
            case ECommandType::CopyBufferToImage:   ExecuteCopyBufferToImage(Frame, Command); break;
            case ECommandType::WriteBuffer:         ExecuteWriteBuffer(Frame, Command); break;
            default: LOG_ERROR("({}) Unknown Command Type: {}", GetRuntimeName(), static_cast<UInt16>(Command.Type)); break;
            }
        }
    }

    void FRHI::
    ExecuteConvertImageLayout(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FConvertImageLayout>(I_Cmd);
        auto* Img = GetVulkanImageChecked(Payload.Image);
        vk::ImageLayout OldLayout = Img->GetLayout();
        vk::ImageLayout NewLayout = TypeCast(Payload.NewLayout);
        EVulkanGraphicsStage  SrcStage{},  DstStage{};
        EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
        InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
        I_Frame.DrawCalls.ConvertImageLayout(Img, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
    }

    void FRHI::
    ExecuteClearColorImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FClearColorImage>(I_Cmd);
        auto* Img = GetVulkanImageChecked(Payload.Image);
        I_Frame.DrawCalls.ClearColorImage(Img, {
            Payload.ClearColor.R,
            Payload.ClearColor.G,
            Payload.ClearColor.B,
            Payload.ClearColor.A,
        });
    }

    void FRHI::
    ExecuteBlitImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FBlitImage>(I_Cmd);
        auto* SrcImg = GetVulkanImageChecked(Payload.SrcImage);
        auto* DstImg = GetVulkanImageChecked(Payload.DstImage);
        I_Frame.DrawCalls.BlitImage(SrcImg, DstImg, TypeCast(Payload.Filter));
    }

    void FRHI::
    ExecuteBlitToSwapChain(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd, FWindow* I_TargetWindow)
    {
        if (bOffScreenMode || !I_TargetWindow) { return; }
        auto* SC = Driver->GetSwapChain(I_TargetWindow);
        if (!SC) { return; }
        const auto& Payload = DecodePayload<FRHICommandList::FBlitToSwapChain>(I_Cmd);
        auto* Texture        = GetVulkanImageChecked(Payload.Image);
        auto* SwapChainImage = SC->GetCurrentImage();
        {
            vk::ImageLayout OldLayout = SwapChainImage->GetLayout();
            vk::ImageLayout NewLayout = TypeCast(ERHIImageLayout::TransferDst);
            EVulkanGraphicsStage  SrcStage{},  DstStage{};
            EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
            InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.DrawCalls.ConvertImageLayout(SwapChainImage, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
        }
        I_Frame.DrawCalls.BlitImage(Texture, SwapChainImage, TypeCast(Payload.Filter));
        {
            vk::ImageLayout OldLayout = SwapChainImage->GetLayout();
            vk::ImageLayout NewLayout = TypeCast(ERHIImageLayout::Present);
            EVulkanGraphicsStage  SrcStage{},  DstStage{};
            EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
            InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.DrawCalls.ConvertImageLayout(SwapChainImage, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
        }
    }

    void FRHI::
    ExecuteCopyBufferToImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FCopyBufferToImage>(I_Cmd);
        auto* VulkanBuffer = GetVulkanBufferChecked(Payload.Buffer);
        auto* VulkanImage  = GetVulkanImageChecked(Payload.Image);
        vk::ImageLayout OldLayout   = VulkanImage->GetLayout();
        vk::ImageLayout TransferDst = TypeCast(ERHIImageLayout::TransferDst);
        {
            EVulkanTransferStage  SrcStage{}, DstStage{};
            EVulkanTransferAccess SrcAccess{}, DstAccess{};
            InferTransferBarrier(OldLayout, TransferDst, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.TransferCalls.ConvertImageLayout(VulkanImage, TransferDst, SrcStage, SrcAccess, DstStage, DstAccess);
        }
        I_Frame.TransferCalls.CopyBufferToImage(VulkanBuffer, VulkanImage);
        {
            EVulkanTransferStage  SrcStage{}, DstStage{};
            EVulkanTransferAccess SrcAccess{}, DstAccess{};
            InferTransferBarrier(TransferDst, OldLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.TransferCalls.ConvertImageLayout(VulkanImage, OldLayout, SrcStage, SrcAccess, DstStage, DstAccess);
        }
    }

    void FRHI::
    ExecuteWriteBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FWriteBuffer>(I_Cmd);
        auto* TargetBuffer  = GetVulkanBufferChecked(Payload.TargetBuffer);
        auto* StagingBuffer = GetVulkanBufferChecked(Payload.StagingBuffer);
        I_Frame.TransferCalls.CopyBuffer(StagingBuffer, TargetBuffer);
    }

    FRHITextureID FRHI::
    CreateTexture(FRHITextureCreateInfo&& I_Desc)
    {
        const auto W = I_Desc.Width, H = I_Desc.Height, D = I_Desc.Depth;
        const auto Fmt = I_Desc.Format;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateTexture: {}x{}x{} {} -> {}", GetRuntimeName(), W, H, D, Fmt, ID.GetHandle());
        return ID;
    }

    FRHIBufferID FRHI::
    CreateBuffer(FRHIBufferCreateInfo&& I_Desc)
    {
        const auto Size = I_Desc.Size;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateBuffer: {} bytes -> {}", GetRuntimeName(), Size, ID.GetHandle());
        return ID;
    }

    FRHISamplerID FRHI::
    CreateSampler(FRHISamplerCreateInfo&& I_Desc)
    {
        const auto Type = I_Desc.Type;
        const auto Addr = I_Desc.AddressMode;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateSampler: {} {} -> {}", GetRuntimeName(), Type, Addr, ID.GetHandle());
        return ID;
    }

    FRHIDescriptorSetID FRHI::
    CreateDescriptorSet(FRHIDescriptorSetCreateInfo&& I_Desc)
    {
        const auto BindingCount = I_Desc.Bindings.GetSize();
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateDescriptorSet: {} bindings -> {}",
                  GetRuntimeName(), BindingCount, ID.GetHandle());
        return ID;
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
