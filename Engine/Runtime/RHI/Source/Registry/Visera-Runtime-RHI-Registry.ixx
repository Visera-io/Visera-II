module;
#include <Visera-RHI.hpp>
#include <atomic>
export module Visera.Runtime.RHI.Registry;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Registry.Handle;
       import Visera.Runtime.RHI.Common;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.RHI.Resource;
       import Visera.Core.Containers.SlotMap;
       import Visera.Core.Containers.Map;
       import Visera.Core.Containers.Array;
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
            FRHIRegistry*        Registry  {nullptr};
            RHIHandle            Handle    {};
            std::atomic<UInt32>  RefCount  {0};
        };

    public:
        [[nodiscard]] RHIHandle
        GetHandle() const { return Block ? Block->Handle : RHIHandle{}; }

        TRHIRegistryEntry() = delete;
        TRHIRegistryEntry(FRHIRegistry& I_Registry, RHIHandle I_Handle);
        TRHIRegistryEntry(const TRHIRegistryEntry& I_Other);
        TRHIRegistryEntry(TRHIRegistryEntry&& I_Other) noexcept;
        TRHIRegistryEntry& operator=(const TRHIRegistryEntry& I_Other);
        TRHIRegistryEntry& operator=(TRHIRegistryEntry&& I_Other) noexcept;
        ~TRHIRegistryEntry();

    private:
        FControlBlock* Block {nullptr};
        void Release();
    };

    using FRHITextureID       = TRHIRegistryEntry<FRHITextureHandle>;
    using FRHISamplerID       = TRHIRegistryEntry<FRHISamplerHandle>;
    using FRHIBufferID        = TRHIRegistryEntry<FRHIBufferHandle>;
    using FRHIDescriptorSetID = TRHIRegistryEntry<FRHIDescriptorSetHandle>;

    class VISERA_RUNTIME_API FRHIRegistry
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
        [[nodiscard]] const FRHITexture*
        Get(FRHITextureHandle I_Handle) const { return Textures.Get(I_Handle); }
        [[nodiscard]] const FRHIBuffer*
        Get(FRHIBufferHandle I_Handle) const { return Buffers.Get(I_Handle); }
        [[nodiscard]] const FRHISampler*
        Get(FRHISamplerHandle I_Handle) const { return Samplers.Get(I_Handle); }
        [[nodiscard]] const FRHIDescriptorSet*
        Get(FRHIDescriptorSetHandle I_Handle) const { return DescriptorSets.Get(I_Handle); }
        [[nodiscard]] FRHITextureID
        Register(FRHITextureCreateInfo&& I_TextureDesc);
        [[nodiscard]] FRHIBufferID
        Register(FRHIBufferCreateInfo&& I_BufferDesc);
        [[nodiscard]] FRHISamplerID
        Register(FRHISamplerCreateInfo&& I_SamplerDesc);
        [[nodiscard]] FRHIDescriptorSetID
        Register(FRHIDescriptorSetCreateInfo&& I_DescriptorSetDesc);
        void
        Unregister(FRHITextureHandle I_Handle);
        void
        Unregister(FRHIBufferHandle I_Handle);
        void
        Unregister(FRHISamplerHandle I_Handle);
        void
        Unregister(FRHIDescriptorSetHandle I_Handle);

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
        TSlotMap<FVulkanDescriptorSetLayout, FRHIDescriptorSetLayoutHandle> DescriptorSetLayouts;


        template<typename HandleType>
        struct FGarbageItem
        {
            HandleType       ResourceHandle {};
            FVulkanFence*    RetiredFence   {nullptr};
        };
        TArray<FGarbageItem<FRHITextureHandle>>         GarbageBinTextures;
        TArray<FGarbageItem<FRHIBufferHandle>>          GarbageBinBuffers;
        TArray<FGarbageItem<FRHISamplerHandle>>         GarbageBinSamplers;
        TArray<FGarbageItem<FRHIDescriptorSetHandle>>   GarbageBinDescriptorSets;

        TMap<UInt64, TArray<FRHITextureHandle>>         RecycleBinTextures;
        TMap<UInt64, TArray<FRHIBufferHandle>>          RecycleBinBuffers;
        TMap<UInt64, TArray<FRHISamplerHandle>>         RecycleBinSamplers;
        TMap<UInt64, TArray<FRHIDescriptorSetHandle>>   RecycleBinDescriptorSets;
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

        FVulkanDriver*        Driver;
        FVulkanDescriptorPool  DescriptorSetPool;
        FVulkanFence*         CurrentRetirementFence {nullptr};

    private:
        template<typename HandleType>
        [[nodiscard]] static UInt64
        CountRecycleHandles(const TMap<UInt64, TArray<HandleType>>& I_RecycleBin)
        {
            UInt64 Count = 0;
            for (const auto& [_, Handles] : I_RecycleBin)
            { Count += Handles.GetSize(); }
            return Count;
        }

        static TArray<vk::DescriptorPoolSize>
        GetDefaultDescriptorPoolSizes();

    private:
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
            DescriptorSetPool = Driver->CreateInforiptorPool(GetDefaultDescriptorPoolSizes());
        }
        ~FRHIRegistry()
        {
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
        if (Block->RefCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            auto* Reg = Block->Registry;
            auto  Hdl = Block->Handle;
            delete Block;
            Block = nullptr;
            if (Reg) { Reg->Unregister(Hdl); }
        }
        else
        {
            Block = nullptr;
        }
    }

    template<Concepts::RHIHandle RHIHandle>
    TRHIRegistryEntry<RHIHandle>::TRHIRegistryEntry(FRHIRegistry& I_Registry, RHIHandle I_Handle)
        : Block(new FControlBlock{&I_Registry, I_Handle, 1})
    {}

    template<Concepts::RHIHandle RHIHandle>
    TRHIRegistryEntry<RHIHandle>::TRHIRegistryEntry(const TRHIRegistryEntry& I_Other) : Block(I_Other.Block)
    {
        if (Block) { Block->RefCount.fetch_add(1, std::memory_order_relaxed); }
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
        if (Block) { Block->RefCount.fetch_add(1, std::memory_order_relaxed); }
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
                    return TRHIRegistryEntry<FRHISamplerHandle>(*this, Handle);
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
        return TRHIRegistryEntry<FRHISamplerHandle>(*this, Handle);
    }

    FRHITextureID FRHIRegistry::
    Register(FRHITextureCreateInfo&& I_TextureDesc)
    {
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
                    return TRHIRegistryEntry<FRHITextureHandle>(*this, Handle);
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
        return TRHIRegistryEntry<FRHITextureHandle>(*this, Handle);
    }

    FRHIBufferID FRHIRegistry::
    Register(FRHIBufferCreateInfo&& I_BufferDesc)
    {
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
                    return TRHIRegistryEntry<FRHIBufferHandle>(*this, Handle);
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

        FVulkanBuffer     Buffer = Driver->CreateBuffer(BufferCreateInfo, MemoryProperties);
        FRHIBufferHandle  Handle = Buffers.Insert(
            FRHIBuffer{std::move(I_BufferDesc), std::move(Buffer)},
            True);
        PROFILING_ONLY_FIELD(++ProfilingMetrics.CreatedBuffers;);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return TRHIRegistryEntry<FRHIBufferHandle>(*this, Handle);
    }

    FRHIDescriptorSetID FRHIRegistry::
    Register(FRHIDescriptorSetCreateInfo&& I_DescriptorSetDesc)
    {
        VISERA_ASSERT(!I_DescriptorSetDesc.Bindings.IsEmpty());
        const UInt64 Key = Hash(I_DescriptorSetDesc);

        auto RecycleBinIter = RecycleBinDescriptorSets.Find(Key);
        if (RecycleBinIter != RecycleBinDescriptorSets.end())
        {
            auto& Handles = RecycleBinIter->second;

            for (UInt32 Idx = 0; Idx < Handles.GetSize(); ++Idx)
            {
                const FRHIDescriptorSetHandle Handle = Handles[Idx];
                const auto* DescriptorSet = DescriptorSets.Get(Handle);
                if (DescriptorSet == nullptr) { continue; }

                if (DescriptorSet->GetInfo().IsCompatibleWith(I_DescriptorSetDesc))
                {
                    Handles.RemoveAtSwap(Idx);
                    PROFILING_ONLY_FIELD(++ProfilingMetrics.ReusedDescriptorSets;);
                    return TRHIRegistryEntry<FRHIDescriptorSetHandle>(*this, Handle);
                }
            }
        }
        // Convert FRHIDescriptorSetLayoutBinding to vk::DescriptorSetLayoutBinding
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
            Driver->CreateInforiptorSetLayout(VulkanBindings);
        FRHIDescriptorSetLayoutHandle LayoutHandle =
            DescriptorSetLayouts.Insert(std::move(Layout), False);
        FVulkanDescriptorSetLayout* LayoutPtr = DescriptorSetLayouts.Get(LayoutHandle);
        VISERA_ASSERT(LayoutPtr != nullptr);
        FVulkanDescriptorSet VulkanDescriptorSet = DescriptorSetPool.CreateInforiptorSet(*LayoutPtr);
        FRHIDescriptorSetHandle Handle = DescriptorSets.Insert(
            FRHIDescriptorSet{std::move(I_DescriptorSetDesc), std::move(VulkanDescriptorSet)},
            False);
        PROFILING_ONLY_FIELD(++ProfilingMetrics.CreatedDescriptorSets;);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return TRHIRegistryEntry<FRHIDescriptorSetHandle>(*this, Handle);
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

    /**
     * Call this function at the BEGIN of the frame (after Wait on SubmitFence)
     */
    void FRHIRegistry::
    CollectGarbage()
    {
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
            const auto* DescriptorSet = DescriptorSets.Get(CurrentItem.ResourceHandle);
            VISERA_ASSERT(DescriptorSet != nullptr);
            RecycleBinDescriptorSets[Hash(DescriptorSet->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle);
            GarbageBinDescriptorSets.RemoveAtSwap(Idx);
            PROFILING_ONLY_FIELD(++RecycledDescriptorSets;);
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
