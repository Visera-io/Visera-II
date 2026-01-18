module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Registry;
#define VISERA_MODULE_NAME "RHI.Registry"
import Visera.RHI.Common;
import Visera.RHI.Vulkan;
import Visera.RHI.Resource;
import Visera.Core.Types.SlotMap;
import Visera.Core.Types.Map;
import Visera.Core.Types.Array;
import Visera.Core.Math.Hash.GoldenRatio;
import Visera.Global.Log;
import vulkan_hpp;

export namespace Visera
{
    class VISERA_RHI_API FRHIRegistry
    {
    public:
        [[nodiscard]] FRHITexture*
        GetTexture(FRHIResourceHandle I_Handle) { VISERA_ASSERT(I_Handle.GetType() == ERHIResourceType::Texture); return Textures.Get(I_Handle); }
        [[nodiscard]] FRHIBuffer*
        GetBuffer(FRHIResourceHandle I_Handle) { VISERA_ASSERT(I_Handle.GetType() == ERHIResourceType::Buffer); return Buffers.Get(I_Handle); }
        [[nodiscard]] FRHIResourceHandle
        Register(FRHITextureCreateDesc&& I_TextureDesc);
        [[nodiscard]] FRHIResourceHandle
        Register(FRHIBufferCreateDesc&& I_BufferDesc);
        void
        Unregister(FRHIResourceHandle I_Handle, UInt8 I_RetiredFrame);

        void
        CollectGarbage(UInt8 I_FrameIndex);
        void
        ClearGarbage();

    private:
        TSlotMap<FRHITexture,   FRHIResourceHandle> Textures;
        TSlotMap<FRHISampler,   FRHIResourceHandle> Samplers;
        TSlotMap<FRHIBuffer,    FRHIResourceHandle> Buffers;

        struct FGarbageItem
        {
            FRHIResourceHandle ResourceHandle { };
            UInt8              RetiredFrame   {0};
        };
        TArray<FGarbageItem>                     GarbageBin;
        TMap<UInt64, TArray<FRHIResourceHandle>> RecycleBin; // Resource may revive

        TUniqueRef<FVulkanDriver>  Driver;

    private:
        [[nodiscard]] UInt64
        Hash(const FRHITextureCreateDesc& I_TextureDesc) const
        {
            return Math::GoldenRatioHashCombine(0,
                I_TextureDesc.Width, I_TextureDesc.Height, I_TextureDesc.Depth,
                I_TextureDesc.Format, I_TextureDesc.Type, I_TextureDesc.Usages,
                I_TextureDesc.ViewType, I_TextureDesc.SampleCount,
                I_TextureDesc.MipLevelRange.Left,   I_TextureDesc.MipLevelRange.Right,
                I_TextureDesc.ArrayLayerRange.Left, I_TextureDesc.ArrayLayerRange.Right);
        }

        [[nodiscard]] UInt64
        Hash(const FRHIBufferCreateDesc& I_BufferDesc) const
        {
            return Math::GoldenRatioHashCombine(0,
                I_BufferDesc.Size, I_BufferDesc.Usages);
        }

    public:
        FRHIRegistry() = delete;
        FRHIRegistry(TUniqueRef<FVulkanDriver> I_Driver)
        : Driver(I_Driver)
        {

        }
        ~FRHIRegistry()
        {

        }
    };

    FRHIResourceHandle FRHIRegistry::
    Register(FRHITextureCreateDesc&& I_TextureDesc)
    {
        const UInt64 Key = Hash(I_TextureDesc);

        auto RecycleBinIter = RecycleBin.Find(Key);
        if (RecycleBinIter != RecycleBin.end())
        {
            auto& Handles = RecycleBinIter->second;

            for (UInt32 Idx = 0; Idx < Handles.GetSize(); ++Idx)
            {
                const auto Handle = Handles[Idx];
                if (Handle.GetType() != ERHIResourceType::Texture) { continue; }

                const auto Texture = Textures.Get(Handle);
                if (Texture == nullptr) { continue; }

                if (Texture->GetInfo().IsCompatibleWith(I_TextureDesc))
                {
                    Handles.RemoveAtSwap(Idx);
                    // if (Handles.IsEmpty())
                    // { RecycleBin.Erase(Key); }
                    return Handle;
                }
            }
        }
        // Create new resource
        const auto ImageCreateInfo = vk::ImageCreateInfo{}
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

        auto Image = Driver->CreateImage(ImageCreateInfo, EVulkanMemoryProperty::Aliasable);
        auto ImageView = Driver->CreateImageView(&Image,
            TypeCast(I_TextureDesc.ViewType),
            vk::ImageAspectFlagBits::eColor,
            I_TextureDesc.MipLevelRange,
            I_TextureDesc.ArrayLayerRange);

        auto Handle = Textures.Insert(
            FRHITexture{std::move(I_TextureDesc), std::move(Image), std::move(ImageView)},
            ERHIResourceType::Texture, True);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return Handle;
    }

    FRHIResourceHandle FRHIRegistry::
    Register(FRHIBufferCreateDesc&& I_BufferDesc)
    {
        const UInt64 Key = Hash(I_BufferDesc);

        auto RecycleBinIter = RecycleBin.Find(Key);
        if (RecycleBinIter != RecycleBin.end())
        {
            auto& Handles = RecycleBinIter->second;

            for (UInt32 Idx = 0; Idx < Handles.GetSize(); ++Idx)
            {
                const auto Handle = Handles[Idx];
                if (Handle.GetType() != ERHIResourceType::Buffer) { continue; }

                const auto Buffer = Buffers.Get(Handle);
                if (Buffer == nullptr) { continue; }

                if (Buffer->GetInfo().IsCompatibleWith(I_BufferDesc))
                {
                    Handles.RemoveAtSwap(Idx);
                    // if (Handles.IsEmpty())
                    // { RecycleBin.Erase(Key); }
                    return Handle;
                }
            }
        }
        // Create new resource
        auto BufferCreateInfo = vk::BufferCreateInfo{}
            .setSize        (I_BufferDesc.Size)
            .setUsage       (TypeCast(I_BufferDesc.Usages))
            .setSharingMode (vk::SharingMode::eExclusive)
        ;

        auto MemoryProperties = EVulkanMemoryProperty::None;
        if (I_BufferDesc.Usages & ERHIBufferUsage::TransferSrc)
        {
            MemoryProperties |= EVulkanMemoryProperty::HostAccessAllowTransferInstead |
                                EVulkanMemoryProperty::HostAccessSequentialWrite;
        }

        auto Buffer = Driver->CreateBuffer(BufferCreateInfo, MemoryProperties);

        auto Handle = Buffers.Insert(
            FRHIBuffer{std::move(I_BufferDesc), std::move(Buffer)},
            ERHIResourceType::Buffer, True);

        LOG_DEBUG("Created a new resource ({}).", Handle);
        return Handle;
    }

    void FRHIRegistry::
    Unregister(FRHIResourceHandle I_Handle, UInt8 I_RetiredFrame)
    {
        GarbageBin.PushBack({
            .ResourceHandle = I_Handle,
            .RetiredFrame = I_RetiredFrame});
    }

    /**
     * Call this function at the BEGIN of the frame
     */
    void FRHIRegistry::
    CollectGarbage(UInt8 I_FrameIndex)
    {
        for (UInt32 Idx = 0; Idx < GarbageBin.GetSize();)
        {
            auto& CurrentItem = GarbageBin[Idx];
            if (CurrentItem.RetiredFrame != I_FrameIndex) // Per-Frame Fence mode
            {
                Idx += 1; // Next Idx
                continue;
            }
            // Move the resource to the Recycle Bin.
            auto Type   = CurrentItem.ResourceHandle.GetType();
            auto Handle = CurrentItem.ResourceHandle;
            switch (Type)
            {
            case ERHIResourceType::Texture:
                {
                    const auto Texture = Textures.Get(CurrentItem.ResourceHandle);
                    VISERA_ASSERT(Texture != nullptr);
                    RecycleBin[Hash(Texture->GetInfo())].EmplaceBack(Handle);
                } break;
            case ERHIResourceType::Sampler:
                {
                    VISERA_UNIMPLEMENTED_API;
                } break;
            case ERHIResourceType::Buffer:
                {
                    const auto Buffer = Buffers.Get(CurrentItem.ResourceHandle);
                    VISERA_ASSERT(Buffer != nullptr);
                    RecycleBin[Hash(Buffer->GetInfo())].EmplaceBack(Handle);
                } break;
            default:
                LOG_ERROR("Unknown Resource Type (handle:{})", CurrentItem.ResourceHandle);
            }
            GarbageBin.RemoveAtSwap(Idx);
        }
    }

    /**
     * Call this function at the END of the frame
     */
    void FRHIRegistry::
    ClearGarbage()
    {
        if (RecycleBin.IsEmpty()) { return; }

        for (auto& Handles : RecycleBin | std::views::values)
        {
            for (auto Handle : Handles)
            {
                switch (Handle.GetType())
                {
                case ERHIResourceType::Texture:
                    {
                        if (!Textures.Erase(Handle))
                        { LOG_ERROR("Failed to erase the texture (handle:{})!", Handle); }
                    } break;
                case ERHIResourceType::Sampler:
                    {
                        if (!Samplers.Erase(Handle))
                        { LOG_ERROR("Failed to erase the sampler (handle:{})!", Handle); }
                    } break;
                case ERHIResourceType::Buffer:
                    {
                        if (!Buffers.Erase(Handle))
                        { LOG_ERROR("Failed to erase the buffer (handle:{})!", Handle); }
                    } break;
                default:
                    LOG_ERROR("Unknown Resource Type (handle:{})", Handle);
                }
                LOG_DEBUG("Destroyed a resource ({}).", Handle);
            }
        }
        RecycleBin.Clear();
    }
}
