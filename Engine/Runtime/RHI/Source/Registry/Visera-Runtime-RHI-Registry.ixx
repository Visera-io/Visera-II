module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Registry;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Registry.Handle;
       import Visera.Core.OS.Thread.Sync.Atomic;
       import Visera.Core.OS.Thread.Sync.RWLock;
       import Visera.Core.Types.Pointer.Shared;
       import Visera.Core.Types.Pointer.Weak;
       import Visera.Runtime.RHI.Common;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.RHI.Resource;
       import Visera.Core.Containers.SlotMap;
       import Visera.Core.Containers.Map;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.Tuple;
       import Visera.Core.OS.Thread.Queue.SPSC;
       import Visera.Core.Algorithm.Ranges;
       import Visera.Core.Math.Hash.GoldenRatio;
       import Visera.Core.Log;
       import vulkan_hpp;

export namespace Visera
{
    class FRHIRegistry;

    template<Concepts::RHIHandle RHIHandle>
    class VISERA_RUNTIME_API TRHIRegistryEntry
    {
        struct FControlBlock
        {
            TWeakPtr<FRHIRegistry> Registry {};
            RHIHandle              Handle   {};
            TAtomic<UInt32>        RefCount {0};
        };

    public:
        [[nodiscard]] constexpr Bool
        IsNull() const { return Block? (Block->Handle == RHIHandle{}) : (False); }
        [[nodiscard]] RHIHandle
        GetHandle() const { return Block ? Block->Handle : RHIHandle{}; }

        TRHIRegistryEntry() = default;
        TRHIRegistryEntry(TWeakPtr<FRHIRegistry> I_Registry, RHIHandle I_Handle);
        TRHIRegistryEntry(const TRHIRegistryEntry& I_Other);
        TRHIRegistryEntry(TRHIRegistryEntry&& I_Other) noexcept;
        TRHIRegistryEntry& operator=(const TRHIRegistryEntry& I_Other);
        TRHIRegistryEntry& operator=(TRHIRegistryEntry&& I_Other) noexcept;
        ~TRHIRegistryEntry();

        constexpr operator Bool()      const { return !IsNull();   }
        constexpr operator RHIHandle() const { return GetHandle(); }

        static TRHIRegistryEntry CreateUnmanaged(RHIHandle I_Handle)
        {
            TRHIRegistryEntry Entry;
            Entry.Block = new FControlBlock{TWeakPtr<FRHIRegistry>{}, I_Handle, 1};
            return Entry;
        }

    private:
        FControlBlock* Block {nullptr};
        void Release();
    };

    using FRHITextureID       = TRHIRegistryEntry<FRHITextureHandle>;
    using FRHISamplerID       = TRHIRegistryEntry<FRHISamplerHandle>;
    using FRHIBufferID        = TRHIRegistryEntry<FRHIBufferHandle>;
    using FRHIDescriptorSetID = TRHIRegistryEntry<FRHIDescriptorSetHandle>;
    using FRHIShaderID        = TRHIRegistryEntry<FRHIShaderHandle>;
    using FRHIRenderPassID    = TRHIRegistryEntry<FRHIRenderPassHandle>;
    using FRHIComputePassID   = TRHIRegistryEntry<FRHIComputePassHandle>;

    class VISERA_RUNTIME_API FRHIRegistry : public FEnableSharedFromThis<FRHIRegistry>
    {
    public:
        [[nodiscard]] FRHITexture*
        Get(FRHITextureHandle I_Handle) { return Textures.Get(I_Handle); }
        [[nodiscard]] FRHIBuffer*
        Get(FRHIBufferHandle I_Handle) { return Buffers.Get(I_Handle); }
        [[nodiscard]] FRHISampler*
        Get(FRHISamplerHandle I_Handle) { return Samplers.Get(I_Handle); }
        [[nodiscard]] FRHIDescriptorSet*
        Get(FRHIDescriptorSetHandle I_Handle) { return DescriptorSets.Get(I_Handle); }
        [[nodiscard]] FRHIShader*
        Get(FRHIShaderHandle I_Handle) { return Shaders.Get(I_Handle); }
        [[nodiscard]] FRHIRenderPass*
        Get(FRHIRenderPassHandle I_Handle) { return RenderPasses.Get(I_Handle); }
        [[nodiscard]] FRHIComputePass*
        Get(FRHIComputePassHandle I_Handle) { return ComputePasses.Get(I_Handle); }
        [[nodiscard]] const FRHITexture*
        Get(FRHITextureHandle I_Handle) const { return Textures.Get(I_Handle); }
        [[nodiscard]] const FRHIBuffer*
        Get(FRHIBufferHandle I_Handle) const { return Buffers.Get(I_Handle); }
        [[nodiscard]] const FRHISampler*
        Get(FRHISamplerHandle I_Handle) const { return Samplers.Get(I_Handle); }
        [[nodiscard]] const FRHIDescriptorSet*
        Get(FRHIDescriptorSetHandle I_Handle) const { return DescriptorSets.Get(I_Handle); }
        [[nodiscard]] const FRHIShader*
        Get(FRHIShaderHandle I_Handle) const { return Shaders.Get(I_Handle); }
        [[nodiscard]] const FRHIRenderPass*
        Get(FRHIRenderPassHandle I_Handle) const { return RenderPasses.Get(I_Handle); }
        [[nodiscard]] const FRHIComputePass*
        Get(FRHIComputePassHandle I_Handle) const { return ComputePasses.Get(I_Handle); }

        [[nodiscard]] FRWLock& GetLock() const { return RegistryLock; }

        [[nodiscard]] FRHITextureID
        Register(FRHITextureCreateInfo&& I_TextureDesc);
        [[nodiscard]] FRHIBufferID
        Register(FRHIBufferCreateInfo&& I_BufferDesc);
        [[nodiscard]] FRHISamplerID
        Register(FRHISamplerCreateInfo&& I_SamplerDesc);
        [[nodiscard]] FRHIDescriptorSetID
        Register(FRHIDescriptorSetCreateInfo&& I_DescriptorSetDesc);
        [[nodiscard]] FRHIShaderID
        Register(FRHIShaderCreateInfo&& I_ShaderDesc);
        [[nodiscard]] FRHIRenderPassID
        Register(FRHIRenderPassCreateInfo&& I_RenderPassDesc);
        [[nodiscard]] FRHIComputePassID
        Register(FRHIComputePassCreateInfo&& I_ComputePassDesc);
        void
        Unregister(FRHITextureHandle I_Handle);
        void
        Unregister(FRHIBufferHandle I_Handle);
        void
        Unregister(FRHISamplerHandle I_Handle);
        void
        Unregister(FRHIDescriptorSetHandle I_Handle);
        void
        Unregister(FRHIShaderHandle I_Handle);
        void
        Unregister(FRHIRenderPassHandle I_Handle);
        void
        Unregister(FRHIComputePassHandle I_Handle);

        /// Enqueue Unregister for RHI thread (thread-safe, callable from any thread).
        void EnqueueUnregister(FRHITextureHandle I_Handle);
        void EnqueueUnregister(FRHIBufferHandle I_Handle);
        void EnqueueUnregister(FRHISamplerHandle I_Handle);
        void EnqueueUnregister(FRHIDescriptorSetHandle I_Handle);
        void EnqueueUnregister(FRHIShaderHandle I_Handle);
        void EnqueueUnregister(FRHIRenderPassHandle I_Handle);
        void EnqueueUnregister(FRHIComputePassHandle I_Handle);

        void
        SetCurrentRetirementFence(FVulkanFence* I_Fence) { CurrentRetirementFence = I_Fence; }
        void
        CollectGarbage();
        void
        ClearGarbage();

        [[nodiscard]] FVulkanDescriptorPool&
        GetDescriptorSetPool() { return DescriptorSetPool; }
        [[nodiscard]] const FVulkanDescriptorPool&
        GetDescriptorSetPool() const { return DescriptorSetPool; }

    private:
        TSlotMap<FRHITexture,                FRHITextureHandle>             Textures;
        TSlotMap<FRHISampler,                FRHISamplerHandle>             Samplers;
        TSlotMap<FRHIBuffer,                 FRHIBufferHandle>              Buffers;
        TSlotMap<FRHIDescriptorSet,          FRHIDescriptorSetHandle>       DescriptorSets;
        TSlotMap<FRHIShader,                 FRHIShaderHandle>              Shaders;
        TSlotMap<FRHIRenderPass,             FRHIRenderPassHandle>          RenderPasses;
        TSlotMap<FRHIComputePass,            FRHIComputePassHandle>         ComputePasses;
        TSlotMap<FVulkanDescriptorSetLayout, FRHIDescriptorSetLayoutHandle> DescriptorSetLayouts;

        template<typename HandleType>
        struct FGarbageItem
        {
            HandleType       ResourceHandle {};
            FVulkanFence*    RetiredFence   {nullptr};
            UInt8            TimeToLive     {4};
        };
        /// All Vulkan-backed resources (Texture, Buffer, Sampler, DescriptorSet, Shader, RenderPass, ComputePass)
        /// must be destroyed only after the GPU has finished using them. Unregister() enqueues the handle into
        /// the corresponding GarbageBin with the current retirement fence; the RHI thread later reaps items
        /// when RetiredFence is signaled (and optional TimeToLive has elapsed), then Erase() destroys the
        /// backing Vulkan object. Do not destroy or release Vulkan objects outside this path to avoid use-after-free.
        TArray<FGarbageItem<FRHITextureHandle>>         GarbageBinTextures;
        TArray<FGarbageItem<FRHIBufferHandle>>          GarbageBinBuffers;
        TArray<FGarbageItem<FRHISamplerHandle>>         GarbageBinSamplers;
        TArray<FGarbageItem<FRHIDescriptorSetHandle>>   GarbageBinDescriptorSets;
        TArray<FGarbageItem<FRHIShaderHandle>>          GarbageBinShaders;
        TArray<FGarbageItem<FRHIRenderPassHandle>>      GarbageBinRenderPasses;
        TArray<FGarbageItem<FRHIComputePassHandle>>     GarbageBinComputePasses;

        TMap<UInt64, TArray<FRHITextureHandle>>         RecycleBinTextures;
        TMap<UInt64, TArray<FRHIBufferHandle>>          RecycleBinBuffers;
        TMap<UInt64, TArray<FRHISamplerHandle>>         RecycleBinSamplers;
        TMap<UInt64, TArray<FRHIDescriptorSetHandle>>   RecycleBinDescriptorSets;

        /// Hash(Canonicalized CreateInfo) -> candidates for hash collision (Scheme A)
        TMap<UInt64, TArray<TPair<FRHIDescriptorSetCreateInfo, FRHIDescriptorSetLayoutHandle>>> DescriptorSetLayoutCache;

        TSPSCQueue<FRHITextureHandle>        PendingUnregisterTextures;
        TSPSCQueue<FRHIBufferHandle>         PendingUnregisterBuffers;
        TSPSCQueue<FRHISamplerHandle>        PendingUnregisterSamplers;
        TSPSCQueue<FRHIDescriptorSetHandle>  PendingUnregisterDescriptorSets;
        TSPSCQueue<FRHIShaderHandle>         PendingUnregisterShaders;
        TSPSCQueue<FRHIRenderPassHandle>     PendingUnregisterRenderPasses;
        TSPSCQueue<FRHIComputePassHandle>    PendingUnregisterComputePasses;
        PROFILING_ONLY_FIELD(
        struct FProfilingMetrics
        {
            UInt64 CreatedTextures {0};
            UInt64 CreatedBuffers {0};
            UInt64 CreatedSamplers {0};
            UInt64 CreatedDescriptorSets {0};

            UInt64 ReusedTextures {0};
            UInt64 ReusedBuffers {0};
            UInt64 ReusedSamplers {0};
            UInt64 ReusedDescriptorSets {0};

            UInt64 GarbageQueuedTextures {0};
            UInt64 GarbageQueuedBuffers {0};
            UInt64 GarbageQueuedSamplers {0};
            UInt64 GarbageQueuedDescriptorSets {0};

            UInt64 GarbageRecycledTextures {0};
            UInt64 GarbageRecycledBuffers {0};
            UInt64 GarbageRecycledSamplers {0};
            UInt64 GarbageRecycledDescriptorSets {0};

            UInt64 DestroyedTextures {0};
            UInt64 DestroyedBuffers {0};
            UInt64 DestroyedSamplers {0};
            UInt64 DestroyedDescriptorSets {0};

            UInt64 PeakGarbageTextures {0};
            UInt64 PeakGarbageBuffers {0};
            UInt64 PeakGarbageSamplers {0};
            UInt64 PeakGarbageDescriptorSets {0};

            UInt64 PeakRecycleTextures {0};
            UInt64 PeakRecycleBuffers {0};
            UInt64 PeakRecycleSamplers {0};
            UInt64 PeakRecycleDescriptorSets {0};

            UInt64 CollectCalls {0};
            UInt64 ClearCalls {0};
        } ProfilingMetrics {};
        );

        mutable FRWLock        RegistryLock;
        FVulkanDriver*        Driver;
        FVulkanDescriptorPool  DescriptorSetPool;
        FVulkanFence*         CurrentRetirementFence {nullptr};

    private:
        void DrainPendingUnregisters();
        void ForceDestroyAll();
        static TArray<vk::DescriptorPoolSize>
        GetDefaultDescriptorPoolSizes();

        template<typename HandleType>
        [[nodiscard]] static UInt64
        CountRecycleHandles(const TMap<UInt64, TArray<HandleType>>& I_RecycleBin)
        {
            UInt64 Count = 0;
            for (const auto& [_, Handles] : I_RecycleBin)
            { Count += Handles.GetSize(); }
            return Count;
        }

        [[nodiscard]] UInt64
        Hash(const FRHITextureCreateInfo& I_TextureDesc) const
        {
            return Math::GoldenRatioHashCombine(0,
                I_TextureDesc.Width, I_TextureDesc.Height, I_TextureDesc.Depth,
                I_TextureDesc.Format, I_TextureDesc.Type, I_TextureDesc.Usages,
                I_TextureDesc.ViewType, I_TextureDesc.SampleCount,
                I_TextureDesc.MipLevelRange.Left,   I_TextureDesc.MipLevelRange.Right,
                I_TextureDesc.ArrayLayerRange.Left, I_TextureDesc.ArrayLayerRange.Right);
        }

        [[nodiscard]] UInt64
        Hash(const FRHIBufferCreateInfo& I_BufferDesc) const
        {
            return Math::GoldenRatioHashCombine(0,
                I_BufferDesc.Size, I_BufferDesc.Usages);
        }

        [[nodiscard]] UInt64
        Hash(const FRHISamplerCreateInfo& I_SamplerDesc) const
        {
            UInt64 Value = static_cast<UInt64>(I_SamplerDesc.Type);
            return (Value << 32) | static_cast<UInt64>(I_SamplerDesc.AddressMode);
        }

        [[nodiscard]] UInt64
        Hash(const FRHIDescriptorSetCreateInfo& I_DescriptorSetDesc) const
        {
            UInt64 H = 0;
            for (const auto& B : I_DescriptorSetDesc.Bindings)
            {
                H = Math::GoldenRatioHashCombine(H,
                    B.Binding,
                    B.Type,
                    B.Count,
                    B.Stages);
            }
            return H;
        }

    public:
        FRHIRegistry() = delete;
        FRHIRegistry(FVulkanDriver* I_Driver)
        : Driver(I_Driver)
        {
            DescriptorSetPool = Driver->CreateDescriptorPool(GetDefaultDescriptorPoolSizes());
        }
        ~FRHIRegistry()
        {
            ForceDestroyAll();
            PROFILING_ONLY_FIELD(
            LOG_INFO("[Profiling] RHI.Registry summary: created(T={},B={},S={},D={}) reused(T={},B={},S={},D={}).",
                ProfilingMetrics.CreatedTextures,
                ProfilingMetrics.CreatedBuffers,
                ProfilingMetrics.CreatedSamplers,
                ProfilingMetrics.CreatedDescriptorSets,
                ProfilingMetrics.ReusedTextures,
                ProfilingMetrics.ReusedBuffers,
                ProfilingMetrics.ReusedSamplers,
                ProfilingMetrics.ReusedDescriptorSets);
            LOG_INFO("[Profiling] RHI.Registry summary: garbage_queued(T={},B={},S={},D={}) garbage_recycled(T={},B={},S={},D={}) destroyed(T={},B={},S={},D={}).",
                ProfilingMetrics.GarbageQueuedTextures,
                ProfilingMetrics.GarbageQueuedBuffers,
                ProfilingMetrics.GarbageQueuedSamplers,
                ProfilingMetrics.GarbageQueuedDescriptorSets,
                ProfilingMetrics.GarbageRecycledTextures,
                ProfilingMetrics.GarbageRecycledBuffers,
                ProfilingMetrics.GarbageRecycledSamplers,
                ProfilingMetrics.GarbageRecycledDescriptorSets,
                ProfilingMetrics.DestroyedTextures,
                ProfilingMetrics.DestroyedBuffers,
                ProfilingMetrics.DestroyedSamplers,
                ProfilingMetrics.DestroyedDescriptorSets);
            LOG_INFO("[Profiling] RHI.Registry peaks: garbage(T={},B={},S={},D={}) recycle(T={},B={},S={},D={}) calls(collect={},clear={}).",
                ProfilingMetrics.PeakGarbageTextures,
                ProfilingMetrics.PeakGarbageBuffers,
                ProfilingMetrics.PeakGarbageSamplers,
                ProfilingMetrics.PeakGarbageDescriptorSets,
                ProfilingMetrics.PeakRecycleTextures,
                ProfilingMetrics.PeakRecycleBuffers,
                ProfilingMetrics.PeakRecycleSamplers,
                ProfilingMetrics.PeakRecycleDescriptorSets,
                ProfilingMetrics.CollectCalls,
                ProfilingMetrics.ClearCalls);
            );
        }
    };

    inline TArray<vk::DescriptorPoolSize> FRHIRegistry::
    GetDefaultDescriptorPoolSizes()
    {
        return TArray<vk::DescriptorPoolSize>{
            { vk::DescriptorType::eSampledImage,         1024 },
            { vk::DescriptorType::eUniformBuffer,         512 },
            { vk::DescriptorType::eStorageBuffer,         128 },
            { vk::DescriptorType::eSampler,               256 },
        };
    }

    template<Concepts::RHIHandle RHIHandle>
    void TRHIRegistryEntry<RHIHandle>::Release()
    {
        if (!Block) { return; }
        if (Block->RefCount.FetchSub(1, EMemoryOrder::AcqRel) == 1)
        {
            TWeakPtr<FRHIRegistry> RegWeak = Block->Registry;
            auto                   Handle  = Block->Handle;
            delete Block;
            Block = nullptr;
            if (RegWeak.IsExpired())
            {
                DEBUG_ONLY_FIELD(
                // SwapChain proxy textures are CreateUnmanaged(Gen=0); they are not in the registry, so do not report as leak.
                const Bool bSwapChainProxy = (Handle.GetType() == FRHIResourceHandle::EType::Texture && Handle.GetGeneration() == 0);
                if (!bSwapChainProxy)
                { LOG_ERROR("App did not release RHI resources({}) before engine shutdown.", Handle); }
                );  
                return;
            }
            if (auto Reg = RegWeak.Lock()) { Reg->EnqueueUnregister(Handle); }
        }
        else { Block = nullptr; }
    }

    template<Concepts::RHIHandle RHIHandle>
    TRHIRegistryEntry<RHIHandle>::TRHIRegistryEntry(TWeakPtr<FRHIRegistry> I_Registry, RHIHandle I_Handle)
        : Block(new FControlBlock{std::move(I_Registry), I_Handle, 1})
    {}

    template<Concepts::RHIHandle RHIHandle>
    TRHIRegistryEntry<RHIHandle>::TRHIRegistryEntry(const TRHIRegistryEntry& I_Other) : Block(I_Other.Block)
    {
        if (Block) { Block->RefCount.FetchAdd(1, EMemoryOrder::Relaxed); }
    }

    template<Concepts::RHIHandle RHIHandle>
    TRHIRegistryEntry<RHIHandle>::TRHIRegistryEntry(TRHIRegistryEntry&& I_Other) noexcept : Block(I_Other.Block)
    {
        I_Other.Block = nullptr;
    }

    template<Concepts::RHIHandle RHIHandle>
    TRHIRegistryEntry<RHIHandle>& TRHIRegistryEntry<RHIHandle>::operator=(const TRHIRegistryEntry& I_Other)
    {
        if (this == &I_Other) { return *this; }
        Release();
        Block = I_Other.Block;
        if (Block) { Block->RefCount.FetchAdd(1, EMemoryOrder::Relaxed); }
        return *this;
    }

    template<Concepts::RHIHandle RHIHandle>
    TRHIRegistryEntry<RHIHandle>& TRHIRegistryEntry<RHIHandle>::operator=(TRHIRegistryEntry&& I_Other) noexcept
    {
        if (this == &I_Other) { return *this; }
        Release();
        Block = I_Other.Block;
        I_Other.Block = nullptr;
        return *this;
    }

    template<Concepts::RHIHandle RHIHandle>
    TRHIRegistryEntry<RHIHandle>::~TRHIRegistryEntry()
    {
        Release();
    }

    FRHISamplerID FRHIRegistry::
    Register(FRHISamplerCreateInfo&& I_SamplerDesc)
    {
        FScopeWriteLock WriteLock(&RegistryLock);
        const UInt64 Key = Hash(I_SamplerDesc);

        auto RecycleBinIter = RecycleBinSamplers.Find(Key);
        if (RecycleBinIter != RecycleBinSamplers.end())
        {
            auto& Handles = RecycleBinIter->second;

            for (UInt32 Idx = 0; Idx < Handles.GetSize(); ++Idx)
            {
                const FRHISamplerHandle Handle  = Handles[Idx];
                const auto*            Sampler = Samplers.Get(Handle);
                if (Sampler == nullptr) { continue; }

                if (Sampler->GetInfo().IsCompatibleWith(I_SamplerDesc))
                {
                    Handles.RemoveAtSwap(Idx);
                    PROFILING_ONLY_FIELD(++ProfilingMetrics.ReusedSamplers;);
                    return TRHIRegistryEntry<FRHISamplerHandle>(SharedFromThis(), Handle);
                }
            }
        }
        // Create new resource
        FVulkanSampler   Sampler = Driver->CreateImageSampler(
            TypeCast(I_SamplerDesc.Type),
            TypeCast(I_SamplerDesc.AddressMode));
        FRHISamplerHandle Handle = Samplers.Insert(
            FRHISampler{std::move(I_SamplerDesc), std::move(Sampler)},
            False);
        PROFILING_ONLY_FIELD(++ProfilingMetrics.CreatedSamplers;);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return TRHIRegistryEntry<FRHISamplerHandle>(SharedFromThis(), Handle);
    }

    FRHITextureID FRHIRegistry::
    Register(FRHITextureCreateInfo&& I_TextureDesc)
    {
        FScopeWriteLock WriteLock(&RegistryLock);
        const UInt64 Key = Hash(I_TextureDesc);

        auto RecycleBinIter = RecycleBinTextures.Find(Key);
        if (RecycleBinIter != RecycleBinTextures.end())
        {
            auto& Handles = RecycleBinIter->second;

            for (UInt32 Idx = 0; Idx < Handles.GetSize(); ++Idx)
            {
                const FRHITextureHandle Handle  = Handles[Idx];
                const auto*            Texture = Textures.Get(Handle);
                if (Texture == nullptr) { continue; }

                if (Texture->GetInfo().IsCompatibleWith(I_TextureDesc))
                {
                    Handles.RemoveAtSwap(Idx);
                    PROFILING_ONLY_FIELD(++ProfilingMetrics.ReusedTextures;);
                    return TRHIRegistryEntry<FRHITextureHandle>(SharedFromThis(), Handle);
                }
            }
        }
        // Create new resource
        const vk::ImageCreateInfo ImageCreateInfo = vk::ImageCreateInfo{}
            .setImageType   (TypeCast(I_TextureDesc.Type))
            .setFormat      (TypeCast(I_TextureDesc.Format))
            .setUsage       (TypeCast(I_TextureDesc.Usages))
            .setExtent      ({I_TextureDesc.Width, I_TextureDesc.Height, I_TextureDesc.Depth})
            .setMipLevels   (I_TextureDesc.MipLevelRange.Length() + 1)
            .setArrayLayers (I_TextureDesc.ArrayLayerRange.Length() + 1)
            .setSamples     (TypeCast(I_TextureDesc.SampleCount))
            .setTiling      (vk::ImageTiling::eOptimal)
            .setSharingMode (vk::SharingMode::eExclusive)
        ;

        FVulkanImage     Image     = Driver->CreateImage(ImageCreateInfo, EVulkanMemoryProperty::Aliasable);
        FVulkanImageView ImageView = Driver->CreateImageView(&Image,
            TypeCast(I_TextureDesc.ViewType),
            vk::ImageAspectFlagBits::eColor,
            I_TextureDesc.MipLevelRange,
            I_TextureDesc.ArrayLayerRange);
        FRHITextureHandle Handle = Textures.Insert(
            FRHITexture{std::move(I_TextureDesc), std::move(Image), std::move(ImageView)},
            True);
        PROFILING_ONLY_FIELD(++ProfilingMetrics.CreatedTextures;);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return TRHIRegistryEntry<FRHITextureHandle>(SharedFromThis(), Handle);
    }

    FRHIBufferID FRHIRegistry::
    Register(FRHIBufferCreateInfo&& I_BufferDesc)
    {
        FScopeWriteLock WriteLock(&RegistryLock);
        const UInt64 Key = Hash(I_BufferDesc);

        auto RecycleBinIter = RecycleBinBuffers.Find(Key);
        if (RecycleBinIter != RecycleBinBuffers.end())
        {
            auto& Handles = RecycleBinIter->second;

            for (UInt32 Idx = 0; Idx < Handles.GetSize(); ++Idx)
            {
                const FRHIBufferHandle Handle = Handles[Idx];
                const FRHIBuffer*      Buffer = Buffers.Get(Handle);
                if (Buffer == nullptr) { continue; }

                if (Buffer->GetInfo().IsCompatibleWith(I_BufferDesc))
                {
                    Handles.RemoveAtSwap(Idx);
                    PROFILING_ONLY_FIELD(++ProfilingMetrics.ReusedBuffers;);
                    return TRHIRegistryEntry<FRHIBufferHandle>(SharedFromThis(), Handle);
                }
            }
        }
        // Create new resource
        vk::BufferCreateInfo       BufferCreateInfo = vk::BufferCreateInfo{}
            .setSize        (I_BufferDesc.Size)
            .setUsage       (TypeCast(I_BufferDesc.Usages))
            .setSharingMode (vk::SharingMode::eExclusive)
        ;
        EVulkanMemoryProperty MemoryProperties = EVulkanMemoryProperty::None;
        if (I_BufferDesc.Usages & ERHIBufferUsage::TransferSrc)
        {
            MemoryProperties |= EVulkanMemoryProperty::HostAccessAllowTransferInstead |
                                EVulkanMemoryProperty::HostAccessSequentialWrite;
        }
        if ((I_BufferDesc.Usages & ERHIBufferUsage::TransferDst) && (I_BufferDesc.Usages & ~ERHIBufferUsage::TransferDst) == ERHIBufferUsage::None)
        {
            // Readback buffer: host-visible and mapped so CPU can read after GPU copy. VMA requires a host-access flag with Mapped.
            MemoryProperties |= EVulkanMemoryProperty::Mapped | EVulkanMemoryProperty::HostAccessRandom;
        }

        FVulkanBuffer     Buffer = Driver->CreateBuffer(BufferCreateInfo, MemoryProperties);
        FRHIBufferHandle  Handle = Buffers.Insert(
            FRHIBuffer{std::move(I_BufferDesc), std::move(Buffer)},
            True);
        PROFILING_ONLY_FIELD(++ProfilingMetrics.CreatedBuffers;);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return TRHIRegistryEntry<FRHIBufferHandle>(SharedFromThis(), Handle);
    }

    FRHIDescriptorSetID FRHIRegistry::
    Register(FRHIDescriptorSetCreateInfo&& I_DescriptorSetDesc)
    {
        FScopeWriteLock WriteLock(&RegistryLock);
        VISERA_ASSERT(!I_DescriptorSetDesc.Bindings.IsEmpty());
        // Canonicalize: sort bindings by Binding index so that Hash and IsCompatibleWith are order-independent
        Algorithm::Sort(I_DescriptorSetDesc.Bindings,
            [](const auto& A, const auto& B) { return A.Binding < B.Binding; });
        const UInt64 Key = Hash(I_DescriptorSetDesc);

        auto RecycleBinIter = RecycleBinDescriptorSets.Find(Key);
        if (RecycleBinIter != RecycleBinDescriptorSets.end())
        {
            auto& CandidateHandles = RecycleBinIter->second;
            for (UInt32 Idx = 0; Idx < CandidateHandles.GetSize(); ++Idx)
            {
                const FRHIDescriptorSetHandle Hdl = CandidateHandles[Idx];
                const auto* DS = DescriptorSets.Get(Hdl);
                if (DS && DS->GetInfo().IsCompatibleWith(I_DescriptorSetDesc))
                {
                    CandidateHandles.RemoveAtSwap(Idx);
                    PROFILING_ONLY_FIELD(++ProfilingMetrics.ReusedDescriptorSets;);
                    return TRHIRegistryEntry<FRHIDescriptorSetHandle>(SharedFromThis(), Hdl);
                }
            }
        }

        FRHIDescriptorSetLayoutHandle LayoutHandle;
        auto LayoutCacheIter = DescriptorSetLayoutCache.Find(Key);
        Bool bFoundInCache = False;
        if (LayoutCacheIter != DescriptorSetLayoutCache.end())
        {
            for (const auto& [CachedInfo, CachedHdl] : LayoutCacheIter->second)
            {
                if (CachedInfo.IsCompatibleWith(I_DescriptorSetDesc))
                {
                    LayoutHandle = CachedHdl;
                    bFoundInCache = True;
                    break;
                }
            }
        }

        if (!bFoundInCache)
        {
            TArray<vk::DescriptorSetLayoutBinding> VulkanBindings;
            VulkanBindings.Reserve(I_DescriptorSetDesc.Bindings.GetSize());
            for (const auto& Binding : I_DescriptorSetDesc.Bindings)
            {
                VulkanBindings.EmplaceBack(vk::DescriptorSetLayoutBinding{}
                    .setBinding         (static_cast<UInt32>(Binding.Binding))
                    .setDescriptorType  (TypeCast(Binding.Type))
                    .setDescriptorCount (Binding.Count)
                    .setStageFlags      (TypeCast(Binding.Stages))
                );
            }
            FVulkanDescriptorSetLayout Layout =
                Driver->CreateDescriptorSetLayout(VulkanBindings);
            LayoutHandle = DescriptorSetLayouts.Insert(std::move(Layout), False);
            DescriptorSetLayoutCache[Key].EmplaceBack(MakePair(FRHIDescriptorSetCreateInfo{I_DescriptorSetDesc}, LayoutHandle));
        }

        FVulkanDescriptorSetLayout* LayoutPtr = DescriptorSetLayouts.Get(LayoutHandle);
        VISERA_ASSERT(LayoutPtr != nullptr);
        FVulkanDescriptorSet VulkanDescriptorSet = DescriptorSetPool.CreateDescriptorSet(*LayoutPtr);
        FRHIDescriptorSetHandle Handle = DescriptorSets.Insert(
            FRHIDescriptorSet{std::move(I_DescriptorSetDesc), std::move(VulkanDescriptorSet)},
            False);
        PROFILING_ONLY_FIELD(++ProfilingMetrics.CreatedDescriptorSets;);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return TRHIRegistryEntry<FRHIDescriptorSetHandle>(SharedFromThis(), Handle);
    }

    FRHIShaderID FRHIRegistry::
    Register(FRHIShaderCreateInfo&& I_ShaderDesc)
    {
        FScopeWriteLock WriteLock(&RegistryLock);
        VISERA_ASSERT(!I_ShaderDesc.SPIRV.IsEmpty());
        FVulkanShaderModule ShaderModule = Driver->CreateShaderModule(I_ShaderDesc.SPIRV);
        FRHIShaderHandle Handle = Shaders.Insert(
            FRHIShader{std::move(I_ShaderDesc), std::move(ShaderModule)});
        LOG_DEBUG("Registered Shader ({}).", Handle);
        return TRHIRegistryEntry<FRHIShaderHandle>(SharedFromThis(), Handle);
    }

    FRHIRenderPassID FRHIRegistry::
    Register(FRHIRenderPassCreateInfo&& I_Info)
    {
        FScopeWriteLock WriteLock(&RegistryLock);
        FRHIShader* pVS = Get(I_Info.VertexShader);
        FRHIShader* pFS = Get(I_Info.FragmentShader);
        VISERA_ASSERT(pVS != nullptr && pFS != nullptr);

        const FRHIShaderLayout& VSRefl = pVS->GetLayout();
        const FRHIShaderLayout& FSRefl = pFS->GetLayout();

        TMap<UInt32, TArray<vk::DescriptorSetLayoutBinding>> SetToBindings;
        for (const auto& R : VSRefl.Resources)
        {
            auto& Binds = SetToBindings[R.Set];
            Binds.EmplaceBack(vk::DescriptorSetLayoutBinding{}
                .setBinding        (R.Binding)
                .setDescriptorType (TypeCast(R.Type))
                .setDescriptorCount(R.ArrayCount)
                .setStageFlags     (TypeCast(R.Stages)));
        }
        for (const auto& R : FSRefl.Resources)
        {
            auto& Binds = SetToBindings[R.Set];
            auto It = Algorithm::FindIf(Binds, [&R](const vk::DescriptorSetLayoutBinding& B) { return B.binding == R.Binding; });
            if (It != Binds.end())
            { It->stageFlags |= TypeCast(R.Stages); }
            else
            {
                Binds.EmplaceBack(vk::DescriptorSetLayoutBinding{}
                    .setBinding        (R.Binding)
                    .setDescriptorType (TypeCast(R.Type))
                    .setDescriptorCount(R.ArrayCount)
                    .setStageFlags     (TypeCast(R.Stages)));
            }
        }

        TArray<UInt32> SetIndices;
        for (const auto& [SetIdx, _] : SetToBindings)
        { SetIndices.PushBack(SetIdx); }
        Algorithm::Sort(SetIndices);

        TArray<vk::DescriptorSetLayout> DSLHandles;
        TArray<FVulkanDescriptorSetLayout> DSLStorage;
        for (UInt32 SetIdx : SetIndices)
        {
            auto& Binds = SetToBindings[SetIdx];
            Algorithm::Sort(Binds, [](const auto& A, const auto& B) { return A.binding < B.binding; });
            DSLStorage.PushBack(Driver->CreateDescriptorSetLayout(Binds));
            DSLHandles.PushBack(DSLStorage.Back().GetHandle());
        }

        TArray<vk::PushConstantRange> PCRanges;
        UInt32 Offset = 0;
        for (const auto& PC : VSRefl.PushConstants)
        {
            if (PC.Size > 0)
            {
                PCRanges.EmplaceBack(vk::PushConstantRange{}
                    .setOffset    (Offset)
                    .setSize     (PC.Size)
                    .setStageFlags(TypeCast(PC.Stages)));
                Offset += PC.Size;
            }
        }
        for (const auto& PC : FSRefl.PushConstants)
        {
            if (PC.Size > 0)
            {
                PCRanges.EmplaceBack(vk::PushConstantRange{}
                    .setOffset    (Offset)
                    .setSize     (PC.Size)
                    .setStageFlags(TypeCast(PC.Stages)));
                Offset += PC.Size;
            }
        }

        auto PipelineLayout   = Driver->CreatePipelineLayout(DSLHandles, PCRanges);
        auto VertShaderModule = Driver->CreateShaderModule(pVS->GetInfo().SPIRV);
        auto FragShaderModule = Driver->CreateShaderModule(pFS->GetInfo().SPIRV);

        TArray<vk::Format> ColorFormats;
        ColorFormats.Reserve(I_Info.PSO.ColorFormats.GetSize());
        for (ERHIFormat F : I_Info.PSO.ColorFormats)
        { ColorFormats.PushBack(TypeCast(F)); }
        if (ColorFormats.IsEmpty())
        { ColorFormats.PushBack(vk::Format::eB8G8R8A8Srgb); }
        FVulkanRenderPipeline Pipeline = Driver->CreateRenderPipeline(
            &PipelineLayout,
            &VertShaderModule,
            &FragShaderModule,
            ColorFormats,
            TypeCast(I_Info.PSO.DepthStencilFormat));

        FRHIRenderPassHandle Handle = RenderPasses.Insert(
            FRHIRenderPass{std::move(I_Info), std::move(Pipeline)});
        LOG_DEBUG("Registered RenderPass ({}).", Handle);
        return TRHIRegistryEntry<FRHIRenderPassHandle>(SharedFromThis(), Handle);
    }

    FRHIComputePassID FRHIRegistry::
    Register(FRHIComputePassCreateInfo&& I_Info)
    {
        FScopeWriteLock WriteLock(&RegistryLock);
        FRHIShader* pCS = Get(I_Info.ComputeShader);
        VISERA_ASSERT(pCS != nullptr);

        const FRHIShaderLayout& CSRefl = pCS->GetLayout();

        TMap<UInt32, TArray<vk::DescriptorSetLayoutBinding>> SetToBindings;
        for (const auto& R : CSRefl.Resources)
        {
            auto& Binds = SetToBindings[R.Set];
            Binds.EmplaceBack(vk::DescriptorSetLayoutBinding{}
                .setBinding        (R.Binding)
                .setDescriptorType (TypeCast(R.Type))
                .setDescriptorCount(R.ArrayCount)
                .setStageFlags     (TypeCast(R.Stages)));
        }

        TArray<UInt32> SetIndices;
        for (const auto& [SetIdx, _] : SetToBindings)
        { SetIndices.PushBack(SetIdx); }
        Algorithm::Sort(SetIndices);

        TArray<vk::DescriptorSetLayout> DSLHandles;
        TArray<FVulkanDescriptorSetLayout> DSLStorage;
        for (UInt32 SetIdx : SetIndices)
        {
            auto& Binds = SetToBindings[SetIdx];
            Algorithm::Sort(Binds, [](const auto& A, const auto& B) { return A.binding < B.binding; });
            DSLStorage.PushBack(Driver->CreateDescriptorSetLayout(Binds));
            DSLHandles.PushBack(DSLStorage.Back().GetHandle());
        }

        TArray<vk::PushConstantRange> PCRanges;
        UInt32 Offset = 0;
        for (const auto& PC : CSRefl.PushConstants)
        {
            if (PC.Size > 0)
            {
                PCRanges.EmplaceBack(vk::PushConstantRange{}
                    .setOffset    (Offset)
                    .setSize     (PC.Size)
                    .setStageFlags(TypeCast(PC.Stages)));
                Offset += PC.Size;
            }
        }

        auto PipelineLayout    = Driver->CreatePipelineLayout(DSLHandles, PCRanges);
        auto ComputeShaderModule = Driver->CreateShaderModule(pCS->GetInfo().SPIRV);

        FVulkanComputePipeline Pipeline = Driver->CreateComputePipeline(
            &PipelineLayout,
            &ComputeShaderModule);

        FRHIComputePassHandle Handle = ComputePasses.Insert(
            FRHIComputePass{std::move(I_Info), std::move(Pipeline)});
        LOG_DEBUG("Registered ComputePass ({}).", Handle);
        return TRHIRegistryEntry<FRHIComputePassHandle>(SharedFromThis(), Handle);
    }

    void FRHIRegistry::
    Unregister(FRHITextureHandle I_Handle)
    {
        GarbageBinTextures.PushBack({.ResourceHandle = I_Handle, .RetiredFence = CurrentRetirementFence});
        PROFILING_ONLY_FIELD(
        ++ProfilingMetrics.GarbageQueuedTextures;
        if (GarbageBinTextures.GetSize() > ProfilingMetrics.PeakGarbageTextures) { ProfilingMetrics.PeakGarbageTextures = GarbageBinTextures.GetSize(); }
        );
    }

    void FRHIRegistry::
    Unregister(FRHIBufferHandle I_Handle)
    {
        GarbageBinBuffers.PushBack({.ResourceHandle = I_Handle, .RetiredFence = CurrentRetirementFence});
        PROFILING_ONLY_FIELD(
        ++ProfilingMetrics.GarbageQueuedBuffers;
        if (GarbageBinBuffers.GetSize() > ProfilingMetrics.PeakGarbageBuffers) { ProfilingMetrics.PeakGarbageBuffers = GarbageBinBuffers.GetSize(); }
        );
    }

    void FRHIRegistry::
    Unregister(FRHISamplerHandle I_Handle)
    {
        GarbageBinSamplers.PushBack({.ResourceHandle = I_Handle, .RetiredFence = CurrentRetirementFence});
        PROFILING_ONLY_FIELD(
        ++ProfilingMetrics.GarbageQueuedSamplers;
        if (GarbageBinSamplers.GetSize() > ProfilingMetrics.PeakGarbageSamplers) { ProfilingMetrics.PeakGarbageSamplers = GarbageBinSamplers.GetSize(); }
        );
    }

    void FRHIRegistry::
    Unregister(FRHIDescriptorSetHandle I_Handle)
    {
        GarbageBinDescriptorSets.PushBack({.ResourceHandle = I_Handle, .RetiredFence = CurrentRetirementFence});
        PROFILING_ONLY_FIELD(
        ++ProfilingMetrics.GarbageQueuedDescriptorSets;
        if (GarbageBinDescriptorSets.GetSize() > ProfilingMetrics.PeakGarbageDescriptorSets) { ProfilingMetrics.PeakGarbageDescriptorSets = GarbageBinDescriptorSets.GetSize(); }
        );
    }

    void FRHIRegistry::
    Unregister(FRHIShaderHandle I_Handle)
    {
        GarbageBinShaders.PushBack({.ResourceHandle = I_Handle, .RetiredFence = CurrentRetirementFence});
    }

    void FRHIRegistry::
    Unregister(FRHIRenderPassHandle I_Handle)
    {
        GarbageBinRenderPasses.PushBack({.ResourceHandle = I_Handle, .RetiredFence = CurrentRetirementFence});
    }

    void FRHIRegistry::
    Unregister(FRHIComputePassHandle I_Handle)
    {
        GarbageBinComputePasses.PushBack({.ResourceHandle = I_Handle, .RetiredFence = CurrentRetirementFence});
    }

    void FRHIRegistry::
    EnqueueUnregister(FRHITextureHandle I_Handle)
    {
        PendingUnregisterTextures.Enqueue(I_Handle);
    }

    void FRHIRegistry::
    EnqueueUnregister(FRHIBufferHandle I_Handle)
    {
        PendingUnregisterBuffers.Enqueue(I_Handle);
    }

    void FRHIRegistry::
    EnqueueUnregister(FRHISamplerHandle I_Handle)
    {
        PendingUnregisterSamplers.Enqueue(I_Handle);
    }

    void FRHIRegistry::
    EnqueueUnregister(FRHIDescriptorSetHandle I_Handle)
    {
        PendingUnregisterDescriptorSets.Enqueue(I_Handle);
    }

    void FRHIRegistry::
    EnqueueUnregister(FRHIShaderHandle I_Handle)
    {
        PendingUnregisterShaders.Enqueue(I_Handle);
    }

    void FRHIRegistry::
    EnqueueUnregister(FRHIRenderPassHandle I_Handle)
    {
        PendingUnregisterRenderPasses.Enqueue(I_Handle);
    }

    void FRHIRegistry::
    EnqueueUnregister(FRHIComputePassHandle I_Handle)
    {
        PendingUnregisterComputePasses.Enqueue(I_Handle);
    }

    void FRHIRegistry::
    ForceDestroyAll()
    {
        FScopeWriteLock WriteLock(&RegistryLock);
        LOG_DEBUG("FRHIRegistry::ForceDestroyAll: destroying all resources.");
        DrainPendingUnregisters();

        for (auto& CurrentItem : GarbageBinTextures)
        {
            const auto* Texture = Textures.Get(CurrentItem.ResourceHandle);
            if (Texture) { RecycleBinTextures[Hash(Texture->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle); }
        }
        GarbageBinTextures.Clear();
        for (auto& CurrentItem : GarbageBinBuffers)
        {
            const auto* Buffer = Buffers.Get(CurrentItem.ResourceHandle);
            if (Buffer) { RecycleBinBuffers[Hash(Buffer->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle); }
        }
        GarbageBinBuffers.Clear();
        for (auto& CurrentItem : GarbageBinSamplers)
        {
            const auto* Sampler = Samplers.Get(CurrentItem.ResourceHandle);
            if (Sampler) { RecycleBinSamplers[Hash(Sampler->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle); }
        }
        GarbageBinSamplers.Clear();
        for (auto& CurrentItem : GarbageBinDescriptorSets)
        {
            const auto* DescriptorSet = DescriptorSets.Get(CurrentItem.ResourceHandle);
            if (DescriptorSet) { RecycleBinDescriptorSets[Hash(DescriptorSet->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle); }
        }
        GarbageBinDescriptorSets.Clear();
        for (auto& CurrentItem : GarbageBinShaders)
        {
            (void)Shaders.Erase(CurrentItem.ResourceHandle);
        }
        GarbageBinShaders.Clear();
        for (auto& CurrentItem : GarbageBinRenderPasses)
        {
            (void)RenderPasses.Erase(CurrentItem.ResourceHandle);
        }
        GarbageBinRenderPasses.Clear();
        for (auto& CurrentItem : GarbageBinComputePasses)
        {
            (void)ComputePasses.Erase(CurrentItem.ResourceHandle);
        }
        GarbageBinComputePasses.Clear();

        ClearGarbage();

        DescriptorSets.Clear();
        DescriptorSetLayouts.Clear();
        Textures.Clear();
        Buffers.Clear();
        Samplers.Clear();
        Shaders.Clear();
        RenderPasses.Clear();
        ComputePasses.Clear();
        DescriptorSetLayoutCache.Clear();
    }

    void FRHIRegistry::
    DrainPendingUnregisters()
    {
        while (auto H = PendingUnregisterTextures.Dequeue())
        { Unregister(*H); }
        while (auto H = PendingUnregisterBuffers.Dequeue())
        { Unregister(*H); }
        while (auto H = PendingUnregisterSamplers.Dequeue())
        { Unregister(*H); }
        while (auto H = PendingUnregisterDescriptorSets.Dequeue())
        { Unregister(*H); }
        while (auto H = PendingUnregisterShaders.Dequeue())
        { Unregister(*H); }
        while (auto H = PendingUnregisterRenderPasses.Dequeue())
        { Unregister(*H); }
        while (auto H = PendingUnregisterComputePasses.Dequeue())
        { Unregister(*H); }
    }

    /**
     * Call this function at the BEGIN of the frame (after Wait on SubmitFence)
     */
    void FRHIRegistry::
    CollectGarbage()
    {
        FScopeWriteLock WriteLock(&RegistryLock);
        DrainPendingUnregisters();
        PROFILING_ONLY_FIELD(
        ++ProfilingMetrics.CollectCalls;
        UInt64 RecycledTextures = 0;
        UInt64 RecycledBuffers = 0;
        UInt64 RecycledSamplers = 0;
        UInt64 RecycledDescriptorSets = 0;
        );
        for (UInt32 Idx = 0; Idx < GarbageBinTextures.GetSize();)
        {
            auto& CurrentItem = GarbageBinTextures[Idx];
            if (CurrentItem.RetiredFence && !CurrentItem.RetiredFence->IsSignaled())
            {
                Idx += 1;
                continue;
            }
            if (CurrentItem.TimeToLive > 0)
            {
                --CurrentItem.TimeToLive;
                Idx += 1;
                continue;
            }
            const auto* Texture = Textures.Get(CurrentItem.ResourceHandle);
            VISERA_ASSERT(Texture != nullptr);
            RecycleBinTextures[Hash(Texture->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle);
            GarbageBinTextures.RemoveAtSwap(Idx);
            PROFILING_ONLY_FIELD(++RecycledTextures;);
        }
        for (UInt32 Idx = 0; Idx < GarbageBinBuffers.GetSize();)
        {
            auto& CurrentItem = GarbageBinBuffers[Idx];
            if (CurrentItem.RetiredFence && !CurrentItem.RetiredFence->IsSignaled())
            {
                Idx += 1;
                continue;
            }
            if (CurrentItem.TimeToLive > 0)
            {
                --CurrentItem.TimeToLive;
                Idx += 1;
                continue;
            }
            const auto* Buffer = Buffers.Get(CurrentItem.ResourceHandle);
            VISERA_ASSERT(Buffer != nullptr);
            RecycleBinBuffers[Hash(Buffer->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle);
            GarbageBinBuffers.RemoveAtSwap(Idx);
            PROFILING_ONLY_FIELD(++RecycledBuffers;);
        }
        for (UInt32 Idx = 0; Idx < GarbageBinSamplers.GetSize();)
        {
            auto& CurrentItem = GarbageBinSamplers[Idx];
            if (CurrentItem.RetiredFence && !CurrentItem.RetiredFence->IsSignaled())
            {
                Idx += 1;
                continue;
            }
            if (CurrentItem.TimeToLive > 0)
            {
                --CurrentItem.TimeToLive;
                Idx += 1;
                continue;
            }
            const auto* Sampler = Samplers.Get(CurrentItem.ResourceHandle);
            VISERA_ASSERT(Sampler != nullptr);
            RecycleBinSamplers[Hash(Sampler->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle);
            GarbageBinSamplers.RemoveAtSwap(Idx);
            PROFILING_ONLY_FIELD(++RecycledSamplers;);
        }
        for (UInt32 Idx = 0; Idx < GarbageBinDescriptorSets.GetSize();)
        {
            auto& CurrentItem = GarbageBinDescriptorSets[Idx];
            if (CurrentItem.RetiredFence && !CurrentItem.RetiredFence->IsSignaled())
            {
                Idx += 1;
                continue;
            }
            if (CurrentItem.TimeToLive > 0)
            {
                --CurrentItem.TimeToLive;
                Idx += 1;
                continue;
            }
            const auto* DescriptorSet = DescriptorSets.Get(CurrentItem.ResourceHandle);
            VISERA_ASSERT(DescriptorSet != nullptr);
            RecycleBinDescriptorSets[Hash(DescriptorSet->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle);
            GarbageBinDescriptorSets.RemoveAtSwap(Idx);
            PROFILING_ONLY_FIELD(++RecycledDescriptorSets;);
        }
        for (UInt32 Idx = 0; Idx < GarbageBinShaders.GetSize();)
        {
            auto& CurrentItem = GarbageBinShaders[Idx];
            if (CurrentItem.RetiredFence && !CurrentItem.RetiredFence->IsSignaled())
            {
                Idx += 1;
                continue;
            }
            if (CurrentItem.TimeToLive > 0)
            {
                --CurrentItem.TimeToLive;
                Idx += 1;
                continue;
            }
            if (Shaders.Erase(CurrentItem.ResourceHandle))
            { LOG_DEBUG("Destroyed Shader ({}).", CurrentItem.ResourceHandle); }
            GarbageBinShaders.RemoveAtSwap(Idx);
        }
        for (UInt32 Idx = 0; Idx < GarbageBinRenderPasses.GetSize();)
        {
            auto& CurrentItem = GarbageBinRenderPasses[Idx];
            if (CurrentItem.RetiredFence && !CurrentItem.RetiredFence->IsSignaled())
            {
                Idx += 1;
                continue;
            }
            if (CurrentItem.TimeToLive > 0)
            {
                --CurrentItem.TimeToLive;
                Idx += 1;
                continue;
            }
            if (RenderPasses.Erase(CurrentItem.ResourceHandle))
            { LOG_DEBUG("Destroyed RenderPass ({}).", CurrentItem.ResourceHandle); }
            GarbageBinRenderPasses.RemoveAtSwap(Idx);
        }
        for (UInt32 Idx = 0; Idx < GarbageBinComputePasses.GetSize();)
        {
            auto& CurrentItem = GarbageBinComputePasses[Idx];
            if (CurrentItem.RetiredFence && !CurrentItem.RetiredFence->IsSignaled())
            {
                Idx += 1;
                continue;
            }
            if (CurrentItem.TimeToLive > 0)
            {
                --CurrentItem.TimeToLive;
                Idx += 1;
                continue;
            }
            if (ComputePasses.Erase(CurrentItem.ResourceHandle))
            { LOG_DEBUG("Destroyed ComputePass ({}).", CurrentItem.ResourceHandle); }
            GarbageBinComputePasses.RemoveAtSwap(Idx);
        }
        PROFILING_ONLY_FIELD(
        ProfilingMetrics.GarbageRecycledTextures += RecycledTextures;
        ProfilingMetrics.GarbageRecycledBuffers += RecycledBuffers;
        ProfilingMetrics.GarbageRecycledSamplers += RecycledSamplers;
        ProfilingMetrics.GarbageRecycledDescriptorSets += RecycledDescriptorSets;
        const UInt64 RecycleTexturesCount = CountRecycleHandles(RecycleBinTextures);
        const UInt64 RecycleBuffersCount = CountRecycleHandles(RecycleBinBuffers);
        const UInt64 RecycleSamplersCount = CountRecycleHandles(RecycleBinSamplers);
        const UInt64 RecycleDescriptorSetsCount = CountRecycleHandles(RecycleBinDescriptorSets);
        if (RecycleTexturesCount > ProfilingMetrics.PeakRecycleTextures) { ProfilingMetrics.PeakRecycleTextures = RecycleTexturesCount; }
        if (RecycleBuffersCount > ProfilingMetrics.PeakRecycleBuffers) { ProfilingMetrics.PeakRecycleBuffers = RecycleBuffersCount; }
        if (RecycleSamplersCount > ProfilingMetrics.PeakRecycleSamplers) { ProfilingMetrics.PeakRecycleSamplers = RecycleSamplersCount; }
        if (RecycleDescriptorSetsCount > ProfilingMetrics.PeakRecycleDescriptorSets) { ProfilingMetrics.PeakRecycleDescriptorSets = RecycleDescriptorSetsCount; }
        );
    }

    /**
     * Call this function at the END of the frame
     */
    void FRHIRegistry::
    ClearGarbage()
    {
        // No lock here: only called from ForceDestroyAll() which already holds RegistryLock.
        PROFILING_ONLY_FIELD(++ProfilingMetrics.ClearCalls;);
        for (auto& [_, Handles] : RecycleBinTextures)
        {
            for (auto Handle : Handles)
            {
                if (!Textures.Erase(Handle))
                { LOG_ERROR("Failed to erase the texture (handle:{})!", Handle); }
                else
                {
                    LOG_DEBUG("Destroyed a resource ({}).", Handle);
                    PROFILING_ONLY_FIELD(++ProfilingMetrics.DestroyedTextures;);
                }
            }
        }
        RecycleBinTextures.Clear();
        for (auto& [_, Handles] : RecycleBinBuffers)
        {
            for (auto Handle : Handles)
            {
                if (!Buffers.Erase(Handle))
                { LOG_ERROR("Failed to erase the buffer (handle:{})!", Handle); }
                else
                {
                    LOG_DEBUG("Destroyed a resource ({}).", Handle);
                    PROFILING_ONLY_FIELD(++ProfilingMetrics.DestroyedBuffers;);
                }
            }
        }
        RecycleBinBuffers.Clear();
        for (auto& [_, Handles] : RecycleBinSamplers)
        {
            for (auto Handle : Handles)
            {
                if (!Samplers.Erase(Handle))
                { LOG_ERROR("Failed to erase the sampler (handle:{})!", Handle); }
                else
                {
                    LOG_DEBUG("Destroyed a resource ({}).", Handle);
                    PROFILING_ONLY_FIELD(++ProfilingMetrics.DestroyedSamplers;);
                }
            }
        }
        RecycleBinSamplers.Clear();
        for (auto& [_, Handles] : RecycleBinDescriptorSets)
        {
            for (auto Handle : Handles)
            {
                if (!DescriptorSets.Erase(Handle))
                { LOG_ERROR("Failed to erase the descriptor set (handle:{})!", Handle); }
                else
                {
                    LOG_DEBUG("Destroyed a resource ({}).", Handle);
                    PROFILING_ONLY_FIELD(++ProfilingMetrics.DestroyedDescriptorSets;);
                }
            }
        }
        RecycleBinDescriptorSets.Clear();
    }
}
