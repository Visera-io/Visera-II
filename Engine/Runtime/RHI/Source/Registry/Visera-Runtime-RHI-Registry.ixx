module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Registry;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Registry.ID;
       import Visera.Runtime.RHI.Common;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Runtime.RHI.Resource;
       import Visera.Core.Types.SlotMap;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Array;
       import Visera.Core.Math.Hash.GoldenRatio;
       import Visera.Core.Log;
       import vulkan_hpp;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHIRegistry
    {
    public:
        template<Concepts::RHIID RHIID>
        class TEntry
        {
        public:

        private:
            FRHIRegistry* Registry {nullptr};
            RHIID         ID       {};

        public:
            TEntry() = delete;
            TEntry(FRHIRegistry& I_Registry, RHIID I_ID) : Registry { &I_Registry }, ID { I_ID } {}
        };
        static_assert(sizeof(TEntry<FRHIResourceID>) == 16);

    public:
        [[nodiscard]] FRHITexture*
        Get(FRHITextureID I_Handle) { return Textures.Get(I_Handle); }
        [[nodiscard]] FRHIBuffer*
        Get(FRHIBufferID I_Handle) { return Buffers.Get(I_Handle); }
        [[nodiscard]] FRHISampler*
        Get(FRHISamplerID I_Handle) { return Samplers.Get(I_Handle); }
        [[nodiscard]] FRHIDescriptorSet*
        Get(FRHIDescriptorSetID I_Handle) { return DescriptorSets.Get(I_Handle); }
        [[nodiscard]] const FRHITexture*
        Get(FRHITextureID I_Handle) const { return Textures.Get(I_Handle); }
        [[nodiscard]] const FRHIBuffer*
        Get(FRHIBufferID I_Handle) const { return Buffers.Get(I_Handle); }
        [[nodiscard]] const FRHISampler*
        Get(FRHISamplerID I_Handle) const { return Samplers.Get(I_Handle); }
        [[nodiscard]] const FRHIDescriptorSet*
        Get(FRHIDescriptorSetID I_Handle) const { return DescriptorSets.Get(I_Handle); }
        [[nodiscard]] FRHITextureID
        Register(FRHITextureCreateInfo&& I_TextureDesc);
        [[nodiscard]] FRHIBufferID
        Register(FRHIBufferCreateInfo&& I_BufferDesc);
        [[nodiscard]] FRHISamplerID
        Register(FRHISamplerCreateInfo&& I_SamplerDesc);
        [[nodiscard]] FRHIDescriptorSetID
        Register(FRHIDescriptorSetCreateInfo&& I_DescriptorSetDesc);
        void
        Unregister(FRHITextureID I_Handle, UInt8 I_RetiredFrame);
        void
        Unregister(FRHIBufferID I_Handle, UInt8 I_RetiredFrame);
        void
        Unregister(FRHISamplerID I_Handle, UInt8 I_RetiredFrame);
        void
        Unregister(FRHIDescriptorSetID I_Handle, UInt8 I_RetiredFrame);

        void
        CollectGarbage(UInt8 I_FrameIndex);
        void
        ClearGarbage();

        [[nodiscard]] FVulkanDescriptorPool&
        GetDescriptorSetPool() { return DescriptorSetPool; }
        [[nodiscard]] const FVulkanDescriptorPool&
        GetDescriptorSetPool() const { return DescriptorSetPool; }

        // New APIs
        template<Concepts::RHIID RHIID> void
        Acquire(RHIID I_ID);

    private:
        TSlotMap<FRHITexture,                FRHITextureID>             Textures;
        TSlotMap<FRHISampler,                FRHISamplerID>             Samplers;
        TSlotMap<FRHIBuffer,                 FRHIBufferID>              Buffers;
        TSlotMap<FRHIDescriptorSet,          FRHIDescriptorSetID>       DescriptorSets;
        TSlotMap<FVulkanDescriptorSetLayout, FRHIDescriptorSetLayoutID> DescriptorSetLayouts;


        template<typename HandleType>
        struct FGarbageItem
        {
            HandleType ResourceHandle { };
            UInt8      RetiredFrame   {0};
        };
        TArray<FGarbageItem<FRHITextureID>>         GarbageBinTextures;
        TArray<FGarbageItem<FRHIBufferID>>          GarbageBinBuffers;
        TArray<FGarbageItem<FRHISamplerID>>         GarbageBinSamplers;
        TArray<FGarbageItem<FRHIDescriptorSetID>>   GarbageBinDescriptorSets;

        TMap<UInt64, TArray<FRHITextureID>>         RecycleBinTextures;
        TMap<UInt64, TArray<FRHIBufferID>>          RecycleBinBuffers;
        TMap<UInt64, TArray<FRHISamplerID>>         RecycleBinSamplers;
        TMap<UInt64, TArray<FRHIDescriptorSetID>>   RecycleBinDescriptorSets;

        FVulkanDriver*        Driver;
        FVulkanDescriptorPool DescriptorSetPool;

    private:
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
                    B.binding,
                    B.descriptorType,
                    B.descriptorCount,
                    B.stageFlags);
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
                const FRHISamplerID Handle  = Handles[Idx];
                const auto*            Sampler = Samplers.Get(Handle);
                if (Sampler == nullptr) { continue; }

                if (Sampler->GetInfo().IsCompatibleWith(I_SamplerDesc))
                {
                    Handles.RemoveAtSwap(Idx);
                    return Handle;
                }
            }
        }
        // Create new resource
        FVulkanSampler   Sampler = Driver->CreateImageSampler(
            TypeCast(I_SamplerDesc.Type),
            TypeCast(I_SamplerDesc.AddressMode));
        FRHISamplerID Handle = Samplers.Insert(
            FRHISampler{std::move(I_SamplerDesc), std::move(Sampler)},
            False);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return Handle;
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
                const FRHITextureID Handle  = Handles[Idx];
                const auto*            Texture = Textures.Get(Handle);
                if (Texture == nullptr) { continue; }

                if (Texture->GetInfo().IsCompatibleWith(I_TextureDesc))
                {
                    Handles.RemoveAtSwap(Idx);
                    return Handle;
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
        FRHITextureID Handle = Textures.Insert(
            FRHITexture{std::move(I_TextureDesc), std::move(Image), std::move(ImageView)},
            True);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return Handle;
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
                const FRHIBufferID Handle = Handles[Idx];
                const FRHIBuffer*      Buffer = Buffers.Get(Handle);
                if (Buffer == nullptr) { continue; }

                if (Buffer->GetInfo().IsCompatibleWith(I_BufferDesc))
                {
                    Handles.RemoveAtSwap(Idx);
                    return Handle;
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
        FRHIBufferID  Handle = Buffers.Insert(
            FRHIBuffer{std::move(I_BufferDesc), std::move(Buffer)},
            True);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return Handle;
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
                const FRHIDescriptorSetID Handle = Handles[Idx];
                const auto* DescriptorSet = DescriptorSets.Get(Handle);
                if (DescriptorSet == nullptr) { continue; }

                if (DescriptorSet->GetInfo().IsCompatibleWith(I_DescriptorSetDesc))
                {
                    Handles.RemoveAtSwap(Idx);
                    return Handle;
                }
            }
        }
        // Convert FRHIDescriptorSetLayoutBinding to vk::DescriptorSetLayoutBinding
        TArray<vk::DescriptorSetLayoutBinding> VulkanBindings;
        VulkanBindings.Reserve(I_DescriptorSetDesc.Bindings.GetSize());
        for (const auto& Binding : I_DescriptorSetDesc.Bindings)
        {
            VulkanBindings.EmplaceBack(
                vk::DescriptorSetLayoutBinding{}
                    .setBinding(static_cast<UInt32>(Binding.binding))
                    .setDescriptorType(TypeCast(Binding.descriptorType))
                    .setDescriptorCount(Binding.descriptorCount)
                    .setStageFlags(TypeCast(Binding.stageFlags))
            );
        }
        FVulkanDescriptorSetLayout Layout =
            Driver->CreateInforiptorSetLayout(VulkanBindings);
        FRHIDescriptorSetLayoutID LayoutHandle =
            DescriptorSetLayouts.Insert(std::move(Layout), False);
        FVulkanDescriptorSetLayout* LayoutPtr = DescriptorSetLayouts.Get(LayoutHandle);
        VISERA_ASSERT(LayoutPtr != nullptr);
        FVulkanDescriptorSet VulkanDescriptorSet = DescriptorSetPool.CreateInforiptorSet(*LayoutPtr);
        FRHIDescriptorSetID Handle = DescriptorSets.Insert(
            FRHIDescriptorSet{std::move(I_DescriptorSetDesc), std::move(VulkanDescriptorSet)},
            False);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return Handle;
    }

    void FRHIRegistry::
    Unregister(FRHITextureID I_Handle, UInt8 I_RetiredFrame)
    {
        GarbageBinTextures.PushBack({ .ResourceHandle = I_Handle, .RetiredFrame = I_RetiredFrame });
    }

    void FRHIRegistry::
    Unregister(FRHIBufferID I_Handle, UInt8 I_RetiredFrame)
    {
        GarbageBinBuffers.PushBack({ .ResourceHandle = I_Handle, .RetiredFrame = I_RetiredFrame });
    }

    void FRHIRegistry::
    Unregister(FRHISamplerID I_Handle, UInt8 I_RetiredFrame)
    {
        GarbageBinSamplers.PushBack({ .ResourceHandle = I_Handle, .RetiredFrame = I_RetiredFrame });
    }

    void FRHIRegistry::
    Unregister(FRHIDescriptorSetID I_Handle, UInt8 I_RetiredFrame)
    {
        GarbageBinDescriptorSets.PushBack({ .ResourceHandle = I_Handle, .RetiredFrame = I_RetiredFrame });
    }

    /**
     * Call this function at the BEGIN of the frame
     */
    void FRHIRegistry::
    CollectGarbage(UInt8 I_FrameIndex)
    {
        for (UInt32 Idx = 0; Idx < GarbageBinTextures.GetSize();)
        {
            auto& CurrentItem = GarbageBinTextures[Idx];
            if (CurrentItem.RetiredFrame != I_FrameIndex)
            {
                Idx += 1;
                continue;
            }
            const auto* Texture = Textures.Get(CurrentItem.ResourceHandle);
            VISERA_ASSERT(Texture != nullptr);
            RecycleBinTextures[Hash(Texture->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle);
            GarbageBinTextures.RemoveAtSwap(Idx);
        }
        for (UInt32 Idx = 0; Idx < GarbageBinBuffers.GetSize();)
        {
            auto& CurrentItem = GarbageBinBuffers[Idx];
            if (CurrentItem.RetiredFrame != I_FrameIndex)
            {
                Idx += 1;
                continue;
            }
            const auto* Buffer = Buffers.Get(CurrentItem.ResourceHandle);
            VISERA_ASSERT(Buffer != nullptr);
            RecycleBinBuffers[Hash(Buffer->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle);
            GarbageBinBuffers.RemoveAtSwap(Idx);
        }
        for (UInt32 Idx = 0; Idx < GarbageBinSamplers.GetSize();)
        {
            auto& CurrentItem = GarbageBinSamplers[Idx];
            if (CurrentItem.RetiredFrame != I_FrameIndex)
            {
                Idx += 1;
                continue;
            }
            const auto* Sampler = Samplers.Get(CurrentItem.ResourceHandle);
            VISERA_ASSERT(Sampler != nullptr);
            RecycleBinSamplers[Hash(Sampler->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle);
            GarbageBinSamplers.RemoveAtSwap(Idx);
        }
        for (UInt32 Idx = 0; Idx < GarbageBinDescriptorSets.GetSize();)
        {
            auto& CurrentItem = GarbageBinDescriptorSets[Idx];
            if (CurrentItem.RetiredFrame != I_FrameIndex)
            {
                Idx += 1;
                continue;
            }
            const auto* DescriptorSet = DescriptorSets.Get(CurrentItem.ResourceHandle);
            VISERA_ASSERT(DescriptorSet != nullptr);
            RecycleBinDescriptorSets[Hash(DescriptorSet->GetInfo())].EmplaceBack(CurrentItem.ResourceHandle);
            GarbageBinDescriptorSets.RemoveAtSwap(Idx);
        }
    }

    /**
     * Call this function at the END of the frame
     */
    void FRHIRegistry::
    ClearGarbage()
    {
        for (auto& [_, Handles] : RecycleBinTextures)
        {
            for (auto Handle : Handles)
            {
                if (!Textures.Erase(Handle))
                { LOG_ERROR("Failed to erase the texture (handle:{})!", Handle); }
                else
                { LOG_DEBUG("Destroyed a resource ({}).", Handle); }
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
                { LOG_DEBUG("Destroyed a resource ({}).", Handle); }
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
                { LOG_DEBUG("Destroyed a resource ({}).", Handle); }
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
                { LOG_DEBUG("Destroyed a resource ({}).", Handle); }
            }
        }
        RecycleBinDescriptorSets.Clear();
    }
}
