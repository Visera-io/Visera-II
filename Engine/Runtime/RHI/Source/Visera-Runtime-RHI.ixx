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
       import Visera.Core.Types.Function;
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
        /// I_TargetSwapChain: index into swap chain array; 0 = primary window.
        void
        Execute(TUniquePtr<FRHICommandList> I_CommandList, FRHISwapChainID I_TargetSwapChain = 0);

        void
        CreateSwapChain(TSharedPtr<FWindow> I_Window);
        void
        DestroySwapChain(TSharedPtr<FWindow> I_Window);
        /// Return swap chain index for I_Window; pass to Execute() for multi-window rendering.
        [[nodiscard]] TOptional<FRHISwapChainID>
        GetSwapChainIndex(FWindow* I_Window) const { return RHIThread.GetSwapChainIndex(I_Window); }
        void
        WaitIdle() const { RHIThread.WaitIdle(); }

        // Resource creation
        [[nodiscard]] FRHITextureID
        CreateTexture(FRHITextureCreateInfo&& I_Desc) { return RHIThread.CreateTexture(std::move(I_Desc)); }
        [[nodiscard]] FRHIBufferID
        CreateBuffer(FRHIBufferCreateInfo&& I_Desc) { return RHIThread.CreateBuffer(std::move(I_Desc)); }
        [[nodiscard]] FRHISamplerID
        CreateSampler(FRHISamplerCreateInfo&& I_Desc) { return RHIThread.CreateSampler(std::move(I_Desc)); }
        [[nodiscard]] FRHIDescriptorSetID
        CreateDescriptorSet(FRHIDescriptorSetCreateInfo&& I_Desc) { return RHIThread.CreateDescriptorSet(std::move(I_Desc)); }
        [[nodiscard]] FRHIShaderID
        CreateShader(FRHIShaderCreateInfo&& I_Desc) { return RHIThread.CreateShader(std::move(I_Desc)); }
        [[nodiscard]] FRHIRenderPassID
        CreateRenderPass(FRHIRenderPassCreateInfo&& I_Desc) { return RHIThread.CreateRenderPass(std::move(I_Desc)); }

        // Low-level API
        [[nodiscard]] inline FVulkanDriver*
        GetDriver()
        {
            DEBUG_ONLY_FIELD(LOG_WARN("Accessed the RHI driver."));
            return RHIThread.GetDriver();
        }

    private:
        struct VISERA_RUNTIME_API FRHIThread
        {
            using FImmediateTask = TUniqueFunction<void()>;

            struct FImmediateCommandQueue
            {
                TSPSCQueue<FImmediateTask> Queue;
                void
                Enqueue(FImmediateTask I_Task) { Queue.Enqueue(std::move(I_Task)); }
                [[nodiscard]] TOptional<FImmediateTask>
                Dequeue() { return Queue.Dequeue(); }
            };

            FRHI*   Owner { nullptr };
            FThread Thread;
            FEvent  WakeEvent;

            TSPSCQueue<TUniquePtr<FRHICommandList>> CommandListQueue;
            FImmediateCommandQueue                  ImmediateCommandQueue;
            FEvent                                  InitEvent;
            FEvent                                  ShutdownEvent;
            Bool                                    bInitSuccess{False};
            Bool                                    bShutdownSuccess{False};
            TSharedPtr<FWindow>                    PrimaryWindow;

            Bool                                    bOffScreenMode {False};
            TUniquePtr<FVulkanDriver>               Driver;
            TUniquePtr<FRHIRegistry>                Registry;
            FVulkanGraphicsCommandPool GraphicsCommandPool;
            FVulkanTransferCommandPool TransferCommandPool;
            static constexpr UInt8      PrimarySwapChainIndex {0};
            TArray<FRHISwapChain>              SwapChains;
            TMap<FWindow*, UInt8>              WindowToSwapChainIndex;
            TMap<FWindow*, TSharedPtr<FWindow>> WindowRefs;

            void Start(FRHI* I_Owner);
            void Run();
            void Stop();
            void Enqueue(TUniquePtr<FRHICommandList> I_CommandList);
            void Enqueue(FImmediateTask I_Task);
            void CreateSwapChain(TSharedPtr<FWindow> I_Window);
            void DestroySwapChain(TSharedPtr<FWindow> I_Window);
            void RecreateSwapChain(TSharedPtr<FWindow> I_Window, UInt32 I_Width, UInt32 I_Height);
            void Initialize();
            Bool WaitInitialize();
            void Shutdown();
            Bool WaitShutdown();
            void ExecuteInitialize();
            void ExecuteShutdown();
            void ExecuteCreateSwapChain(TSharedPtr<FWindow> I_Window);
            void ExecuteDestroySwapChain(TSharedPtr<FWindow> I_Window);
            void ExecuteRecreateSwapChain(TSharedPtr<FWindow> I_Window, UInt32 I_Width, UInt32 I_Height);
            void ExecuteImmediate(FRHICommandList& I_CommandList);
            [[nodiscard]] TOptional<FRHISwapChainID> GetSwapChainIndex(FWindow* I_Window) const;
            void WaitIdle() const;
            [[nodiscard]] FRHITextureID CreateTexture(FRHITextureCreateInfo&& I_Desc);
            [[nodiscard]] FRHIBufferID CreateBuffer(FRHIBufferCreateInfo&& I_Desc);
            [[nodiscard]] FRHISamplerID CreateSampler(FRHISamplerCreateInfo&& I_Desc);
            [[nodiscard]] FRHIDescriptorSetID CreateDescriptorSet(FRHIDescriptorSetCreateInfo&& I_Desc);
            [[nodiscard]] FRHIShaderID CreateShader(FRHIShaderCreateInfo&& I_Desc);
            [[nodiscard]] FRHIRenderPassID CreateRenderPass(FRHIRenderPassCreateInfo&& I_Desc);
            [[nodiscard]] FVulkanDriver* GetDriver() const { return Driver.Get(); }
            void ExecuteConvertImageLayout(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void ExecuteClearColorImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void ExecuteBlitImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void ExecuteBlitToSwapChain(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd, FWindow* I_TargetWindow);
            void ExecuteCopyBufferToImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void ExecuteWriteBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void ExecuteEnterRenderPass(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd, FWindow* I_TargetWindow);
            void ExecuteLeaveRenderPass(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd, FWindow* I_TargetWindow);
            void ExecuteSetViewport(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void ExecuteSetScissor(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void ExecuteBindVertexBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void ExecuteBindDescriptorSet(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void ExecuteDraw(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void ExecuteDrawIndexed(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            template<typename T>
            [[nodiscard]] static const T& DecodePayload(const FRHICommandView& I_Cmd);
            [[nodiscard]] FVulkanImage* GetVulkanImageChecked(FRHITextureHandle I_Handle) const;
            [[nodiscard]] FVulkanBuffer* GetVulkanBufferChecked(FRHIBufferHandle I_Handle) const;
            static void MapGraphicsLayoutToBarrier(vk::ImageLayout I_Layout, EVulkanGraphicsStage* IO_Stage, EVulkanGraphicsAccess* IO_Access);
            void InferGraphicsBarrier(vk::ImageLayout I_OldLayout, vk::ImageLayout I_NewLayout, EVulkanGraphicsStage* IO_SrcStage, EVulkanGraphicsAccess* IO_SrcAccess, EVulkanGraphicsStage* IO_DstStage, EVulkanGraphicsAccess* IO_DstAccess) const;
            static void MapTransferLayoutToBarrier(vk::ImageLayout I_Layout, EVulkanTransferStage* IO_Stage, EVulkanTransferAccess* IO_Access);
            void InferTransferBarrier(vk::ImageLayout I_OldLayout, vk::ImageLayout I_NewLayout, EVulkanTransferStage* IO_SrcStage, EVulkanTransferAccess* IO_SrcAccess, EVulkanTransferStage* IO_DstStage, EVulkanTransferAccess* IO_DstAccess) const;
        };

        FRHIThread     RHIThread;

    public:
        FRHI(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
        {
            Dependencies =
            {

            };

            RHIThread.bOffScreenMode = !I_Registry->Contains(EName::Window);
            
            if(!RHIThread.bOffScreenMode)
            {
                 Dependencies.Insert(EName::Window);
            }

            if (!OnBootstrap.TryBind([this]
            {
                TSharedPtr<FWindow> Window;
                if (!RHIThread.bOffScreenMode)
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
                auto ExpectedGPU    = GetConfig().GetString(TJSONRoute<"RHI.GPU">(), "");
                auto bVSync = GetConfig().GetBool(TJSONRoute<"RHI.VSync">(), True);
                vk::PresentModeKHR PresentMode = bVSync?
                    vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
                UInt32 AppVersion = vk::makeVersion(1, 0, 0);
                RHIThread.Driver = MakeUnique<FVulkanDriver>(FVulkanDriverCreateInfo{
                    .Window = Window,
                    .SwapChainPresentMode = PresentMode,
                    .bOffScreenMode = RHIThread.bOffScreenMode,
                    .ApplicationName = GetRuntimeName(),
                    .ApplicationVersion = AppVersion,
                    .GPUName = ExpectedGPU
                });
                RHIThread.Driver->CreateInstance();
                RHIThread.Driver->CreateDebugMessenger();
                if (!RHIThread.bOffScreenMode)
                {
                    // Stage 1 on main thread: create window surface first.
                    RHIThread.Driver->CreateSwapChain(Window.Get());
                    RHIThread.PrimaryWindow = Window;
                }

                RHIThread.Start(this);
                RHIThread.Initialize();
                if (!RHIThread.WaitInitialize())
                {
                    LOG_FATAL("Failed to initialize Vulkan driver on RHI thread!");
                    return False;
                }

                if (RHIThread.Driver->GetDevice().GraphicsQueueFamilyIndex
                    !=
                    RHIThread.Driver->GetDevice().TransferQueueFamilyIndex)
                { LOG_WARN("({}) NOT support \"Queue Family Ownership Transfer\"!", GetRuntimeName()); }

                if (ExpectedGPU.IsEmpty())
                {
                    FString GPUName(RHIThread.Driver->GetGPU().Properties.deviceName.data());
                    SetConfig(TJSONRoute<"RHI.GPU">(), GPUName);
                }

                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                RHIThread.Shutdown();
                if (!RHIThread.WaitShutdown())
                {
                    LOG_FATAL("Failed to shutdown Vulkan driver on RHI thread!");
                }

                RHIThread.Stop();

                if (RHIThread.Driver)
                {
                    // Stage 2 on main thread: instance-level teardown.
                    RHIThread.Driver->DestroyDebugMessenger();
                    RHIThread.Driver->DestroyInstance();
                    RHIThread.Driver.Reset();
                }
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };

    void FRHI::
    CreateSwapChain(TSharedPtr<FWindow> I_Window)
    {
        RHIThread.CreateSwapChain(std::move(I_Window));
    }

    void FRHI::
    DestroySwapChain(TSharedPtr<FWindow> I_Window)
    {
        RHIThread.DestroySwapChain(std::move(I_Window));
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
                if (auto T = ImmediateCommandQueue.Dequeue(); T.HasValue())
                {
                    bExecuted = True;
                    auto Task = std::move(T).GetValue();
                    if (Task) { Task(); }
                }
                else break;
            } while (True);

            do
            {
                if (auto R = CommandListQueue.Dequeue(); R.HasValue())
                {
                    bExecuted = True;
                    ExecuteImmediate(*std::move(R).GetValue());
                }
                else break;
            } while (True);

            if (!bExecuted) { WakeEvent.Wait(); }
        }

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

    void FRHI::FRHIThread::
    Enqueue(FImmediateTask I_Task)
    {
        if (!I_Task) { return; }
        ImmediateCommandQueue.Enqueue(std::move(I_Task));
        WakeEvent.Trigger();
    }

    void FRHI::FRHIThread::
    CreateSwapChain(TSharedPtr<FWindow> I_Window)
    {
        if (!I_Window) { return; }
        Enqueue([this, Window = std::move(I_Window)]() { ExecuteCreateSwapChain(std::move(Window)); });
    }

    void FRHI::FRHIThread::
    Initialize()
    {
        bInitSuccess = False;
        Enqueue([this]() { ExecuteInitialize(); });
    }

    Bool FRHI::FRHIThread::
    WaitInitialize()
    {
        InitEvent.Wait();
        return bInitSuccess;
    }

    void FRHI::FRHIThread::
    Shutdown()
    {
        bShutdownSuccess = False;
        Enqueue([this]() { ExecuteShutdown(); });
    }

    Bool FRHI::FRHIThread::
    WaitShutdown()
    {
        ShutdownEvent.Wait();
        return bShutdownSuccess;
    }

    void FRHI::FRHIThread::
    DestroySwapChain(TSharedPtr<FWindow> I_Window)
    {
        if (!I_Window) { return; }
        Enqueue([this, Window = std::move(I_Window)]() { ExecuteDestroySwapChain(std::move(Window)); });
    }

    void FRHI::FRHIThread::
    RecreateSwapChain(TSharedPtr<FWindow> I_Window, UInt32 I_Width, UInt32 I_Height)
    {
        if (!I_Window || I_Width == 0 || I_Height == 0) { return; }
        Enqueue([this, Window = std::move(I_Window), I_Width, I_Height]() { ExecuteRecreateSwapChain(std::move(Window), I_Width, I_Height); });
    }

    void FRHI::FRHIThread::
    ExecuteInitialize()
    {
        bInitSuccess = False;
        if (!Driver)
        {
            InitEvent.Trigger();
            return;
        }

        Driver->CreateDevice();
        Driver->CreateAllocator();
        Driver->CreatePipelineCache();

        Registry = MakeUnique<FRHIRegistry>(Driver.Get());
        GraphicsCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Graphics>(False);
        TransferCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Transfer>(False);

        if (bOffScreenMode)
        {
            SwapChains.EmplaceBack();
            SwapChains.Back().Initialize(Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, nullptr);
        }
        else if (PrimaryWindow)
        {
            ExecuteCreateSwapChain(PrimaryWindow);
        }

        bInitSuccess = True;
        InitEvent.Trigger();
    }

    void FRHI::FRHIThread::
    ExecuteShutdown()
    {
        bShutdownSuccess = False;

        if (!Driver)
        {
            bShutdownSuccess = True;
            ShutdownEvent.Trigger();
            return;
        }

        // Tear down swap chains while device is still alive.
        while (!WindowRefs.IsEmpty())
        {
            auto It = WindowRefs.begin();
            ExecuteDestroySwapChain(It->second);
        }
        if (bOffScreenMode)
        {
            for (auto& Ctx : SwapChains) { Ctx.UnsubscribeFromResize(); }
            SwapChains.Clear();
        }

        GraphicsCommandPool = {};
        TransferCommandPool = {};

        Registry.Reset();

        Driver->DestroyPipelineCache();
        Driver->DestroyAllocator();
        Driver->DestroyDevice();

        PrimaryWindow.Reset();
        bShutdownSuccess = True;
        ShutdownEvent.Trigger();
    }

    void FRHI::FRHIThread::
    ExecuteCreateSwapChain(TSharedPtr<FWindow> I_Window)
    {
        if (bOffScreenMode || !I_Window || !Driver) { return; }
        auto* Ptr = I_Window.Get();
        if (WindowToSwapChainIndex.Contains(Ptr)) { return; }
        Driver->CreateSwapChain(Ptr);
        SwapChains.EmplaceBack();
        SwapChains.Back().Initialize(Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, Ptr);
        SwapChains.Back().SubscribeToResize(
            Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, Ptr,
            [this, WindowShared = I_Window](FWindow* I_ResizeWindow, UInt32 I_Width, UInt32 I_Height)
            { RecreateSwapChain(WindowShared, I_Width, I_Height); });
        WindowToSwapChainIndex.Insert(Ptr, static_cast<UInt8>(SwapChains.GetSize() - 1));
        WindowRefs.Insert(Ptr, std::move(I_Window));
    }

    void FRHI::FRHIThread::
    ExecuteDestroySwapChain(TSharedPtr<FWindow> I_Window)
    {
        if (bOffScreenMode || !I_Window || !Driver) { return; }
        auto* Ptr = I_Window.Get();
        if (auto It = WindowToSwapChainIndex.Find(Ptr); It != WindowToSwapChainIndex.end())
        {
            UInt8 Idx = It->second;
            SwapChains[Idx].UnsubscribeFromResize();
            if (Idx != SwapChains.GetSize() - 1)
            {
                SwapChains[Idx] = std::move(SwapChains.Back());
                if (FWindow* Moved = SwapChains[Idx].Window)
                { WindowToSwapChainIndex[Moved] = Idx; }
            }
            SwapChains.PopBack();
            WindowToSwapChainIndex.Erase(Ptr);
            WindowRefs.Erase(Ptr);
        }
        Driver->DestroySwapChain(Ptr);
    }

    void FRHI::FRHIThread::
    ExecuteRecreateSwapChain(TSharedPtr<FWindow> I_Window, UInt32 I_Width, UInt32 I_Height)
    {
        if (bOffScreenMode || !I_Window || !Driver) { return; }
        if (I_Width == 0 || I_Height == 0) { return; }
        auto* Ptr = I_Window.Get();
        auto It = WindowToSwapChainIndex.Find(Ptr);
        if (It == WindowToSwapChainIndex.end()) { return; }

        const UInt8 Idx = It->second;
        LOG_DEBUG("({}) Recreating SwapChain ({}x{}) for window (title:{}).",
            Owner ? Owner->GetRuntimeName() : FString("Unknown"),
            I_Width, I_Height, I_Window->GetTitle());

        SwapChains[Idx].UnsubscribeFromResize();
        Driver->WaitIdle();
        Driver->RecreateSwapChain(Ptr, I_Width, I_Height);

        FRHISwapChain NewCtx{};
        NewCtx.Initialize(Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, Ptr);
        NewCtx.SubscribeToResize(
            Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, Ptr,
            [this, WindowShared = I_Window](FWindow* I_ResizeWindow, UInt32 I_NewWidth, UInt32 I_NewHeight)
            { RecreateSwapChain(WindowShared, I_NewWidth, I_NewHeight); });
        SwapChains[Idx] = std::move(NewCtx);
    }

    void FRHI::
    Execute(TUniquePtr<FRHICommandList> I_CommandList, FRHISwapChainID I_TargetSwapChain)
    {
        if (I_CommandList) { I_CommandList->TargetSwapChain = I_TargetSwapChain; }
        RHIThread.Enqueue(std::move(I_CommandList));
    }

    void FRHI::FRHIThread::
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
            case ECommandType::EnterRenderPass:     ExecuteEnterRenderPass(Frame, Command, Ctx->Window); break;
            case ECommandType::LeaveRenderPass:     ExecuteLeaveRenderPass(Frame, Command, Ctx->Window); break;
            case ECommandType::SetViewport:         ExecuteSetViewport(Frame, Command); break;
            case ECommandType::SetScissor:          ExecuteSetScissor(Frame, Command); break;
            case ECommandType::BindVertexBuffer:    ExecuteBindVertexBuffer(Frame, Command); break;
            case ECommandType::BindDescriptorSet:   ExecuteBindDescriptorSet(Frame, Command); break;
            case ECommandType::Draw:               ExecuteDraw(Frame, Command); break;
            case ECommandType::DrawIndexed:        ExecuteDrawIndexed(Frame, Command); break;
            default: LOG_ERROR("({}) Unknown Command Type: {}", Owner ? Owner->GetRuntimeName() : FString("Unknown"), static_cast<UInt16>(Command.Type)); break;
            }
        }
    }

    void FRHI::FRHIThread::
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

    void FRHI::FRHIThread::
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

    void FRHI::FRHIThread::
    ExecuteBlitImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FBlitImage>(I_Cmd);
        auto* SrcImg = GetVulkanImageChecked(Payload.SrcImage);
        auto* DstImg = GetVulkanImageChecked(Payload.DstImage);
        I_Frame.DrawCalls.BlitImage(SrcImg, DstImg, TypeCast(Payload.Filter));
    }

    void FRHI::FRHIThread::
    ExecuteBlitToSwapChain(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd, FWindow* I_TargetWindow)
    {
        if (bOffScreenMode || !I_TargetWindow || !Driver) { return; }
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

    void FRHI::FRHIThread::
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

    void FRHI::FRHIThread::
    ExecuteWriteBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FWriteBuffer>(I_Cmd);
        auto* TargetBuffer  = GetVulkanBufferChecked(Payload.TargetBuffer);
        auto* StagingBuffer = GetVulkanBufferChecked(Payload.StagingBuffer);
        I_Frame.TransferCalls.CopyBuffer(StagingBuffer, TargetBuffer);
    }

    void FRHI::FRHIThread::
    ExecuteEnterRenderPass(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd, FWindow* I_TargetWindow)
    {
        if (bOffScreenMode || !I_TargetWindow || !Driver) { return; }
        auto* SC = Driver->GetSwapChain(I_TargetWindow);
        if (!SC || SC->Images.IsEmpty()) { return; }

        const auto& Payload = DecodePayload<FRHICommandList::FEnterRenderPass>(I_Cmd);
        auto* RenderPass = Registry->Get(Payload.RenderPass);
        if (!RenderPass) { return; }

        auto* Pipeline = RenderPass->GetVulkanRenderPipeline();
        auto* SwapChainImage = SC->GetCurrentImage();
        auto* ImageView = SC->GetCurrentImageView();

        // Transition swap chain image to ColorAttachmentOptimal
        {
            vk::ImageLayout OldLayout = SwapChainImage->GetLayout();
            vk::ImageLayout NewLayout = vk::ImageLayout::eColorAttachmentOptimal;
            EVulkanGraphicsStage  SrcStage{},  DstStage{};
            EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
            InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.DrawCalls.ConvertImageLayout(SwapChainImage, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
        }

        FVulkanRenderTarget ColorRT(ImageView);
        Pipeline->SetColorRT(&ColorRT);
        Pipeline->SetRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, {SC->Extent.width, SC->Extent.height}});
        I_Frame.DrawCalls.EnterRenderPipeline(Pipeline);
    }

    void FRHI::FRHIThread::
    ExecuteLeaveRenderPass(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd, FWindow* I_TargetWindow)
    {
        (void)I_Cmd;
        I_Frame.DrawCalls.LeaveRenderPipeline();
        if (bOffScreenMode || !I_TargetWindow || !Driver) { return; }
        auto* SC = Driver->GetSwapChain(I_TargetWindow);
        if (!SC || SC->Images.IsEmpty()) { return; }
        auto* SwapChainImage = SC->GetCurrentImage();
        vk::ImageLayout OldLayout = SwapChainImage->GetLayout();
        vk::ImageLayout NewLayout = vk::ImageLayout::ePresentSrcKHR;
        EVulkanGraphicsStage  SrcStage{},  DstStage{};
        EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
        InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
        I_Frame.DrawCalls.ConvertImageLayout(SwapChainImage, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
    }

    void FRHI::FRHIThread::
    ExecuteSetViewport(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FSetViewport>(I_Cmd);
        I_Frame.DrawCalls.SetViewport(TypeCast(Payload.Viewport));
    }

    void FRHI::FRHIThread::
    ExecuteSetScissor(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FSetScissor>(I_Cmd);
        I_Frame.DrawCalls.SetScissor(TypeCast(Payload.Scissor));
    }

    void FRHI::FRHIThread::
    ExecuteBindVertexBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FBindVertexBuffer>(I_Cmd);
        auto* VulkanBuffer = GetVulkanBufferChecked(Payload.Buffer);
        I_Frame.DrawCalls.BindVertexBuffer(Payload.Binding, VulkanBuffer, Payload.Offset);
    }

    void FRHI::FRHIThread::
    ExecuteBindDescriptorSet(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FBindDescriptorSet>(I_Cmd);
        auto* DescriptorSet = Registry->Get(Payload.DescriptorSet);
        if (!DescriptorSet) { return; }
        I_Frame.DrawCalls.BindDescriptorSet(Payload.SetIndex, DescriptorSet->GetVulkanDescriptorSet());
    }

    void FRHI::FRHIThread::
    ExecuteDraw(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FDraw>(I_Cmd);
        I_Frame.DrawCalls.Draw(Payload.VertexCount, Payload.InstanceCount, Payload.FirstVertex, Payload.FirstInstance);
    }

    void FRHI::FRHIThread::
    ExecuteDrawIndexed(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FDrawIndexed>(I_Cmd);
        I_Frame.DrawCalls.DrawIndexed(Payload.IndexCount, Payload.InstanceCount, Payload.FirstIndex, Payload.VertexOffset, Payload.FirstInstance);
    }

    template<typename T>
    [[nodiscard]] const T& FRHI::FRHIThread::DecodePayload(const FRHICommandView& I_Cmd)
    {
        VISERA_ASSERT(I_Cmd.PayloadPtrAligned != nullptr);
        VISERA_ASSERT(I_Cmd.PayloadBytes == sizeof(T));
        return *reinterpret_cast<const T*>(I_Cmd.PayloadPtrAligned);
    }

    FVulkanImage* FRHI::FRHIThread::GetVulkanImageChecked(FRHITextureHandle I_Handle) const
    {
        auto* Tex = Registry->Get(I_Handle);
        VISERA_ASSERT(Tex);
        return Tex->GetVulkanImage();
    }

    FVulkanBuffer* FRHI::FRHIThread::GetVulkanBufferChecked(FRHIBufferHandle I_Handle) const
    {
        auto* Buf = Registry->Get(I_Handle);
        VISERA_ASSERT(Buf);
        return Buf->GetVulkanBuffer();
    }

    TOptional<FRHISwapChainID> FRHI::FRHIThread::GetSwapChainIndex(FWindow* I_Window) const
    {
        if (auto It = WindowToSwapChainIndex.Find(I_Window); It != WindowToSwapChainIndex.end())
        { return TOptional<FRHISwapChainID>(It->second); }
        return NullOpt;
    }

    void FRHI::FRHIThread::WaitIdle() const
    {
        if (Driver) { Driver->WaitIdle(); }
    }

    FRHITextureID FRHI::FRHIThread::CreateTexture(FRHITextureCreateInfo&& I_Desc)
    {
        const auto W = I_Desc.Width, H = I_Desc.Height, D = I_Desc.Depth;
        const auto Fmt = I_Desc.Format;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateTexture: {}x{}x{} {} -> {}", Owner ? Owner->GetRuntimeName() : FString("Unknown"), W, H, D, Fmt, ID.GetHandle());
        return ID;
    }

    FRHIBufferID FRHI::FRHIThread::CreateBuffer(FRHIBufferCreateInfo&& I_Desc)
    {
        const auto Size = I_Desc.Size;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateBuffer: {} bytes -> {}", Owner ? Owner->GetRuntimeName() : FString("Unknown"), Size, ID.GetHandle());
        return ID;
    }

    FRHISamplerID FRHI::FRHIThread::CreateSampler(FRHISamplerCreateInfo&& I_Desc)
    {
        const auto Type = I_Desc.Type;
        const auto Addr = I_Desc.AddressMode;
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateSampler: {} {} -> {}", Owner ? Owner->GetRuntimeName() : FString("Unknown"), Type, Addr, ID.GetHandle());
        return ID;
    }

    FRHIDescriptorSetID FRHI::FRHIThread::CreateDescriptorSet(FRHIDescriptorSetCreateInfo&& I_Desc)
    {
        const auto BindingCount = I_Desc.Bindings.GetSize();
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateDescriptorSet: {} bindings -> {}",
                  Owner ? Owner->GetRuntimeName() : FString("Unknown"), BindingCount, ID.GetHandle());
        return ID;
    }

    FRHIShaderID FRHI::FRHIThread::CreateShader(FRHIShaderCreateInfo&& I_Desc)
    {
        auto ID = Registry->Register(std::move(I_Desc));
        LOG_DEBUG("({}) CreateShader -> {}", Owner ? Owner->GetRuntimeName() : FString("Unknown"), ID.GetHandle());
        return ID;
    }

    FRHIRenderPassID FRHI::FRHIThread::CreateRenderPass(FRHIRenderPassCreateInfo&& I_Desc)
    {
        return Registry->Register(std::move(I_Desc));
    }

    void FRHI::FRHIThread::
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

    void FRHI::FRHIThread::
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

    void FRHI::FRHIThread::
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

    void FRHI::FRHIThread::
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
