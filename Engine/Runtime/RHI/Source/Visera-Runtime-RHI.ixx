module;
#include <Visera-RHI.hpp>
#include <atomic>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
export import Visera.Runtime.RHI.Attachments;
export import Visera.Runtime.RHI.Resource;
export import Visera.Runtime.RHI.CommandList;
export import Visera.Runtime.RHI.Registry;
export import Visera.Runtime.RHI.SwapChain;
       import Visera.Runtime.RHI.Barrier;
       import Visera.Runtime.RHI.StagingRing;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.Window;
       import Visera.Runtime.Global;
       import Visera.Core.OS.Thread;
       import Visera.Core.OS.Memory;
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
        [[nodiscard]] FRHICommandList
        CreateCommandList();

        void
        Submit(FRHICommandList&& I_CommandList);
        void
        Submit(const FRHICommandList& I_CommandList);

        [[nodiscard]] FRHITextureID
        AcquireSwapChainTexture(FRHISwapChainID I_SwapChainID = 0);
        void
        Present(FRHISwapChainID I_SwapChainID = 0);

        void
        CreateSwapChain(TSharedPtr<FWindow> I_Window);
        void
        DestroySwapChain(TSharedPtr<FWindow> I_Window);
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

        // Descriptor writes
        void
        WriteDescriptorCombinedImageSampler(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                                            const FRHITextureID& I_Texture, const FRHISamplerID& I_Sampler,
                                            ERHIImageLayout I_ImageLayout = ERHIImageLayout::ShaderReadOnly);
        void
        WriteDescriptorUniformBuffer(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                                     const FRHIBufferID& I_Buffer);
        void
        WriteDescriptorStorageBuffer(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                                     const FRHIBufferID& I_Buffer);
        void
        WriteDescriptorStorageImage(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                                    const FRHITextureID& I_Texture,
                                    ERHIImageLayout I_ImageLayout = ERHIImageLayout::General);
        void
        WriteDescriptorSampledImage(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                                    const FRHITextureID& I_Texture,
                                    ERHIImageLayout I_ImageLayout = ERHIImageLayout::ShaderReadOnly);
        void
        WriteDescriptorSampler(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                               const FRHISamplerID& I_Sampler);

        // Texture upload via staging ring
        void
        UploadTexture(const FRHITextureID& I_Texture, const FByte* I_Data, UInt64 I_Size);
        void
        UploadBuffer(const FRHIBufferID& I_Buffer, const FByte* I_Data, UInt64 I_Size, UInt64 I_Offset = 0);

        /** Synchronously transition texture from I_OldLayout to I_NewLayout. */
        void
        TransitionTexture(const FRHITextureID& I_Texture, ERHIImageLayout I_OldLayout, ERHIImageLayout I_NewLayout);

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
            using FImmediateTask = TUniqueFunction<void(), 32>;

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

            TSPSCQueue<FRHICommandList>  CommandListQueue;
            TSPSCQueue<FRHICommandList>  FreeCommandListQueue;
            UInt64                       CommandListHighWaterMark { 64_KB };
            FImmediateCommandQueue       ImmediateCommandQueue;
            std::atomic<UInt8>           ActiveSwapChainIndex {0};
            FEvent                       InitEvent;
            FEvent                       ShutdownEvent;
            Bool                         bInitSuccess{False};
            Bool                         bShutdownSuccess{False};
            TSharedPtr<FWindow>          PrimaryWindow;

            Bool                                bOffScreenMode {False};
            TUniquePtr<FVulkanDriver>           Driver;
            TUniquePtr<FRHIRegistry>            Registry;
            TUniquePtr<FRHIStagingRingBuffer>   StagingRing;
            FVulkanGraphicsCommandPool          GraphicsCommandPool;
            FVulkanTransferCommandPool          TransferCommandPool;
            static constexpr UInt8              PrimarySwapChainIndex {0};
            TArray<FRHISwapChain>               SwapChains;
            TMap<FWindow*, UInt8>               WindowToSwapChainIndex;
            TMap<FWindow*, TSharedPtr<FWindow>> WindowRefs;

            PROFILING_ONLY_FIELD(
            FRHICommandList::FProfilingMetrics CommandListProfilingMetrics {};
            )

            void
            Start(FRHI* I_Owner);
            void
            Run();
            void
            Stop();
            void
            Enqueue(FRHICommandList I_CommandList);
            void
            Enqueue(FImmediateTask I_Task);
            void
            CreateSwapChain(TSharedPtr<FWindow> I_Window);
            void
            DestroySwapChain(TSharedPtr<FWindow> I_Window);
            void
            RecreateSwapChain(TSharedPtr<FWindow> I_Window, UInt32 I_Width, UInt32 I_Height);
            void
            Initialize();
            Bool WaitInitialize();
            void
            Shutdown();
            Bool WaitShutdown();
            void
            ExecuteInitialize();
            void
            ExecuteShutdown();
            void
            ExecuteCreateSwapChain(TSharedPtr<FWindow> I_Window);
            void
            ExecuteDestroySwapChain(TSharedPtr<FWindow> I_Window);
            void
            ExecuteRecreateSwapChain(TSharedPtr<FWindow> I_Window, UInt32 I_Width, UInt32 I_Height);
            void
            ExecuteImmediate(FRHICommandList& I_CommandList);
            Bool BeginFrame(FRHISwapChainID I_SwapChainID);
            void
            PresentSwapChain(FRHISwapChainID I_SwapChainID, FEvent* I_Done);
            [[nodiscard]] TOptional<FRHISwapChainID>
            GetSwapChainIndex(FWindow* I_Window) const;
            void
            WaitIdle() const;
            [[nodiscard]] FRHITextureID
            CreateTexture(FRHITextureCreateInfo&& I_Desc);
            [[nodiscard]] FRHIBufferID
            CreateBuffer(FRHIBufferCreateInfo&& I_Desc);
            [[nodiscard]] FRHISamplerID
            CreateSampler(FRHISamplerCreateInfo&& I_Desc);
            [[nodiscard]] FRHIDescriptorSetID
            CreateDescriptorSet(FRHIDescriptorSetCreateInfo&& I_Desc);
            [[nodiscard]] FRHIShaderID
            CreateShader(FRHIShaderCreateInfo&& I_Desc);
            [[nodiscard]] FRHIRenderPassID
            CreateRenderPass(FRHIRenderPassCreateInfo&& I_Desc);
            void
            UploadTexture(const FRHITextureID& I_Texture, const FByte* I_Data, UInt64 I_Size);
            void
            UploadBuffer(const FRHIBufferID& I_Buffer, const FByte* I_Data, UInt64 I_Size, UInt64 I_Offset);
            void
            TransitionTexture(const FRHITextureID& I_Texture, ERHIImageLayout I_OldLayout, ERHIImageLayout I_NewLayout);
            [[nodiscard]] FVulkanDriver*
            GetDriver() const { return Driver.Get(); }
            void
            ExecuteConvertImageLayout(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteClearColorImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteBlitImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteCopyBufferToImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteWriteBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteEnterRenderPass(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteLeaveRenderPass(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteSetViewport(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteSetScissor(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteBindVertexBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteBindDescriptorSet(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteDraw(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteDrawIndexed(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            template<typename T>
            [[nodiscard]] static const T&
            DecodePayload(const FRHICommandView& I_Cmd);
            [[nodiscard]] FVulkanImage*
            GetVulkanImageChecked(FRHITextureHandle I_Handle) const;
            [[nodiscard]] FVulkanImageView*
            GetVulkanImageViewChecked(FRHITextureHandle I_Handle) const;
            [[nodiscard]] FVulkanBuffer*
            GetVulkanBufferChecked(FRHIBufferHandle I_Handle) const;
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
                RHIThread.CommandListHighWaterMark = GetConfig().GetNumber<UInt64>(
                    TJSONRoute<"RHI.CommandListHighWaterMark">(), 64_KB);
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
                if (auto R = CommandListQueue.Dequeue(); R.HasValue())
                {
                    bExecuted = True;
                    auto CmdList = std::move(R).GetValue();
                    ExecuteImmediate(CmdList);

                    PROFILING_ONLY_FIELD(
                    const auto& m = CmdList.GetProfilingMetrics();
                    auto& agg = CommandListProfilingMetrics;
                    if (m.PeakCommandCount > agg.PeakCommandCount) agg.PeakCommandCount = m.PeakCommandCount;
                    if (m.PeakBufferSizeBytes > agg.PeakBufferSizeBytes) agg.PeakBufferSizeBytes = m.PeakBufferSizeBytes;
                    if (m.PeakBufferCapacityBytes > agg.PeakBufferCapacityBytes) agg.PeakBufferCapacityBytes = m.PeakBufferCapacityBytes;
                    if (m.PeakCommandBytes > agg.PeakCommandBytes)
                    {
                        agg.PeakCommandBytes = m.PeakCommandBytes;
                        agg.PeakCommandType = m.PeakCommandType;
                    }
                    );

                    CmdList.Reset();
                    CmdList.ShrinkTo(CommandListHighWaterMark);
                    FreeCommandListQueue.Enqueue(std::move(CmdList));
                }
                else break;
            } while (True);

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
    Enqueue(FRHICommandList I_CommandList)
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
        StagingRing = MakeUnique<FRHIStagingRingBuffer>(Driver.Get(), 16_MB);
        GraphicsCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Graphics>(False);
        TransferCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Transfer>(False);

        if (bOffScreenMode)
        {
            SwapChains.EmplaceBack();
            SwapChains.Back().Initialize(Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, nullptr);
            SwapChains.Back().CachedProxyTextureID = FRHITextureID::CreateUnmanaged(
                FRHITextureHandle::CreateSwapChainProxy(0));
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
            Driver->WaitIdle();
            for (auto& Ctx : SwapChains) { Ctx.UnsubscribeFromResize(); }
            SwapChains.Clear();
        }

        GraphicsCommandPool = {};
        TransferCommandPool = {};

        Registry.Reset();
        StagingRing.Reset();

        PROFILING_ONLY_FIELD(
        const auto& pm = CommandListProfilingMetrics;
        LOG_INFO("[Profiling] RHI.CommandList summary: peak_command_count={} peak_buffer_size={} bytes "
                 "peak_buffer_capacity={} bytes peak_command_bytes={} (type={}).",
            pm.PeakCommandCount, pm.PeakBufferSizeBytes, pm.PeakBufferCapacityBytes,
            pm.PeakCommandBytes, static_cast<UInt16>(pm.PeakCommandType));
        );

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
        UInt8 SCIdx = static_cast<UInt8>(SwapChains.GetSize() - 1);
        SwapChains.Back().Initialize(Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, I_Window);
        SwapChains.Back().CachedProxyTextureID = FRHITextureID::CreateUnmanaged(
            FRHITextureHandle::CreateSwapChainProxy(SCIdx));
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
        Driver->WaitIdle();
        auto* Ptr = I_Window.Get();
        if (auto It = WindowToSwapChainIndex.Find(Ptr); It != WindowToSwapChainIndex.end())
        {
            UInt8 Idx = It->second;
            SwapChains[Idx].UnsubscribeFromResize();
            if (Idx != SwapChains.GetSize() - 1)
            {
                SwapChains[Idx] = std::move(SwapChains.Back());
                if (auto Moved = SwapChains[Idx].Window.Lock())
                { WindowToSwapChainIndex[Moved.Get()] = Idx; }
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

        auto& OldCtx = SwapChains[Idx];
        if (OldCtx.bFrameActive)
        {
            auto& Frame = OldCtx.InFlightFrames[OldCtx.FrameIndex];
            Frame.GraphicsCalls.End();
            Frame.TransferCalls.End();
            OldCtx.bFrameActive = False;
        }

        OldCtx.UnsubscribeFromResize();
        Driver->WaitIdle();
        Driver->RecreateSwapChain(Ptr, I_Width, I_Height);

        FRHISwapChain NewCtx{};
        NewCtx.Initialize(Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, I_Window);
        NewCtx.CachedProxyTextureID = FRHITextureID::CreateUnmanaged(
            FRHITextureHandle::CreateSwapChainProxy(Idx));
        NewCtx.SubscribeToResize(
            Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, Ptr,
            [this, WindowShared = I_Window](FWindow* I_ResizeWindow, UInt32 I_NewWidth, UInt32 I_NewHeight)
            { RecreateSwapChain(WindowShared, I_NewWidth, I_NewHeight); });
        SwapChains[Idx] = std::move(NewCtx);
    }

    Bool FRHI::FRHIThread::
    BeginFrame(FRHISwapChainID I_SwapChainID)
    {
        if (I_SwapChainID >= SwapChains.GetSize()) { return False; }
        auto& Ctx   = SwapChains[I_SwapChainID];
        auto& Frame = Ctx.InFlightFrames[Ctx.FrameIndex];

        if (Frame.ExecuteFence.Wait())
        {
            (void)Frame.ExecuteFence.Reset();
        }
        else LOG_ERROR("Failed to wait the fence of swapchain (id:{})", I_SwapChainID);

        if (auto Win = Ctx.Window.Lock(); Win)
        {
            if (!Driver->WaitNextFrame(Win.Get(), &Frame.SwapChainReadySemaphore))
            { return False; }
        }

        Frame.GraphicsCalls.Reset();
        Frame.GraphicsCalls.Begin();
        Frame.TransferCalls.Reset();
        Frame.TransferCalls.Begin();
        Ctx.bFrameActive = True;
        return True;
    }

    void FRHI::FRHIThread::
    PresentSwapChain(FRHISwapChainID I_SwapChainID, FEvent* I_Done)
    {
        if (I_SwapChainID >= SwapChains.GetSize())
        {
            if (I_Done) { I_Done->Trigger(); }
            return;
        }
        auto& Ctx = SwapChains[I_SwapChainID];
        if (!Ctx.bFrameActive)
        {
            if (I_Done) { I_Done->Trigger(); }
            return;
        }

        auto& Frame = Ctx.InFlightFrames[Ctx.FrameIndex];
        Frame.TransferCalls.End();
        Frame.GraphicsCalls.End();

        if (auto Win = Ctx.Window.Lock(); !bOffScreenMode && Win)
        {
            auto* SC = Driver->GetSwapChain(Win.Get());
            const UInt8 ImageIndex = SC ? static_cast<UInt8>(SC->Cursor) : 0U;

            Driver->Submit(&Frame.GraphicsCalls,
                           &Frame.SwapChainReadySemaphore,
                           &Ctx.RenderFinishedSemaphores[ImageIndex],
                           &Frame.ExecuteFence);
            if (!Driver->Present(Win.Get(), &Ctx.RenderFinishedSemaphores[ImageIndex]))
            { LOG_ERROR("Failed to present the swapchain (id:{})", I_SwapChainID); }
        }

        Ctx.bFrameActive = False;
        Ctx.FrameIndex = (Ctx.FrameIndex + 1) % static_cast<UInt8>(Ctx.InFlightFrames.GetSize());

        if (I_Done) { I_Done->Trigger(); }
    }

    FRHICommandList FRHI::
    CreateCommandList()
    {
        if (auto R = RHIThread.FreeCommandListQueue.Dequeue(); R.HasValue())
        { return std::move(R).GetValue(); }
        return FRHICommandList{};
    }

    void FRHI::
    Submit(FRHICommandList&& I_CommandList)
    {
        RHIThread.Enqueue(std::move(I_CommandList));
    }

    void FRHI::
    Submit(const FRHICommandList& I_CommandList)
    {
        FRHICommandList Copy(I_CommandList);
        RHIThread.Enqueue(std::move(Copy));
    }

    FRHITextureID FRHI::
    AcquireSwapChainTexture(FRHISwapChainID I_SwapChainID)
    {
        RHIThread.ActiveSwapChainIndex.store(I_SwapChainID, std::memory_order_release);
        return RHIThread.SwapChains[I_SwapChainID].CachedProxyTextureID;
    }

    void FRHI::
    Present(FRHISwapChainID I_SwapChainID)
    {
        FEvent Done;
        RHIThread.Enqueue([this, I_SwapChainID, &Done]()
        {
            RHIThread.PresentSwapChain(I_SwapChainID, &Done);
        });
        Done.Wait();
    }

    void FRHI::FRHIThread::
    ExecuteImmediate(FRHICommandList& I_CommandList)
    {
        UInt8 Idx = ActiveSwapChainIndex.load(std::memory_order_acquire);
        if (Idx >= SwapChains.GetSize()) { return; }
        auto* Ctx = &SwapChains[Idx];

        if (!Ctx->bFrameActive)
        {
            if (!BeginFrame(Idx)) { return; }
        }

        FRHIInFlightFrame& Frame = Ctx->InFlightFrames[Ctx->FrameIndex];
        VISERA_ASSERT(Frame.GraphicsCalls.IsRecording());
        VISERA_ASSERT(Frame.TransferCalls.IsRecording());

        for (auto Command : I_CommandList)
        {
            if (Command.PayloadPtrAligned == nullptr) { continue; }

            switch (Command.Type)
            {
            case ERHICommandType::TransitionTexture:   ExecuteConvertImageLayout(Frame, Command); break;
            case ERHICommandType::ClearColorImage:     ExecuteClearColorImage(Frame, Command); break;
            case ERHICommandType::BlitImage:           ExecuteBlitImage(Frame, Command); break;
            case ERHICommandType::CopyBufferToImage:   ExecuteCopyBufferToImage(Frame, Command); break;
            case ERHICommandType::WriteBuffer:         ExecuteWriteBuffer(Frame, Command); break;
            case ERHICommandType::EnterRenderPass:     ExecuteEnterRenderPass(Frame, Command); break;
            case ERHICommandType::LeaveRenderPass:     ExecuteLeaveRenderPass(Frame, Command); break;
            case ERHICommandType::SetViewport:         ExecuteSetViewport(Frame, Command); break;
            case ERHICommandType::SetScissor:          ExecuteSetScissor(Frame, Command); break;
            case ERHICommandType::BindVertexBuffer:    ExecuteBindVertexBuffer(Frame, Command); break;
            case ERHICommandType::BindDescriptorSet:   ExecuteBindDescriptorSet(Frame, Command); break;
            case ERHICommandType::Draw:               ExecuteDraw(Frame, Command); break;
            case ERHICommandType::DrawIndexed:        ExecuteDrawIndexed(Frame, Command); break;
            default: LOG_ERROR("({}) Unknown Command Type: {}", Owner ? Owner->GetRuntimeName() : FString("Unknown"), static_cast<UInt16>(Command.Type)); break;
            }
        }
    }

    void FRHI::FRHIThread::
    ExecuteConvertImageLayout(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FTransitionTexturePayload>(I_Cmd);
        auto* Img = GetVulkanImageChecked(Payload.Image);
        vk::ImageLayout OldLayout = TypeCast(Payload.OldLayout);
        vk::ImageLayout NewLayout = TypeCast(Payload.NewLayout);
        EVulkanGraphicsStage  SrcStage{},  DstStage{};
        EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
        FRHIImageBarrier::InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
        I_Frame.GraphicsCalls.ConvertImageLayout(Img, OldLayout, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
    }

    void FRHI::FRHIThread::
    ExecuteClearColorImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FClearColorImage>(I_Cmd);
        auto* Img = GetVulkanImageChecked(Payload.Image);
        I_Frame.GraphicsCalls.ClearColorImage(Img, {
            Payload.ClearColor.R,
            Payload.ClearColor.G,
            Payload.ClearColor.B,
            Payload.ClearColor.A,
        }, TypeCast(Payload.ImageLayout));
    }

    void FRHI::FRHIThread::
    ExecuteBlitImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FBlitImage>(I_Cmd);
        auto* SrcImg = GetVulkanImageChecked(Payload.SrcImage);
        auto* DstImg = GetVulkanImageChecked(Payload.DstImage);
        I_Frame.GraphicsCalls.BlitImage(SrcImg, DstImg, TypeCast(Payload.Filter),
            TypeCast(Payload.SrcImageLayout), TypeCast(Payload.DstImageLayout));
    }

    void FRHI::FRHIThread::
    ExecuteCopyBufferToImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FCopyBufferToImage>(I_Cmd);
        auto* VulkanBuffer = GetVulkanBufferChecked(Payload.Buffer);
        auto* VulkanImage  = GetVulkanImageChecked(Payload.Image);
        vk::ImageLayout InitialLayout = TypeCast(Payload.InitialLayout);
        vk::ImageLayout TransferDst  = TypeCast(ERHIImageLayout::TransferDst);
        vk::ImageLayout FinalLayout  = TypeCast(Payload.FinalLayout);
        {
            EVulkanTransferStage  SrcStage{}, DstStage{};
            EVulkanTransferAccess SrcAccess{}, DstAccess{};
            FRHIImageBarrier::InferTransferBarrier(InitialLayout, TransferDst, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.TransferCalls.ConvertImageLayout(VulkanImage, InitialLayout, TransferDst, SrcStage, SrcAccess, DstStage, DstAccess);
        }
        I_Frame.TransferCalls.CopyBufferToImage(VulkanBuffer, VulkanImage, TransferDst);
        {
            EVulkanTransferStage  SrcStage{}, DstStage{};
            EVulkanTransferAccess SrcAccess{}, DstAccess{};
            FRHIImageBarrier::InferTransferBarrier(TransferDst, FinalLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            I_Frame.TransferCalls.ConvertImageLayout(VulkanImage, TransferDst, FinalLayout, SrcStage, SrcAccess, DstStage, DstAccess);
        }
    }

    void FRHI::FRHIThread::
    ExecuteWriteBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FWriteBuffer>(I_Cmd);
        auto* TargetBuffer  = GetVulkanBufferChecked(Payload.TargetBuffer);
        auto* StagingBufferfer = GetVulkanBufferChecked(Payload.StagingBuffer);
        I_Frame.TransferCalls.CopyBuffer(StagingBufferfer, TargetBuffer);
    }

    void FRHI::FRHIThread::
    ExecuteEnterRenderPass(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FEnterRenderPass>(I_Cmd);
        auto* RenderPass = Registry->Get(Payload.RenderPass);
        if (!RenderPass) { return; }
        auto* Pipeline = RenderPass->GetVulkanRenderPipeline();
        if (Payload.ColorTargetCount > 0)
        {
            FVulkanRenderTarget ColorRTs[kMaxColorAttachments];
            FVulkanRenderTarget* ColorRTPtrs[kMaxColorAttachments];
            UInt32 ValidCount = 0;
            for (UInt32 i = 0; i < Payload.ColorTargetCount; ++i)
            {
                const auto& Slot = Payload.ColorSlots[i];
                if (Slot.Handle == FRHITextureHandle{}) { continue; }
                auto* ImageView = GetVulkanImageViewChecked(Slot.Handle);
                if (!ImageView) { continue; }
                ColorRTs[ValidCount] = FVulkanRenderTarget(ImageView);
                ColorRTs[ValidCount].SetLoadOp(TypeCast(Slot.LoadOp));
                ColorRTs[ValidCount].SetStoreOp(TypeCast(Slot.StoreOp));
                ColorRTs[ValidCount].SetClearColor(vk::ClearColorValue(
                    Slot.ClearR, Slot.ClearG, Slot.ClearB, Slot.ClearA));
                ColorRTPtrs[ValidCount] = &ColorRTs[ValidCount];
                ++ValidCount;
            }
            if (ValidCount > 0)
            {
                Pipeline->SetColorRTs(ColorRTPtrs, ValidCount);
                auto* Image = ColorRTs[0].GetImage();
                if (Image)
                {
                    const auto Ext = Image->GetExtent();
                    Pipeline->SetRenderArea(vk::Rect2D{}
                        .setOffset({0, 0})
                        .setExtent({Ext.width, Ext.height}));
                }
            }
        }
        I_Frame.GraphicsCalls.EnterRenderPipeline(Pipeline);
    }

    void FRHI::FRHIThread::
    ExecuteLeaveRenderPass(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FLeaveRenderPass>(I_Cmd);
        I_Frame.GraphicsCalls.LeaveRenderPipeline();
    }

    void FRHI::FRHIThread::
    ExecuteSetViewport(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FSetViewport>(I_Cmd);
        I_Frame.GraphicsCalls.SetViewport(TypeCast(Payload.Viewport));
    }

    void FRHI::FRHIThread::
    ExecuteSetScissor(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FSetScissor>(I_Cmd);
        I_Frame.GraphicsCalls.SetScissor(TypeCast(Payload.Scissor));
    }

    void FRHI::FRHIThread::
    ExecuteBindVertexBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FBindVertexBuffer>(I_Cmd);
        auto* VulkanBuffer = GetVulkanBufferChecked(Payload.Buffer);
        I_Frame.GraphicsCalls.BindVertexBuffer(Payload.Binding, VulkanBuffer, Payload.Offset);
    }

    void FRHI::FRHIThread::
    ExecuteBindDescriptorSet(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FBindDescriptorSet>(I_Cmd);
        auto* DescriptorSet = Registry->Get(Payload.DescriptorSet);
        if (!DescriptorSet) { return; }
        I_Frame.GraphicsCalls.BindDescriptorSet(Payload.SetIndex, DescriptorSet->GetVulkanDescriptorSet());
    }

    void FRHI::FRHIThread::
    ExecuteDraw(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FDraw>(I_Cmd);
        I_Frame.GraphicsCalls.Draw(Payload.VertexCount, Payload.InstanceCount, Payload.FirstVertex, Payload.FirstInstance);
    }

    void FRHI::FRHIThread::
    ExecuteDrawIndexed(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FDrawIndexed>(I_Cmd);
        I_Frame.GraphicsCalls.DrawIndexed(Payload.IndexCount, Payload.InstanceCount, Payload.FirstIndex, Payload.VertexOffset, Payload.FirstInstance);
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
        if (I_Handle.IsSwapChainProxy())
        {
            UInt8 SCIdx = static_cast<UInt8>(I_Handle.GetIndex());
            if (SCIdx >= SwapChains.GetSize()) { return nullptr; }
            if (auto WindowWeak = SwapChains[SCIdx].Window; !WindowWeak.IsExpired())
            {
                auto Window = WindowWeak.Lock();
                auto* SC = Driver->GetSwapChain(Window.Get());
                return SC ? SC->GetCurrentImage() : nullptr;
            }
            else return nullptr;
        }
        auto* Tex = Registry->Get(I_Handle);
        VISERA_ASSERT(Tex);
        return Tex->GetVulkanImage();
    }

    FVulkanImageView* FRHI::FRHIThread::GetVulkanImageViewChecked(FRHITextureHandle I_Handle) const
    {
        if (I_Handle.IsSwapChainProxy())
        {
            UInt8 SCIdx = static_cast<UInt8>(I_Handle.GetIndex());
            if (SCIdx >= SwapChains.GetSize()) { return nullptr; }
            if (auto WindowWeak = SwapChains[SCIdx].Window; !WindowWeak.IsExpired())
            {
                auto Window = WindowWeak.Lock();
                auto* SC = Driver->GetSwapChain(Window.Get());
                return SC ? SC->GetCurrentImageView() : nullptr;
            }
            else return nullptr;
        }
        auto* Tex = Registry->Get(I_Handle);
        VISERA_ASSERT(Tex);
        return Tex->GetVulkanImageView();
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

    // --- Descriptor Write APIs ---

    void FRHI::
    WriteDescriptorCombinedImageSampler(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                                        const FRHITextureID& I_Texture, const FRHISamplerID& I_Sampler,
                                        ERHIImageLayout I_ImageLayout)
    {
        auto* DS  = RHIThread.Registry->Get(I_DS.GetHandle());
        auto* Tex = RHIThread.Registry->Get(I_Texture.GetHandle());
        auto* Smp = RHIThread.Registry->Get(I_Sampler.GetHandle());
        if (!DS || !Tex || !Smp) { LOG_ERROR("WriteDescriptorCombinedImageSampler: invalid handle."); return; }
        DS->GetVulkanDescriptorSet()->WriteCombinedImageSampler(
            I_Binding, Tex->GetVulkanImageView(), Smp->GetVulkanSampler(), TypeCast(I_ImageLayout));
    }

    void FRHI::
    WriteDescriptorUniformBuffer(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                                 const FRHIBufferID& I_Buffer)
    {
        auto* DS  = RHIThread.Registry->Get(I_DS.GetHandle());
        auto* Buf = RHIThread.Registry->Get(I_Buffer.GetHandle());
        if (!DS || !Buf) { LOG_ERROR("WriteDescriptorUniformBuffer: invalid handle."); return; }
        DS->GetVulkanDescriptorSet()->WriteUniformBuffer(I_Binding, Buf->GetVulkanBuffer());
    }

    void FRHI::
    WriteDescriptorStorageBuffer(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                                 const FRHIBufferID& I_Buffer)
    {
        auto* DS  = RHIThread.Registry->Get(I_DS.GetHandle());
        auto* Buf = RHIThread.Registry->Get(I_Buffer.GetHandle());
        if (!DS || !Buf) { LOG_ERROR("WriteDescriptorStorageBuffer: invalid handle."); return; }
        DS->GetVulkanDescriptorSet()->WriteStorageBuffer(I_Binding, Buf->GetVulkanBuffer());
    }

    void FRHI::
    WriteDescriptorStorageImage(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                                const FRHITextureID& I_Texture,
                                ERHIImageLayout I_ImageLayout)
    {
        auto* DS  = RHIThread.Registry->Get(I_DS.GetHandle());
        auto* Tex = RHIThread.Registry->Get(I_Texture.GetHandle());
        if (!DS || !Tex) { LOG_ERROR("WriteDescriptorStorageImage: invalid handle."); return; }
        DS->GetVulkanDescriptorSet()->WriteStorageImage(I_Binding, Tex->GetVulkanImageView(), TypeCast(I_ImageLayout));
    }

    void FRHI::
    WriteDescriptorSampledImage(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                                const FRHITextureID& I_Texture,
                                ERHIImageLayout I_ImageLayout)
    {
        auto* DS  = RHIThread.Registry->Get(I_DS.GetHandle());
        auto* Tex = RHIThread.Registry->Get(I_Texture.GetHandle());
        if (!DS || !Tex) { LOG_ERROR("WriteDescriptorSampledImage: invalid handle."); return; }
        DS->GetVulkanDescriptorSet()->WriteSampledImage(I_Binding, Tex->GetVulkanImageView(), TypeCast(I_ImageLayout));
    }

    void FRHI::
    WriteDescriptorSampler(const FRHIDescriptorSetID& I_DS, UInt32 I_Binding,
                           const FRHISamplerID& I_Sampler)
    {
        auto* DS  = RHIThread.Registry->Get(I_DS.GetHandle());
        auto* Smp = RHIThread.Registry->Get(I_Sampler.GetHandle());
        if (!DS || !Smp) { LOG_ERROR("WriteDescriptorSampler: invalid handle."); return; }
        DS->GetVulkanDescriptorSet()->WriteSampler(I_Binding, Smp->GetVulkanSampler());
    }

    // --- Texture Upload via Staging Ring ---

    void FRHI::
    UploadTexture(const FRHITextureID& I_Texture, const FByte* I_Data, UInt64 I_Size)
    {
        RHIThread.UploadTexture(I_Texture, I_Data, I_Size);
    }

    void FRHI::
    UploadBuffer(const FRHIBufferID& I_Buffer, const FByte* I_Data, UInt64 I_Size, UInt64 I_Offset)
    {
        RHIThread.UploadBuffer(I_Buffer, I_Data, I_Size, I_Offset);
    }

    void FRHI::
    TransitionTexture(const FRHITextureID& I_Texture, ERHIImageLayout I_OldLayout, ERHIImageLayout I_NewLayout)
    {
        RHIThread.TransitionTexture(I_Texture, I_OldLayout, I_NewLayout);
    }

    void FRHI::FRHIThread::
    UploadTexture(const FRHITextureID& I_Texture, const FByte* I_Data, UInt64 I_Size)
    {
        VISERA_ASSERT(I_Data && I_Size > 0);

        auto* Tex = Registry->Get(I_Texture.GetHandle());
        if (!Tex) { LOG_ERROR("UploadTexture: invalid texture handle."); return; }

        auto Alloc = StagingRing->Allocate(I_Size);
        if (!Alloc.IsValid())
        { LOG_ERROR("UploadTexture: staging ring allocation failed for {} bytes.", I_Size); return; }
        StagingRing->Write(Alloc, I_Data, I_Size);

        auto* VulkanImage   = Tex->GetVulkanImage();
        auto* StagingBuffer = StagingRing->GetVulkanBuffer();
        vk::ImageLayout DstLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        auto Cmd = TransferCommandPool.CreateCommandBuffer(True);
        Cmd.Begin();
        {
            EVulkanTransferStage  SrcStage{}, DstStage{};
            EVulkanTransferAccess SrcAccess{}, DstAccess{};
            FRHIImageBarrier::InferTransferBarrier(
                vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            Cmd.ConvertImageLayout(VulkanImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                SrcStage, SrcAccess, DstStage, DstAccess);
        }
        Cmd.CopyBufferToImage(StagingBuffer, VulkanImage, Alloc.Offset, vk::ImageLayout::eTransferDstOptimal);
        {
            EVulkanTransferStage  SrcStage{}, DstStage{};
            EVulkanTransferAccess SrcAccess{}, DstAccess{};
            FRHIImageBarrier::InferTransferBarrier(
                vk::ImageLayout::eTransferDstOptimal, DstLayout,
                &SrcStage, &SrcAccess, &DstStage, &DstAccess);
            Cmd.ConvertImageLayout(VulkanImage, vk::ImageLayout::eTransferDstOptimal, DstLayout,
                SrcStage, SrcAccess, DstStage, DstAccess);
        }
        Cmd.End();

        FVulkanFence Fence = Driver->CreateFence(False);
        Driver->Submit(&Cmd, nullptr, nullptr, &Fence);
        // Fence wait is required: (1) StagingRing correctness - AdvanceFence must run only after the GPU
        // finishes reading from this staging region, else the next allocation could overwrite in-flight data;
        // (2) API contract - this is a synchronous API, caller expects data ready on return.
        // For async uploads, use CommandList (WriteBuffer, CopyBufferToImage) instead.
        if (!Fence.Wait())
        { LOG_FATAL("UploadTexture: fence wait failed!"); }

        StagingRing->AdvanceFence(Alloc.Offset + Alloc.Size);
    }

    void FRHI::FRHIThread::
    TransitionTexture(const FRHITextureID& I_Texture, ERHIImageLayout I_OldLayout, ERHIImageLayout I_NewLayout)
    {
        auto* Tex = Registry->Get(I_Texture.GetHandle());
        if (!Tex) { LOG_ERROR("TransitionTexture: invalid texture handle."); return; }

        auto* VulkanImage = Tex->GetVulkanImage();
        vk::ImageLayout OldLayout = TypeCast(I_OldLayout);
        vk::ImageLayout NewLayout = TypeCast(I_NewLayout);
        if (OldLayout == NewLayout) { return; }

        EVulkanGraphicsStage  SrcStage{}, DstStage{};
        EVulkanGraphicsAccess SrcAccess{}, DstAccess{};
        FRHIImageBarrier::InferGraphicsBarrier(OldLayout, NewLayout, &SrcStage, &SrcAccess, &DstStage, &DstAccess);

        auto Cmd = GraphicsCommandPool.CreateCommandBuffer(True);
        Cmd.Begin();
        Cmd.ConvertImageLayout(VulkanImage, OldLayout, NewLayout, SrcStage, SrcAccess, DstStage, DstAccess);
        Cmd.End();

        FVulkanFence Fence = Driver->CreateFence(False);
        Driver->Submit(&Cmd, nullptr, nullptr, &Fence);
        if (!Fence.Wait())
        { LOG_FATAL("TransitionTexture: fence wait failed!"); }
    }

    void FRHI::FRHIThread::
    UploadBuffer(const FRHIBufferID& I_Buffer, const FByte* I_Data, UInt64 I_Size, UInt64 I_Offset)
    {
        VISERA_ASSERT(I_Data && I_Size > 0);

        auto* Buf = Registry->Get(I_Buffer.GetHandle());
        if (!Buf) { LOG_ERROR("UploadBuffer: invalid buffer handle."); return; }

        auto Alloc = StagingRing->Allocate(I_Size);
        if (!Alloc.IsValid())
        { LOG_ERROR("UploadBuffer: staging ring allocation failed for {} bytes.", I_Size); return; }
        StagingRing->Write(Alloc, I_Data, I_Size);

        auto* TargetBuffer  = Buf->GetVulkanBuffer();
        auto* StagingBuffer = StagingRing->GetVulkanBuffer();

        auto Cmd = TransferCommandPool.CreateCommandBuffer(True);
        Cmd.Begin();
        Cmd.CopyBuffer(StagingBuffer, TargetBuffer, Alloc.Offset, I_Offset, I_Size);
        Cmd.End();

        FVulkanFence Fence = Driver->CreateFence(False);
        Driver->Submit(&Cmd, nullptr, nullptr, &Fence);
        // Fence wait is required: (1) StagingRing correctness - AdvanceFence must run only after the GPU
        // finishes reading from this staging region, else the next allocation could overwrite in-flight data;
        // (2) API contract - this is a synchronous API, caller expects data ready on return.
        // For async uploads, use CommandList (WriteBuffer, CopyBufferToImage) instead.
        if (!Fence.Wait())
        { LOG_FATAL("UploadBuffer: fence wait failed!"); }

        StagingRing->AdvanceFence(Alloc.Offset + Alloc.Size);
    }
}