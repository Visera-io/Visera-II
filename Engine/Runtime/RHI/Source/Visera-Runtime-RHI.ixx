module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
export import Visera.Runtime.RHI.Resource;
export import Visera.Runtime.RHI.CommandList;
export import Visera.Runtime.RHI.Registry;
export import Visera.Runtime.RHI.SwapChain;
export import Visera.Runtime.RHI.Barrier;
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
    class VISERA_RUNTIME_API FRHI : public IRuntimeService
    {
    public:
        [[nodiscard]] FRHICommandList
        CreateCommandList();

        /** Set current SwapChain context; all Submit() until EndFrame() go to this SwapChain. Returns backbuffer texture for this SwapChain. */
        [[nodiscard]] FRHITextureID
        BeginFrame(FRHISwapChainID I_SwapChainID);
        void
        Submit(FRHICommandList&& I_CommandList);
        void
        Submit(const FRHICommandList& I_CommandList);
        /** Clear current SwapChain context. Call after submitting all command lists for the current frame/SwapChain. */
        void
        EndFrame();
        void
        Present(FRHISwapChainID I_SwapChainID);

        /** Create a swap chain. I_Window=nullptr creates a headless context. Blocks until RHI thread finishes allocation. Returns the new SwapChainID. */
        [[nodiscard]] FRHISwapChainID
        CreateSwapChain(FWindow* I_Window);
        /** Blocks until the RHI thread has finished destroying the swap chain. */
        void
        DestroySwapChain(FWindow* I_Window);
        void
        DestroySwapChain(FRHISwapChainID I_SwapChainID);
        void
        RecreateSwapChain(FWindow* I_Window);
        [[nodiscard]] FRHISwapChainID
        QuerySwapChainID(FWindow* I_Window) const { return RHIThread.QuerySwapChainID(I_Window); }
        [[nodiscard]] Bool
        IsSwapChainDirty(FRHISwapChainID I_SwapChainID) const;
        /** True if this swap chain should be submitted and presented (not destroyed, not minimized). */
        [[nodiscard]] Bool
        IsValidSwapChain(FRHISwapChainID I_SwapChainID) const;
        /** True if the swap chain is backed by a window (windowed); false for headless contexts. */
        [[nodiscard]] Bool
        HasWindow(FRHISwapChainID I_SwapChainID) const;
        void
        UpdateSwapChainMinimized(FRHISwapChainID I_SwapChainID, Bool I_bMinimized);
        /** Mark a swap chain as destroyed (sets bDestroyed atomic and wakes any thread blocked on backpressure). Does NOT enqueue async resource destruction. */
        void
        MarkSwapChainDestroyed(FRHISwapChainID I_SwapChainID);
        /** Number of Present tasks enqueued but not yet completed for this swap chain. Used by Graphics for BeginFrame backpressure. */
        [[nodiscard]] UInt32
        GetPendingPresentCount(FRHISwapChainID I_SwapChainID) const;
        /** FPS for the window (from that window's swap chain present timing). 0 if I_Window is null or not associated with a windowed swap chain. */
        [[nodiscard]] Float
        GetFrameRate(FWindow* I_Window) const;
        /** Max in-flight frames (kMaxInFlightFrames). Used for backpressure and swapchain InFlightFrames size. */
        [[nodiscard]] UInt32
        GetMaxInFlightFrames() const { return kMaxInFlightFrames; }
        void
        WaitDeviceIdle() const;
        void
        WaitSwapChainIdle(FWindow* I_Window) const;

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
        [[nodiscard]] FRHIComputePassID
        CreateComputePass(FRHIComputePassCreateInfo&& I_Desc)
        { return RHIThread.Registry->Register(std::move(I_Desc)); }

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

        /** Block until I_Texture (e.g. offscreen RT) is copied to I_OutData. Image region [0,0] to [I_Width, I_Height]. Does not expose Map. */
        void
        ReadbackTexture(const FRHITextureID& I_Texture, void* I_OutData, UInt64 I_Size, UInt32 I_Width, UInt32 I_Height);

        // Low-level API
        [[nodiscard]] const FVulkanDriver*
        GetDriver() const
        {
            DEBUG_ONLY_FIELD(LOG_WARN("Accessed the RHI driver."));
            return RHIThread.GetDriver();
        }

    private:
        FRHISwapChainID CurrentSwapChainID { kInvalidSwapChainID };

        struct VISERA_RUNTIME_API FRHIThread
        {
            using FImmediateTask = TUniqueFunction<void(), 64>;

            struct FImmediateCommandQueue
            {
                TMPSCQueue<FImmediateTask> Queue;
                void
                Enqueue(FImmediateTask I_Task) { Queue.Enqueue(std::move(I_Task)); }
                [[nodiscard]] TOptional<FImmediateTask>
                Dequeue() { return Queue.Dequeue(); }
            };

            FRHI*   Owner { nullptr };
            FThread Thread;
            FEvent  WakeEvent;

            struct FCommandListEntry { FRHICommandList Cmd; FRHISwapChainID SwapChainID {kInvalidSwapChainID}; };
            TSPSCQueue<FCommandListEntry> CommandListQueue;
            TSPSCQueue<FRHICommandList>   FreeCommandListQueue;
            UInt64                        CommandListHighWaterMark { kCommandListHighWaterMarkBytes };
            FImmediateCommandQueue        ImmediateCommandQueue;
            FEvent                        InitEvent;
            FEvent                        ShutdownEvent;
            mutable FEvent                IdleSyncEvent;
            Bool                          bInitSuccess{False};
            Bool                          bShutdownSuccess{False};

            FString                             PreferredGPUName {};
            TUniquePtr<FVulkanDriver>           Driver;
            TSharedPtr<FRHIRegistry>            Registry;
            TUniquePtr<FRHIStagingRingBuffer>   StagingRing;
            FVulkanGraphicsCommandPool          GraphicsCommandPool;
            FVulkanTransferCommandPool          TransferCommandPool;
            TArray<FRHISwapChain>               SwapChains;
            TArray<UInt8>                       FreeSlots;  // Reusable swap chain indices for stable IDs.
            mutable FRWLock                     WindowToSwapChainLock;
            TMap<FWindow*, FRHISwapChainID>     WindowToSwapChainIndex;
            /** Utility frame for synchronous one-off GPU work (e.g. DoReadbackTexture). */
            TOptional<FRHIInFlightFrame>        UtilityFrame;

            PROFILING_ONLY_FIELD(
            FRHICommandList::FProfilingMetrics CommandListProfilingMetrics {};
            )

            FVulkanPipelineLayout* CurrentComputePipelineLayout {nullptr};

            void
            Start(FRHI* I_Owner);
            void
            Run();
            void
            Stop();
            void
            Enqueue(FRHICommandList I_CommandList, FRHISwapChainID I_SwapChainID = kInvalidSwapChainID);
            void
            Execute(FImmediateTask I_Task);
            void
            CreateSwapChain(FWindow* I_Window);
            void
            RecreateSwapChain(FWindow* I_Window);
            Bool
            Initialize();
            Bool
            Shutdown();
            UInt8
            AllocateSlot();
            void
            FreeSlot(UInt8 I_Index);
            void
            ExecuteInitialize();
            void
            ExecuteShutdown();
            void
            ExecuteCreateSwapChain(FWindow* I_Window, void* I_PreCreatedSurface = nullptr);
            FRHISwapChainID
            ExecuteCreateHeadlessSwapChain();
            void
            ExecuteDestroySwapChain(FWindow* I_Window, FEvent* I_Done = nullptr);
            void
            ExecuteDestroySwapChain(FRHISwapChainID I_ID, FEvent* I_Done = nullptr);
            /** Thread-safe lookup; call from any thread. */
            [[nodiscard]] FRHISwapChainID
            QuerySwapChainID(FWindow* I_Window) const;
            void
            ExecuteRecreateSwapChain(FWindow* I_Window);
            void
            ExecuteImmediate(FRHICommandList& I_CommandList, FRHISwapChainID I_SwapChainID);
            Bool BeginFrame(FRHISwapChainID I_SwapChainID);
            void
            Present(FRHISwapChainID I_SwapChainID);
            /** Mark swap chain dirty and enqueue recreate task (single in-flight). */
            void
            RequestRecreateSwapChain(FRHISwapChainID I_SwapChainID);
            void
            WaitDeviceIdle() const;
            void
            DoReadbackTexture(const FRHITextureID& I_Texture, void* I_OutData, UInt64 I_Size, UInt32 I_Width, UInt32 I_Height);
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
            ExecuteTransitionTexture(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteMemoryBarrier(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteBufferBarrier(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteClearColorImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteBlitImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteCopyImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteCopyBufferToImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
            void
            ExecuteCopyImageToBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
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
            ExecutePushConstants(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd);
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
        FRHI(FString I_Name, FServiceRegistry* I_Registry, FJSONView I_ConfigView,
             TMulticastDelegate<const FJSONRoute&>* I_OnConfigChange, FStringView I_RuntimeName)
            : IRuntimeService(I_Name, I_Registry, std::move(I_ConfigView), I_OnConfigChange, I_RuntimeName)
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
                RHIThread.PreferredGPUName = GetConfig().GetString(TJSONRoute<"RHI.GPU">(), "");
                RHIThread.CommandListHighWaterMark = GetConfig().GetNumber<UInt64>(
                    TJSONRoute<"RHI.CommandListHighWaterMark">(), kCommandListHighWaterMarkBytes);
                UInt32 AppVersion = vk::makeVersion(1, 0, 0);
                RHIThread.Driver = MakeUnique<FVulkanDriver>();
                auto InstanceExtensions = FPlatformWindow::GetVulkanRequiredInstanceExtensions();
                RHIThread.Driver->CreateInstance(GetRuntimeName(), AppVersion, InstanceExtensions);
                RHIThread.Driver->CreateDebugMessenger();

                RHIThread.Start(this);
                if (!RHIThread.Initialize())
                {
                    LOG_FATAL("Failed to initialize Vulkan driver on RHI thread!");
                    return False;
                }

                if (RHIThread.Driver->GetDevice().GraphicsQueueFamilyIndex
                    !=
                    RHIThread.Driver->GetDevice().TransferQueueFamilyIndex)
                { LOG_WARN("({}) NOT support \"Queue Family Ownership Transfer\"!", GetRuntimeName()); }

                if (RHIThread.PreferredGPUName.IsEmpty())
                {
                    FString GPUName(RHIThread.Driver->GetGPU().Properties.deviceName.data());
                    SetConfig(TJSONRoute<"RHI.GPU">(), GPUName);
                }

                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                if (!RHIThread.Shutdown())
                { LOG_ERROR("({}) RHI shutdown failed.", GetRuntimeName()); }
                RHIThread.Stop();
                if (RHIThread.Driver)
                {
                    RHIThread.Driver->DestroyDebugMessenger();
                    RHIThread.Driver->DestroyInstance();
                    RHIThread.Driver.Reset();
                }
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };

    FRHISwapChainID FRHI::
    CreateSwapChain(FWindow* I_Window)
    {
        if (I_Window)
        {
            // Pre-create VkSurface on the calling thread (main thread).
            // macOS requires AppKit/CAMetalLayer operations on the main thread;
            // deferring glfwCreateWindowSurface to the RHI thread produces a
            // layer that is never composited, resulting in a blank window.
            void* PreCreatedSurface = I_Window->GetPlatformWindow()
                ->CreateVulkanSurface(*RHIThread.Driver->GetInstance());
            FRHISwapChainID ResultID = kInvalidSwapChainID;
            FEvent Done;
            RHIThread.Execute([this, I_Window, PreCreatedSurface, &ResultID, &Done]()
            {
                RHIThread.ExecuteCreateSwapChain(I_Window, PreCreatedSurface);
                ResultID = RHIThread.QuerySwapChainID(I_Window);
                Done.Trigger();
            });
            Done.Wait();
            if (ResultID != kInvalidSwapChainID)
            {
                I_Window->OnWindowClosing.Subscribe([this](FWindow* W) { MarkSwapChainDestroyed(QuerySwapChainID(W)); });
            }
            return ResultID;
        }
        else
        {
            FRHISwapChainID ResultID = kInvalidSwapChainID;
            FEvent Done;
            RHIThread.Execute([this, &ResultID, &Done]()
            {
                ResultID = RHIThread.ExecuteCreateHeadlessSwapChain();
                Done.Trigger();
            });
            Done.Wait();
            return ResultID;
        }
    }

    void FRHI::
    DestroySwapChain(FWindow* I_Window)
    {
        if (!I_Window) { return; }
        MarkSwapChainDestroyed(QuerySwapChainID(I_Window));
        FEvent Done;
        RHIThread.Execute([this, Window = I_Window, &Done]()
        {
            RHIThread.ExecuteDestroySwapChain(Window, &Done);
        });
        Done.Wait();
    }

    void FRHI::
    DestroySwapChain(FRHISwapChainID I_SwapChainID)
    {
        if (I_SwapChainID == kInvalidSwapChainID) { return; }
        MarkSwapChainDestroyed(I_SwapChainID);
        FEvent Done;
        RHIThread.Execute([this, ID = I_SwapChainID, &Done]()
        {
            RHIThread.ExecuteDestroySwapChain(ID, &Done);
        });
        Done.Wait();
    }

    void FRHI::
    RecreateSwapChain(FWindow* I_Window)
    {
        if (!I_Window) { return; }
        RHIThread.RecreateSwapChain(I_Window);
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
                    auto Entry = std::move(R).GetValue();
                    Bool bSkipExecute = (Entry.SwapChainID < SwapChains.GetSize()
                        && SwapChains[Entry.SwapChainID].bDestroyed.Load(EMemoryOrder::Relaxed));
                    if (!bSkipExecute)
                    {
                        ExecuteImmediate(Entry.Cmd, Entry.SwapChainID);
                        PROFILING_ONLY_FIELD(
                        const auto& m = Entry.Cmd.GetProfilingMetrics();
                        auto& agg = CommandListProfilingMetrics;
                        if (m.PeakCommandCount > agg.PeakCommandCount) agg.PeakCommandCount = m.PeakCommandCount;
                        if (m.PeakBufferSizeBytes > agg.PeakBufferSizeBytes) agg.PeakBufferSizeBytes = m.PeakBufferSizeBytes;
                        if (m.PeakBufferCapacityBytes > agg.PeakBufferCapacityBytes) agg.PeakBufferCapacityBytes = m.PeakBufferCapacityBytes;
                        if (m.PeakCommandBytes > agg.PeakCommandBytes)
                        {
                            agg.PeakCommandBytes = m.PeakCommandBytes;
                            agg.PeakCommandType  = m.PeakCommandType;
                        }
                        );
                    }
                    Entry.Cmd.Reset();
                    Entry.Cmd.ShrinkTo(CommandListHighWaterMark);
                    FreeCommandListQueue.Enqueue(std::move(Entry.Cmd));
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
    Enqueue(FRHICommandList I_CommandList, FRHISwapChainID I_SwapChainID)
    {
        CommandListQueue.Enqueue(FCommandListEntry{ std::move(I_CommandList), I_SwapChainID });
        WakeEvent.Trigger();
    }

    void FRHI::FRHIThread::
    Execute(FImmediateTask I_Task)
    {
        if (!I_Task) { return; }
        ImmediateCommandQueue.Enqueue(std::move(I_Task));
        WakeEvent.Trigger();
    }

    void FRHI::FRHIThread::
    CreateSwapChain(FWindow* I_Window)
    {
        if (!I_Window) { return; }
        Execute([this, Window = I_Window]() { ExecuteCreateSwapChain(Window); });
    }

    Bool FRHI::FRHIThread::
    Initialize()
    {
        bInitSuccess = False;
        Execute([this]() { ExecuteInitialize(); });
        InitEvent.Wait();
        return bInitSuccess;
    }

    Bool FRHI::FRHIThread::
    Shutdown()
    {
        bShutdownSuccess = False;
        Execute([this]() { ExecuteShutdown(); });
        ShutdownEvent.Wait();
        return bShutdownSuccess;
    }

    void FRHI::FRHIThread::
    RecreateSwapChain(FWindow* I_Window)
    {
        if (!I_Window) { return; }
        Execute([this, Window = I_Window]() { ExecuteRecreateSwapChain(Window); });
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

        Driver->CreateDevice(PreferredGPUName);
        Driver->CreateAllocator();
        Driver->CreatePipelineCache();

        Registry    = MakeShared<FRHIRegistry>(Driver.Get());
        StagingRing = MakeUnique<FRHIStagingRingBuffer>(Driver.Get(), 16_MB);
        GraphicsCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Graphics>(False);
        TransferCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Transfer>(False);

        // Utility frame for synchronous one-off GPU work (readback, etc.).
        UtilityFrame.Emplace();
        UtilityFrame->ExecuteFence = Driver->CreateFence(True);
        UtilityFrame->SwapChainReadySemaphore = Driver->CreateSemaphore();
        UtilityFrame->GraphicsCalls = GraphicsCommandPool.CreateCommandBuffer(True);
        UtilityFrame->TransferFinishedSemaphore = Driver->CreateSemaphore();
        UtilityFrame->TransferCalls = TransferCommandPool.CreateCommandBuffer(True);

        // Pre-reserve so EmplaceBack never reallocates while Graphics thread holds references.
        SwapChains.Reserve(kMaxSwapChainCount);

        bInitSuccess = True;
        LOG_DEBUG("({}) RHI initialized (no reserved headless slot; use CreateSwapChain(nullptr) for headless).", Owner->GetRuntimeName());
        InitEvent.Trigger();
    }

    void FRHI::FRHIThread::
    ExecuteShutdown()
    {
        bShutdownSuccess = False;
        const FString RuntimeName = Owner ? Owner->GetRuntimeName() : FString("Unknown");

        if (!Driver)
        {
            bShutdownSuccess = True;
            ShutdownEvent.Trigger();
            LOG_DEBUG("({}) ExecuteShutdown: no driver, done.", RuntimeName);
            return;
        }

        // Wait for all queue submissions before destroying any semaphore/fence (VUID 05149 / 01120).
        Driver->WaitDeviceIdle();

        // Tear down swap chains while device is still alive.
        for (UInt8 Idx = 0; Idx < static_cast<UInt8>(SwapChains.GetSize()); ++Idx)
        {
            auto& SC = SwapChains[Idx];
            if (SC.bDestroyed.Load(EMemoryOrder::Relaxed)) { continue; }
            if (SC.Window)
            {
                LOG_TRACE("({}) Destroy windowed SwapChain (id:{}).", RuntimeName, Idx);
                Driver->DestroySwapChain(SC.Window);
            }
            else
            {
                LOG_TRACE("({}) Destroy headless SwapChain (id:{}).", RuntimeName, Idx);
            }
        }
        {
            FScopeWriteLock WriteLock(&WindowToSwapChainLock);
            WindowToSwapChainIndex.Clear();
        }
        SwapChains.Clear();
        FreeSlots.Clear();
        UtilityFrame.Reset();

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

        bShutdownSuccess = True;
        ShutdownEvent.Trigger();
    }

    UInt8 FRHI::FRHIThread::
    AllocateSlot()
    {
        if (!FreeSlots.IsEmpty())
        {
            UInt8 Idx = FreeSlots.Back();
            FreeSlots.PopBack();
            SwapChains[Idx] = FRHISwapChain{};
            return Idx;
        }
        VISERA_ASSERT(SwapChains.GetSize() < kMaxSwapChainCount
            && "SwapChain slots exhausted.");
        SwapChains.EmplaceBack();
        return static_cast<UInt8>(SwapChains.GetSize() - 1);
    }

    void FRHI::FRHIThread::
    FreeSlot(UInt8 I_Index)
    {
        if (I_Index >= SwapChains.GetSize()) { return; }
        auto& SC = SwapChains[I_Index];
        SC.bDestroyed.Store(True, EMemoryOrder::Relaxed);
        SC.InFlightFrames.Clear();
        SC.RenderFinishedSemaphores.Clear();
        SC.FrameSlotFreeEvent.Reset();
        SC.Window = nullptr;
        FreeSlots.PushBack(I_Index);
    }

    void FRHI::FRHIThread::
    ExecuteCreateSwapChain(FWindow* I_Window, void* I_PreCreatedSurface)
    {
        if (!I_Window) { return; }
        UInt8 SCIdx = kInvalidSwapChainID;
        {
            FScopeWriteLock WriteLock(&WindowToSwapChainLock);
            if (WindowToSwapChainIndex.Contains(I_Window)) { return; }
            Driver->CreateSwapChain(I_Window, I_PreCreatedSurface);
            SCIdx = AllocateSlot();
            SwapChains[SCIdx].Initialize(Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, I_Window);
            SwapChains[SCIdx].CachedProxyTextureID = FRHITextureID::CreateUnmanaged(
                FRHITextureHandle::CreateSwapChainProxy(SCIdx));
            WindowToSwapChainIndex.Insert(I_Window, SCIdx);
        }
        LOG_TRACE("({}) ExecuteCreateSwapChain: windowed (id:{}, title:{}).",
            Owner->GetRuntimeName(), SCIdx, I_Window->GetTitle());
    }

    FRHISwapChainID FRHI::FRHIThread::
    ExecuteCreateHeadlessSwapChain()
    {
        UInt8 SCIdx = AllocateSlot();
        SwapChains[SCIdx].Initialize(Driver.Get(), &GraphicsCommandPool, &TransferCommandPool, nullptr);
        LOG_TRACE("({}) ExecuteCreateHeadlessSwapChain: headless (id:{}, in-flight:{}).",
            Owner->GetRuntimeName(), SCIdx, SwapChains[SCIdx].InFlightFrames.GetSize());
        return SCIdx;
    }

    void FRHI::FRHIThread::
    ExecuteDestroySwapChain(FWindow* I_Window, FEvent* I_Done)
    {
        if (!I_Window) { if (I_Done) { I_Done->Trigger(); } return; }

        UInt8 Idx = kInvalidSwapChainID;
        {
            FScopeWriteLock WriteLock(&WindowToSwapChainLock);
            auto It = WindowToSwapChainIndex.Find(I_Window);
            if (It != WindowToSwapChainIndex.end())
            {
                Idx = It->second;
                WindowToSwapChainIndex.Erase(I_Window);
            }
        }
        if (Idx != kInvalidSwapChainID)
        {
            LOG_TRACE("({}) ExecuteDestroySwapChain: windowed (id:{}).", Owner->GetRuntimeName(), Idx);
            Driver->WaitSwapChainIdle(I_Window);
            FreeSlot(Idx);
        }
        Driver->DestroySwapChain(I_Window);
        if (I_Done) { I_Done->Trigger(); }
    }

    void FRHI::FRHIThread::
    ExecuteDestroySwapChain(FRHISwapChainID I_ID, FEvent* I_Done)
    {
        if (I_ID >= SwapChains.GetSize()) { if (I_Done) { I_Done->Trigger(); } return; }
        auto& SC = SwapChains[I_ID];
        for (auto& Frame : SC.InFlightFrames)
        {
            if (Frame.ExecuteFence.Wait(kFrameFenceTimeoutNs))
            { (void)Frame.ExecuteFence.Reset(); }
        }
        if (SC.Window)
        {
            Driver->DestroySwapChain(SC.Window);
            FScopeWriteLock WriteLock(&WindowToSwapChainLock);
            WindowToSwapChainIndex.Erase(SC.Window);
        }
        LOG_TRACE("({}) ExecuteDestroySwapChain: (id:{}, headless:{}).",
            Owner->GetRuntimeName(), I_ID, SC.Window == nullptr);
        FreeSlot(I_ID);
        if (I_Done) { I_Done->Trigger(); }
    }

    FRHISwapChainID FRHI::FRHIThread::
    QuerySwapChainID(FWindow* I_Window) const
    {
        if (!I_Window) { return kInvalidSwapChainID; }
        FScopeReadLock ReadLock(&WindowToSwapChainLock);
        auto It = WindowToSwapChainIndex.Find(I_Window);
        return It == WindowToSwapChainIndex.end() ? kInvalidSwapChainID : It->second;
    }

    void FRHI::FRHIThread::
    ExecuteRecreateSwapChain(FWindow* I_Window)
    {
        if (!I_Window || !Driver) { return; }
        FScopeReadLock ReadLock(&WindowToSwapChainLock);
        auto It = WindowToSwapChainIndex.Find(I_Window);
        if (It == WindowToSwapChainIndex.end()) { return; }

        const UInt8 Idx = It->second;
        auto& Ctx = SwapChains[Idx];
        if (Ctx.bFrameActive)
        {
            auto& Frame = Ctx.InFlightFrames[Ctx.FrameIndex];
            Frame.GraphicsCalls.End();
            Frame.TransferCalls.End();
            Ctx.bFrameActive = False;
        }

        Driver->RecreateSwapChain(I_Window);

        Ctx.Reinitialize(Driver.Get(), &GraphicsCommandPool, &TransferCommandPool);
        Ctx.CachedProxyTextureID = FRHITextureID::CreateUnmanaged(
            FRHITextureHandle::CreateSwapChainProxy(Idx));
        Ctx.bDirty.Store(False, EMemoryOrder::Relaxed);
    }

    Bool FRHI::FRHIThread::
    BeginFrame(FRHISwapChainID I_SwapChainID)
    {
        if (I_SwapChainID >= SwapChains.GetSize()) { return False; }
        auto& Ctx = SwapChains[I_SwapChainID];
        if (Ctx.bDestroyed.Load(EMemoryOrder::Relaxed)) { return False; }
        if (Ctx.InFlightFrames.IsEmpty()) { return False; }
        auto& Frame = Ctx.InFlightFrames[Ctx.FrameIndex];

        if (Frame.ExecuteFence.Wait(kFrameFenceTimeoutNs))
        {
            (void)Frame.ExecuteFence.Reset();
        }
        else LOG_ERROR("Failed to wait the fence of swapchain (id:{})", I_SwapChainID);

        Registry->SetCurrentRetirementFence(&Frame.ExecuteFence);
        Registry->CollectGarbage();

        if (auto* Win = Ctx.Window)
        {
            if (Ctx.bMinimized.Load(EMemoryOrder::Relaxed))
            {
                LOG_DEBUG("RHI BeginFrame: swapchain {} skipped (minimized).", I_SwapChainID);
                Frame.ExecuteFence = Driver->CreateFence(True);
                return False;
            }
            const UInt32 W = Win->GetWidth(), H = Win->GetHeight();
            if (W != Ctx.CachedWidth || H != Ctx.CachedHeight)
            {
                LOG_DEBUG("RHI BeginFrame: swapchain {} skipped (size change {}x{} -> {}x{}).",
                    I_SwapChainID, Ctx.CachedWidth, Ctx.CachedHeight, W, H);
                RequestRecreateSwapChain(I_SwapChainID);
                Frame.ExecuteFence = Driver->CreateFence(True);
                return False;
            }
            if (!Driver->WaitNextFrame(Win, &Frame.SwapChainReadySemaphore))
            {
                LOG_WARN("RHI BeginFrame: swapchain {} WaitNextFrame failed.", I_SwapChainID);
                RequestRecreateSwapChain(I_SwapChainID);
                Frame.ExecuteFence = Driver->CreateFence(True);
                return False;
            }
            Ctx.CachedWidth = W;
            Ctx.CachedHeight = H;
        }
        // Headless: fence waited above, no AcquireNextImage needed.

        Frame.GraphicsCalls.Reset();
        Frame.GraphicsCalls.Begin();
        Frame.TransferCalls.Reset();
        Frame.TransferCalls.Begin();
        Ctx.bFrameActive = True;
        return True;
    }

    void FRHI::FRHIThread::
    Present(FRHISwapChainID I_SwapChainID)
    {
        if (I_SwapChainID >= SwapChains.GetSize()) { return; }

        auto& Ctx = SwapChains[I_SwapChainID];
        if (Ctx.InFlightFrames.IsEmpty()) { return; }

        // FetchSub + Load is not atomic: another thread may FetchAdd between the two,
        // causing a spurious missed wakeup (Graphics thread waits one extra cycle). Benign.
        auto ReleaseSlot = [&Ctx, this]()
        {
            Ctx.PendingPresentCount.FetchSub(1, EMemoryOrder::Relaxed);
            if (Ctx.FrameSlotFreeEvent && Ctx.PendingPresentCount.Load(EMemoryOrder::Relaxed) < kMaxInFlightFrames)
            { Ctx.FrameSlotFreeEvent->Trigger(); }
        };

        if (Ctx.bDestroyed.Load(EMemoryOrder::Relaxed))
        {
            ReleaseSlot();
            return;
        }

        if (!Ctx.bFrameActive)
        {
            ReleaseSlot();
            return;
        }

        auto& Frame = Ctx.InFlightFrames[Ctx.FrameIndex];
        Frame.TransferCalls.End();
        Frame.GraphicsCalls.End();

        // Re-check bDestroyed before touching the driver (window close can set it after initial check).
        if (Ctx.bDestroyed.Load(EMemoryOrder::Relaxed))
        {
            Ctx.bFrameActive = False;
            ReleaseSlot();
            return;
        }

        if (auto* Win = Ctx.Window; Win)
        {
            auto* SC = Driver->GetSwapChain(Win);
            const UInt8 ImageIndex = SC ? static_cast<UInt8>(SC->Cursor) : 0U;

            Driver->Submit(&Frame.GraphicsCalls,
                           &Frame.SwapChainReadySemaphore,
                           &Ctx.RenderFinishedSemaphores[ImageIndex],
                           &Frame.ExecuteFence);
            if (!Driver->Present(Win, &Ctx.RenderFinishedSemaphores[ImageIndex]))
            {
                RequestRecreateSwapChain(I_SwapChainID);
            }
            else
            {
                auto Interval = Ctx.FrameTimer.Elapsed();
                Float Seconds = static_cast<Float>(Interval.Microseconds()) / 1e6f;
                Float InstantFPS = (Seconds > 1e-7f) ? (1.f / Seconds) : 0.f;
                constexpr Float kSmoothing = 0.05f;
                Float Smoothed = Ctx.FrameRate.Load(EMemoryOrder::Relaxed);
                Smoothed = (Smoothed == 0.f) ? InstantFPS : Smoothed + kSmoothing * (InstantFPS - Smoothed);
                Ctx.FrameRate.Store(Smoothed, EMemoryOrder::Relaxed);
                Ctx.FrameTimer.Reset();
            }
        }
        else
        {
            // Headless: submit Transfer then Graphics; no vkQueuePresent.
            Driver->Submit(&Frame.TransferCalls, nullptr, &Frame.TransferFinishedSemaphore, nullptr);
            Driver->Submit(&Frame.GraphicsCalls, &Frame.TransferFinishedSemaphore, nullptr, &Frame.ExecuteFence);
        }

        Ctx.bFrameActive = False;
        Ctx.FrameIndex = (Ctx.FrameIndex + 1) % static_cast<UInt8>(Ctx.InFlightFrames.GetSize());
        ReleaseSlot();
    }

    void FRHI::FRHIThread::
    RequestRecreateSwapChain(FRHISwapChainID I_SwapChainID)
    {
        if (I_SwapChainID >= SwapChains.GetSize()) { return; }
        auto& Ctx = SwapChains[I_SwapChainID];
        if (auto* Win = Ctx.Window)
        {
            Ctx.bDirty.Store(True, EMemoryOrder::Relaxed);
            Ctx.PendingRecreateWidth  = Win->GetWidth();
            Ctx.PendingRecreateHeight = Win->GetHeight();
            if (!Ctx.bRecreateEnqueued)
            {
                Ctx.bRecreateEnqueued = True;
                const FRHISwapChainID Idx = I_SwapChainID;
                Execute([this, Idx]()
                {
                    auto& Sc = SwapChains[Idx];
                    ExecuteRecreateSwapChain(Sc.Window);
                    Sc.bRecreateEnqueued = False;
                });
            }
        }
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
        if (CurrentSwapChainID == kInvalidSwapChainID)
        {
            LOG_TRACE("RHI Submit: dropped (no active frame; swap chain may have been destroyed).");
            return;
        }
        RHIThread.Enqueue(std::move(I_CommandList), CurrentSwapChainID);
    }

    void FRHI::
    Submit(const FRHICommandList& I_CommandList)
    {
        if (CurrentSwapChainID == kInvalidSwapChainID)
        {
            LOG_TRACE("RHI Submit: dropped (no active frame; swap chain may have been destroyed).");
            return;
        }
        FRHICommandList Copy(I_CommandList);
        RHIThread.Enqueue(std::move(Copy), CurrentSwapChainID);
    }

    FRHITextureID FRHI::
    BeginFrame(FRHISwapChainID I_SwapChainID)
    {
        if (I_SwapChainID >= RHIThread.SwapChains.GetSize()) { return {}; }
        auto& SC = RHIThread.SwapChains[I_SwapChainID];
        if (SC.bDestroyed.Load(EMemoryOrder::Relaxed)) { return {}; }
        const UInt32 MaxInFlight = GetMaxInFlightFrames();
        while (GetPendingPresentCount(I_SwapChainID) >= MaxInFlight)
        {
            if (SC.bDestroyed.Load(EMemoryOrder::Relaxed)) { return {}; }
            auto Event = SC.FrameSlotFreeEvent.Get();
            Event->WaitAndReset();
        }
        if (SC.bDestroyed.Load(EMemoryOrder::Relaxed)) { return {}; }
        CurrentSwapChainID = I_SwapChainID;
        return SC.CachedProxyTextureID;
    }

    void FRHI::
    EndFrame()
    {
        CurrentSwapChainID = kInvalidSwapChainID;
    }

    void FRHI::
    Present(FRHISwapChainID I_SwapChainID)
    {
        if (I_SwapChainID < RHIThread.SwapChains.GetSize())
        { RHIThread.SwapChains[I_SwapChainID].PendingPresentCount.FetchAdd(1, EMemoryOrder::Relaxed); }
        RHIThread.Execute([this, I_SwapChainID]
        {
            RHIThread.Present(I_SwapChainID);
        });
    }

    void FRHI::FRHIThread::
    ExecuteImmediate(FRHICommandList& I_CommandList, FRHISwapChainID I_SwapChainID)
    {
        if (I_SwapChainID >= SwapChains.GetSize()) { return; }
        auto* Ctx = &SwapChains[I_SwapChainID];
        if (Ctx->bDestroyed.Load(EMemoryOrder::Relaxed)) { return; }
        if (Ctx->InFlightFrames.IsEmpty()) { return; }

        if (!Ctx->bFrameActive)
        {
            if (!BeginFrame(I_SwapChainID))
            {
                LOG_TRACE("RHI ExecuteImmediate: BeginFrame({}) failed, skipping command list.", I_SwapChainID); return;
            }
        }
        FRHIInFlightFrame& Frame = Ctx->InFlightFrames[Ctx->FrameIndex];
        VISERA_ASSERT(Frame.GraphicsCalls.IsRecording());
        VISERA_ASSERT(Frame.TransferCalls.IsRecording());

        {
            FScopeReadLock ReadLock(&Registry->GetLock());
            for (auto Command : I_CommandList)
            {
                if (Command.PayloadPtrAligned == nullptr) { continue; }

                switch (Command.Type)
                {
                case ERHICommandType::TransitionTexture:   ExecuteTransitionTexture(Frame, Command); break;
                case ERHICommandType::MemoryBarrier:       ExecuteMemoryBarrier(Frame, Command); break;
                case ERHICommandType::BufferBarrier:       ExecuteBufferBarrier(Frame, Command); break;
                case ERHICommandType::ClearColorImage:     ExecuteClearColorImage(Frame, Command); break;
                case ERHICommandType::BlitImage:           ExecuteBlitImage(Frame, Command); break;
                case ERHICommandType::CopyImage:           ExecuteCopyImage(Frame, Command); break;
                case ERHICommandType::CopyBufferToImage:   ExecuteCopyBufferToImage(Frame, Command); break;
                case ERHICommandType::CopyImageToBuffer:   ExecuteCopyImageToBuffer(Frame, Command); break;
                case ERHICommandType::WriteBuffer:         ExecuteWriteBuffer(Frame, Command); break;
                case ERHICommandType::EnterRenderPass:     ExecuteEnterRenderPass(Frame, Command); break;
                case ERHICommandType::LeaveRenderPass:     ExecuteLeaveRenderPass(Frame, Command); break;
                case ERHICommandType::SetViewport:         ExecuteSetViewport(Frame, Command); break;
                case ERHICommandType::SetScissor:          ExecuteSetScissor(Frame, Command); break;
                case ERHICommandType::BindVertexBuffer:    ExecuteBindVertexBuffer(Frame, Command); break;
                case ERHICommandType::BindDescriptorSet:   ExecuteBindDescriptorSet(Frame, Command); break;
                case ERHICommandType::PushConstants:       ExecutePushConstants(Frame, Command); break;
                case ERHICommandType::Draw:                ExecuteDraw(Frame, Command); break;
                case ERHICommandType::DrawIndexed:         ExecuteDrawIndexed(Frame, Command); break;
                case ERHICommandType::EnterComputePass:
                {
                    const auto& Payload = *reinterpret_cast<const FRHICommandList::FEnterComputePass*>(Command.PayloadPtrAligned);
                    auto* ComputePass = Registry->Get(Payload.ComputePass);
                    VISERA_ASSERT(ComputePass != nullptr);
                    auto* VulkanPipeline = ComputePass->GetVulkanComputePipeline();
                    Frame.GraphicsCalls.GetHandle().bindPipeline(vk::PipelineBindPoint::eCompute, *VulkanPipeline->GetHandle());
                    if (VulkanPipeline->GetLayout())
                    { CurrentComputePipelineLayout = VulkanPipeline->GetLayout(); }
                    LOG_TRACE("ExecuteImmediate: EnterComputePass.");
                    break;
                }
                case ERHICommandType::LeaveComputePass:
                {
                    CurrentComputePipelineLayout = nullptr;
                    LOG_TRACE("ExecuteImmediate: LeaveComputePass.");
                    break;
                }
                case ERHICommandType::Dispatch:
                {
                    const auto& Payload = *reinterpret_cast<const FRHICommandList::FDispatch*>(Command.PayloadPtrAligned);
                    Frame.GraphicsCalls.GetHandle().dispatch(Payload.GroupCountX, Payload.GroupCountY, Payload.GroupCountZ);
                    LOG_TRACE("ExecuteImmediate: Dispatch({}, {}, {}).", Payload.GroupCountX, Payload.GroupCountY, Payload.GroupCountZ);
                    break;
                }
                default: LOG_ERROR("({}) Unknown Command Type: {}", Owner ? Owner->GetRuntimeName() : FString("Unknown"), static_cast<UInt16>(Command.Type)); break;
                }
            }
        }
    }

    void FRHI::FRHIThread::
    ExecuteTransitionTexture(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FTransitionTexturePayload>(I_Cmd);
        auto* Img = GetVulkanImageChecked(Payload.Image);
        vk::ImageLayout OldLayout = TypeCast(Payload.OldLayout);
        vk::ImageLayout NewLayout = TypeCast(Payload.NewLayout);
        if (NewLayout == vk::ImageLayout::eUndefined || OldLayout == NewLayout)
        { return; }

        auto SubresourceRange = vk::ImageSubresourceRange{}
            .setAspectMask    (vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel  (0)
            .setLevelCount    (Img->GetMipmapLevels())
            .setBaseArrayLayer(0)
            .setLayerCount    (Img->GetArrayLayers());

        auto Barrier = vk::ImageMemoryBarrier2{}
            .setSrcStageMask       (TypeCast(Payload.SourceStage))
            .setSrcAccessMask      (TypeCast(Payload.SourceAccess))
            .setDstStageMask       (TypeCast(Payload.DestStage))
            .setDstAccessMask      (TypeCast(Payload.DestAccess))
            .setOldLayout          (OldLayout)
            .setNewLayout          (NewLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage              (Img->GetHandle())
            .setSubresourceRange   (SubresourceRange);

        auto DependencyInfo = vk::DependencyInfo{}
            .setImageMemoryBarrierCount(1)
            .setPImageMemoryBarriers   (&Barrier);
        I_Frame.GraphicsCalls.GetHandle().pipelineBarrier2(DependencyInfo);
    }

    void FRHI::FRHIThread::
    ExecuteMemoryBarrier(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FMemoryBarrierPayload>(I_Cmd);

        auto Barrier = vk::MemoryBarrier2{}
            .setSrcStageMask (TypeCast(Payload.SourceStage))
            .setSrcAccessMask(TypeCast(Payload.SourceAccess))
            .setDstStageMask (TypeCast(Payload.DestStage))
            .setDstAccessMask(TypeCast(Payload.DestAccess));

        auto DependencyInfo = vk::DependencyInfo{}
            .setMemoryBarrierCount(1)
            .setPMemoryBarriers   (&Barrier);
        I_Frame.GraphicsCalls.GetHandle().pipelineBarrier2(DependencyInfo);
    }

    void FRHI::FRHIThread::
    ExecuteBufferBarrier(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FBufferBarrierPayload>(I_Cmd);
        auto* Buf = GetVulkanBufferChecked(Payload.Buffer);

        auto Barrier = vk::BufferMemoryBarrier2{}
            .setSrcStageMask       (TypeCast(Payload.SourceStage))
            .setSrcAccessMask      (TypeCast(Payload.SourceAccess))
            .setDstStageMask       (TypeCast(Payload.DestStage))
            .setDstAccessMask      (TypeCast(Payload.DestAccess))
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setBuffer             (Buf->GetHandle())
            .setOffset             (Payload.Offset)
            .setSize               (Payload.Size == 0 ? vk::WholeSize : Payload.Size);

        auto DependencyInfo = vk::DependencyInfo{}
            .setBufferMemoryBarrierCount(1)
            .setPBufferMemoryBarriers   (&Barrier);
        I_Frame.GraphicsCalls.GetHandle().pipelineBarrier2(DependencyInfo);
    }

    void FRHI::FRHIThread::
    ExecuteClearColorImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FClearColorImage>(I_Cmd);
        auto* Img = GetVulkanImageChecked(Payload.Image);
        if (!Img)
        { LOG_ERROR("ExecuteClearColorImage: swapchain/image resolved to null, skip."); return; }
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
    ExecuteCopyImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FCopyImage>(I_Cmd);
        auto* SrcImg = GetVulkanImageChecked(Payload.SrcImage);
        auto* DstImg = GetVulkanImageChecked(Payload.DstImage);
        I_Frame.GraphicsCalls.CopyImage(SrcImg, DstImg,
            TypeCast(Payload.SrcImageLayout), TypeCast(Payload.DstImageLayout));
    }

    void FRHI::FRHIThread::
    ExecuteCopyBufferToImage(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FCopyBufferToImage>(I_Cmd);
        auto* VulkanBuffer = GetVulkanBufferChecked(Payload.Buffer);
        auto* VulkanImage  = GetVulkanImageChecked(Payload.Image);
        vk::ImageLayout InitialLayout = TypeCast(Payload.InitialLayout);
        vk::ImageLayout TransferDst   = TypeCast(ERHIImageLayout::TransferDst);
        vk::ImageLayout FinalLayout   = TypeCast(Payload.FinalLayout);

        auto SubresourceRange = vk::ImageSubresourceRange{}
            .setAspectMask    (vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel  (0)
            .setLevelCount    (VulkanImage->GetMipmapLevels())
            .setBaseArrayLayer(0)
            .setLayerCount    (VulkanImage->GetArrayLayers());

        auto MakeImageBarrier = [&](vk::ImageLayout I_Old, vk::ImageLayout I_New,
                                    vk::PipelineStageFlags2 I_SrcStage, vk::AccessFlags2 I_SrcAccess,
                                    vk::PipelineStageFlags2 I_DstStage, vk::AccessFlags2 I_DstAccess)
        {
            auto Barrier = vk::ImageMemoryBarrier2{}
                .setSrcStageMask       (I_SrcStage)
                .setSrcAccessMask      (I_SrcAccess)
                .setDstStageMask       (I_DstStage)
                .setDstAccessMask      (I_DstAccess)
                .setOldLayout          (I_Old)
                .setNewLayout          (I_New)
                .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                .setImage              (VulkanImage->GetHandle())
                .setSubresourceRange   (SubresourceRange);
            auto DepInfo = vk::DependencyInfo{}
                .setImageMemoryBarrierCount(1)
                .setPImageMemoryBarriers   (&Barrier);
            I_Frame.TransferCalls.GetHandle().pipelineBarrier2(DepInfo);
        };

        MakeImageBarrier(InitialLayout, TransferDst,
            vk::PipelineStageFlagBits2::eTopOfPipe, vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eTransfer,  vk::AccessFlagBits2::eTransferWrite);

        I_Frame.TransferCalls.CopyBufferToImage(VulkanBuffer, VulkanImage, TransferDst);

        MakeImageBarrier(TransferDst, FinalLayout,
            vk::PipelineStageFlagBits2::eTransfer,     vk::AccessFlagBits2::eTransferWrite,
            vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eNone);
    }

    void FRHI::FRHIThread::
    ExecuteCopyImageToBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FCopyImageToBuffer>(I_Cmd);
        auto* SourceImage = GetVulkanImageChecked(Payload.SourceTexture);
        auto* DestBuffer  = GetVulkanBufferChecked(Payload.DestBuffer);
        const vk::Offset2D ImageOffset = TypeCast(Payload.ImageOffset);
        const vk::Extent2D ImageExtent = TypeCast(Payload.ImageExtent);
        I_Frame.TransferCalls.CopyImageToBuffer(SourceImage, DestBuffer, Payload.DestBufferOffset,
            ImageOffset, ImageExtent, vk::ImageLayout::eTransferSrcOptimal);
    }

    void FRHI::FRHIThread::
    ExecuteWriteBuffer(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FWriteBuffer>(I_Cmd);
        auto* TargetBuffer  = GetVulkanBufferChecked(Payload.TargetBuffer);
        auto* StagingBuf = GetVulkanBufferChecked(Payload.StagingBuffer);
        I_Frame.TransferCalls.CopyBuffer(StagingBuf, TargetBuffer);
    }

    void FRHI::FRHIThread::
    ExecuteEnterRenderPass(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        const auto& Payload = DecodePayload<FRHICommandList::FEnterRenderPass>(I_Cmd);
        auto* RenderPass = Registry->Get(Payload.RenderPass);
        if (!RenderPass) { return; }
        auto* Pipeline = RenderPass->GetVulkanRenderPipeline();
        if (Payload.ColorTargetCount == 0)
        {
            LOG_WARN("ExecuteEnterRenderPass: ColorTargetCount is 0, skipping.");
            return;
        }

        FVulkanColorAttachment  ColorAttachments[kMaxColorAttachments];
        FVulkanColorAttachment* ColorAttachmentPtrs[kMaxColorAttachments];
        UInt32 ValidCount = 0;
        for (UInt32 i = 0; i < Payload.ColorTargetCount; ++i)
        {
            const auto& Slot = Payload.ColorSlots[i];
            if (Slot.ColorTexture == FRHITextureHandle{}) { continue; }
            auto* ImageView = GetVulkanImageViewChecked(Slot.ColorTexture);
            if (!ImageView)
            {
                LOG_WARN("ExecuteEnterRenderPass: slot {} has invalid ImageView (handle={}), skipping.",
                    i, Slot.ColorTexture);
                continue;
            }
            ColorAttachments[ValidCount] = FVulkanColorAttachment(ImageView);
            ColorAttachments[ValidCount].SetLoadOp(TypeCast(Slot.ColorLoadOp));
            ColorAttachments[ValidCount].SetStoreOp(TypeCast(Slot.ColorStoreOp));
            ColorAttachments[ValidCount].SetClearColor(vk::ClearColorValue(
                Slot.ColorClearValue.R, Slot.ColorClearValue.G, Slot.ColorClearValue.B, Slot.ColorClearValue.A));
            ColorAttachmentPtrs[ValidCount] = &ColorAttachments[ValidCount];
            ++ValidCount;
        }

        if (ValidCount == 0)
        {
            LOG_WARN("ExecuteEnterRenderPass: no valid color attachments after filtering, skipping render pass.");
            return;
        }

        Pipeline->SetColorAttachments(ColorAttachmentPtrs, ValidCount);

        const auto& DsSlot = Payload.DepthStencilSlot;
        if (DsSlot.DepthStencilTexture != FRHITextureHandle{})
        {
            auto* DsImageView = GetVulkanImageViewChecked(DsSlot.DepthStencilTexture);
            if (DsImageView)
            {
                FVulkanDepthStencilAttachment DepthStencilAttachment(DsImageView);
                DepthStencilAttachment
                    .SetDepthLoadOp   (TypeCast(DsSlot.DepthLoadOp))
                    .SetDepthStoreOp  (TypeCast(DsSlot.DepthStoreOp))
                    .SetStencilLoadOp (TypeCast(DsSlot.StencilLoadOp))
                    .SetStencilStoreOp(TypeCast(DsSlot.StencilStoreOp))
                    .SetClearDepthStencil(vk::ClearDepthStencilValue{DsSlot.DepthClearValue, DsSlot.StencilClearValue});
                Pipeline->SetDepthStencilAttachment(&DepthStencilAttachment);
            }
            else
            { Pipeline->ClearDepthStencilAttachment(); }
        }
        else
        { Pipeline->ClearDepthStencilAttachment(); }

        auto* Image = GetVulkanImageChecked(Payload.ColorSlots[0].ColorTexture);
        if (!Image)
        {
            LOG_WARN("ExecuteEnterRenderPass: could not resolve image for first color slot, skipping.");
            return;
        }
        const auto Ext = Image->GetExtent();
        if (Ext.width == 0 || Ext.height == 0)
        {
            LOG_WARN("ExecuteEnterRenderPass: render area extent is {}x{}, skipping.", Ext.width, Ext.height);
            return;
        }
        Pipeline->SetRenderArea(vk::Rect2D{}
            .setOffset({0, 0})
            .setExtent({Ext.width, Ext.height}));
        I_Frame.GraphicsCalls.EnterRenderPipeline(Pipeline);
    }

    void FRHI::FRHIThread::
    ExecuteLeaveRenderPass(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        if (!I_Frame.GraphicsCalls.IsInsideRenderPass())
        {
            LOG_WARN("ExecuteLeaveRenderPass: not inside a render pass (prior EnterRenderPass likely skipped), skipping.");
            return;
        }
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

        if (CurrentComputePipelineLayout)
        {
            I_Frame.GraphicsCalls.GetHandle().bindDescriptorSets(
                vk::PipelineBindPoint::eCompute,
                CurrentComputePipelineLayout->GetHandle(),
                Payload.SetIndex,
                {DescriptorSet->GetVulkanDescriptorSet()->GetHandle()},
                {});
        }
        else
        {
            if (!I_Frame.GraphicsCalls.IsInsideRenderPass()) { return; }
            I_Frame.GraphicsCalls.BindDescriptorSet(Payload.SetIndex, DescriptorSet->GetVulkanDescriptorSet());
        }
    }

    void FRHI::FRHIThread::
    ExecuteDraw(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        if (!I_Frame.GraphicsCalls.IsInsideRenderPass()) { return; }
        const auto& Payload = DecodePayload<FRHICommandList::FDraw>(I_Cmd);
        I_Frame.GraphicsCalls.Draw(Payload.VertexCount, Payload.InstanceCount, Payload.FirstVertex, Payload.FirstInstance);
    }

    void FRHI::FRHIThread::
    ExecutePushConstants(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        if (!I_Frame.GraphicsCalls.IsInsideRenderPass()) { return; }
        const auto& Payload = DecodePayload<FRHICommandList::FPushConstants>(I_Cmd);
        I_Frame.GraphicsCalls.PushConstants(Payload.Data, Payload.Offset, Payload.Size);
    }

    void FRHI::FRHIThread::
    ExecuteDrawIndexed(FRHIInFlightFrame& I_Frame, const FRHICommandView& I_Cmd)
    {
        if (!I_Frame.GraphicsCalls.IsInsideRenderPass()) { return; }
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
            if (SCIdx >= SwapChains.GetSize())
            { LOG_WARN("GetVulkanImageChecked: swapchain proxy SCIdx {} >= SwapChains.GetSize().", SCIdx); return nullptr; }
            if (auto* Win = SwapChains[SCIdx].Window)
            {
                auto* SC = Driver->GetSwapChain(Win);
                if (!SC)
                { LOG_WARN("GetVulkanImageChecked: swapchain proxy SCIdx {} GetSwapChain(Win) null.", SCIdx); return nullptr; }
                auto* Img = SC->GetCurrentImage();
                if (!Img)
                { LOG_WARN("GetVulkanImageChecked: swapchain proxy SCIdx {} GetCurrentImage() null.", SCIdx); return nullptr; }
                return Img;
            }
            LOG_WARN("GetVulkanImageChecked: swapchain proxy SCIdx {} no Window.", SCIdx);
            return nullptr;
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
            if (auto* Win = SwapChains[SCIdx].Window)
            {
                auto* SC = Driver->GetSwapChain(Win);
                return SC ? SC->GetCurrentImageView() : nullptr;
            }
            return nullptr;
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

    Bool FRHI::IsSwapChainDirty(FRHISwapChainID I_SwapChainID) const
    {
        if (I_SwapChainID >= RHIThread.SwapChains.GetSize()) { return False; }
        return RHIThread.SwapChains[I_SwapChainID].bDirty.Load(EMemoryOrder::Relaxed);
    }

    void FRHI::UpdateSwapChainMinimized(FRHISwapChainID I_SwapChainID, Bool I_bMinimized)
    {
        if (I_SwapChainID >= RHIThread.SwapChains.GetSize()) { return; }
        RHIThread.SwapChains[I_SwapChainID].bMinimized.Store(I_bMinimized, EMemoryOrder::Relaxed);
    }

    void FRHI::MarkSwapChainDestroyed(FRHISwapChainID I_SwapChainID)
    {
        if (I_SwapChainID >= RHIThread.SwapChains.GetSize()) { return; }
        RHIThread.SwapChains[I_SwapChainID].bDestroyed.Store(True, EMemoryOrder::Relaxed);
        if (auto* Evt = RHIThread.SwapChains[I_SwapChainID].FrameSlotFreeEvent.Get())
        { Evt->Trigger(); }
    }

    Bool FRHI::IsValidSwapChain(FRHISwapChainID I_SwapChainID) const
    {
        if (I_SwapChainID >= RHIThread.SwapChains.GetSize()) { return False; }
        const auto& SC = RHIThread.SwapChains[I_SwapChainID];
        if (SC.bDestroyed.Load(EMemoryOrder::Relaxed)) { return False; }
        if (!SC.Window) { return True; }
        return !SC.bMinimized.Load(EMemoryOrder::Relaxed);
    }

    Bool FRHI::HasWindow(FRHISwapChainID I_SwapChainID) const
    {
        if (I_SwapChainID >= RHIThread.SwapChains.GetSize()) { return False; }
        return RHIThread.SwapChains[I_SwapChainID].Window != nullptr;
    }

    UInt32 FRHI::GetPendingPresentCount(FRHISwapChainID I_SwapChainID) const
    {
        if (I_SwapChainID >= RHIThread.SwapChains.GetSize()) { return 0; }
        return RHIThread.SwapChains[I_SwapChainID].PendingPresentCount.Load(EMemoryOrder::Relaxed);
    }

    Float FRHI::GetFrameRate(FWindow* I_Window) const
    {
        if (!I_Window) { return 0.f; }
        FRHISwapChainID const id = QuerySwapChainID(I_Window);
        if (id == kInvalidSwapChainID || id >= RHIThread.SwapChains.GetSize()) { return 0.f; }
        FRHISwapChain const& sc = RHIThread.SwapChains[id];
        if (!sc.Window) { return 0.f; }
        return sc.FrameRate.Load(EMemoryOrder::Relaxed);
    }

    void FRHI::WaitDeviceIdle() const
    {
        FRHI* Self = const_cast<FRHI*>(this);
        Self->RHIThread.IdleSyncEvent.Reset();
        Self->RHIThread.Execute([Driver = Self->RHIThread.Driver.Get(), &IdleSyncEvent = Self->RHIThread.IdleSyncEvent]()
        {
            if (Driver) { Driver->WaitDeviceIdle(); }
            IdleSyncEvent.Trigger();
        });
        Self->RHIThread.IdleSyncEvent.Wait();
    }

    void FRHI::WaitSwapChainIdle(FWindow* I_Window) const
    {
        if (!I_Window) { return; }
        FRHI* Self = const_cast<FRHI*>(this);
        Self->RHIThread.IdleSyncEvent.Reset();
        Self->RHIThread.Execute([Driver = Self->RHIThread.Driver.Get(), Window = I_Window, &IdleSyncEvent = Self->RHIThread.IdleSyncEvent]()
        {
            if (Driver) { Driver->WaitSwapChainIdle(Window); }
            IdleSyncEvent.Trigger();
        });
        Self->RHIThread.IdleSyncEvent.Wait();
    }

    void FRHI::FRHIThread::WaitDeviceIdle() const
    {
        if (Driver) { Driver->WaitDeviceIdle(); }
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

    void FRHI::
    ReadbackTexture(const FRHITextureID& I_Texture, void* I_OutData, UInt64 I_Size, UInt32 I_Width, UInt32 I_Height)
    {
        if (!I_OutData || I_Size == 0 || I_Width == 0 || I_Height == 0) { return; }
        FEvent Done;
        const FRHITextureHandle TextureHandle = I_Texture.GetHandle();
        RHIThread.Execute([this, TextureHandle, I_OutData, I_Size, I_Width, I_Height, EventPtr = &Done]() noexcept
        {
            RHIThread.DoReadbackTexture(FRHITextureID::CreateUnmanaged(TextureHandle), I_OutData, I_Size, I_Width, I_Height);
            EventPtr->Trigger();
        });
        Done.WaitAndReset();
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

        auto SubresourceRange = vk::ImageSubresourceRange{}
            .setAspectMask    (vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel  (0)
            .setLevelCount    (VulkanImage->GetMipmapLevels())
            .setBaseArrayLayer(0)
            .setLayerCount    (VulkanImage->GetArrayLayers());

        auto MakeImageBarrier = [&](vk::CommandBuffer I_CmdBuf,
                                    vk::ImageLayout I_Old, vk::ImageLayout I_New,
                                    vk::PipelineStageFlags2 I_SrcStage, vk::AccessFlags2 I_SrcAccess,
                                    vk::PipelineStageFlags2 I_DstStage, vk::AccessFlags2 I_DstAccess)
        {
            auto Barrier = vk::ImageMemoryBarrier2{}
                .setSrcStageMask       (I_SrcStage)
                .setSrcAccessMask      (I_SrcAccess)
                .setDstStageMask       (I_DstStage)
                .setDstAccessMask      (I_DstAccess)
                .setOldLayout          (I_Old)
                .setNewLayout          (I_New)
                .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                .setImage              (VulkanImage->GetHandle())
                .setSubresourceRange   (SubresourceRange);
            auto DepInfo = vk::DependencyInfo{}
                .setImageMemoryBarrierCount(1)
                .setPImageMemoryBarriers   (&Barrier);
            I_CmdBuf.pipelineBarrier2(DepInfo);
        };

        auto Cmd = TransferCommandPool.CreateCommandBuffer(True);
        Cmd.Begin();
        MakeImageBarrier(Cmd.GetHandle(),
            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
            vk::PipelineStageFlagBits2::eTopOfPipe, vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eTransfer,  vk::AccessFlagBits2::eTransferWrite);

        Cmd.CopyBufferToImage(StagingBuffer, VulkanImage, Alloc.Offset, vk::ImageLayout::eTransferDstOptimal);

        MakeImageBarrier(Cmd.GetHandle(),
            vk::ImageLayout::eTransferDstOptimal, DstLayout,
            vk::PipelineStageFlagBits2::eTransfer,     vk::AccessFlagBits2::eTransferWrite,
            vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eNone);
        Cmd.End();

        FVulkanFence Fence = Driver->CreateFence(False);
        Driver->Submit(&Cmd, nullptr, nullptr, &Fence);
        // Fence wait is required: (1) StagingRing correctness - AdvanceFence must run only after the GPU
        // finishes reading from this staging region, else the next allocation could overwrite in-flight data;
        // (2) API contract - this is a synchronous API, caller expects data ready on return.
        // For async uploads, use CommandList (WriteBuffer, CopyBufferToImage) instead.
        if (!Fence.Wait(kUploadFenceTimeoutNs))
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

        auto SubresourceRange = vk::ImageSubresourceRange{}
            .setAspectMask    (vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel  (0)
            .setLevelCount    (VulkanImage->GetMipmapLevels())
            .setBaseArrayLayer(0)
            .setLayerCount    (VulkanImage->GetArrayLayers());

        auto Barrier = vk::ImageMemoryBarrier2{}
            .setSrcStageMask       (vk::PipelineStageFlagBits2::eAllCommands)
            .setSrcAccessMask      (vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite)
            .setDstStageMask       (vk::PipelineStageFlagBits2::eAllCommands)
            .setDstAccessMask      (vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite)
            .setOldLayout          (OldLayout)
            .setNewLayout          (NewLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage              (VulkanImage->GetHandle())
            .setSubresourceRange   (SubresourceRange);

        auto Cmd = GraphicsCommandPool.CreateCommandBuffer(True);
        Cmd.Begin();
        auto DepInfo = vk::DependencyInfo{}
            .setImageMemoryBarrierCount(1)
            .setPImageMemoryBarriers   (&Barrier);
        Cmd.GetHandle().pipelineBarrier2(DepInfo);
        Cmd.End();

        FVulkanFence Fence = Driver->CreateFence(False);
        Driver->Submit(&Cmd, nullptr, nullptr, &Fence);
        if (!Fence.Wait(kUploadFenceTimeoutNs))
        { LOG_FATAL("TransitionTexture: fence wait failed!"); }
    }

    void FRHI::FRHIThread::
    DoReadbackTexture(const FRHITextureID& I_Texture, void* I_OutData, UInt64 I_Size, UInt32 I_Width, UInt32 I_Height)
    {
        auto* Tex = Registry->Get(I_Texture.GetHandle());
        if (!Tex) { LOG_ERROR("DoReadbackTexture: invalid texture handle."); return; }

        FRHIBufferCreateInfo StagingInfo;
        StagingInfo.Size   = I_Size;
        StagingInfo.Usages = ERHIBufferUsage::TransferDst;
        FRHIBufferID StagingBufferID = CreateBuffer(std::move(StagingInfo));

        if (!UtilityFrame.HasValue())
        { LOG_ERROR("DoReadbackTexture: UtilityFrame not available."); return; }

        FVulkanImage* VulkanImage = Tex->GetVulkanImage();
        FVulkanBuffer* VulkanBuffer = GetVulkanBufferChecked(StagingBufferID.GetHandle());

        FRHIInFlightFrame& Frame = UtilityFrame.GetValue();
        Frame.TransferCalls.Reset();
        Frame.TransferCalls.Begin();
        // Image is already in TransferSrc from the caller's command list (e.g. after clear + transition). Copy only.
        Frame.TransferCalls.CopyImageToBuffer(VulkanImage, VulkanBuffer, 0,
            vk::Offset2D{0, 0}, vk::Extent2D{I_Width, I_Height}, vk::ImageLayout::eTransferSrcOptimal);
        Frame.TransferCalls.End();

        (void)Frame.ExecuteFence.Reset();
        Driver->Submit(&Frame.TransferCalls, nullptr, nullptr, &Frame.ExecuteFence);
        if (!Frame.ExecuteFence.Wait(kUtilityFenceTimeoutNs))
        { LOG_ERROR("DoReadbackTexture: utility fence wait timed out."); return; }
        (void)Frame.ExecuteFence.Reset();

        void* MappedPointer = VulkanBuffer->GetMappedPtr();
        if (MappedPointer) { Memory::Memcpy(I_OutData, MappedPointer, I_Size); }
        else { LOG_ERROR("DoReadbackTexture: staging buffer not mappable."); }
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
        if (!Fence.Wait(kUploadFenceTimeoutNs))
        { LOG_FATAL("UploadBuffer: fence wait failed!"); }

        StagingRing->AdvanceFence(Alloc.Offset + Alloc.Size);
    }
}