module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Registry;
#define VISERA_MODULE_NAME "RHI.Registry"
import Visera.RHI.Common;
import Visera.RHI.Vulkan;
import Visera.RHI.Resource;
import Visera.Core.Types.SlotMap;
import Visera.Core.Types.Array;
import Visera.Global.Log;

export namespace Visera
{
    using FRHISampler       = FVulkanSampler;
    using FRHIBuffer        = FVulkanBuffer;
    // Note: FRHICommandBuffer is now a template type, use FVulkanCommandBuffer<QueueFamily> directly

    class VISERA_RHI_API FRHIRegistry
    {
    public:
        [[nodiscard]] FRHIResourceHandle
        Register(const FRHITextureCreateDesc& I_ImageDesc);

        void
        ClearRecycleBin();

    private:
        TSlotMap<FRHITexture,   FRHIResourceHandle> Textures;
        TSlotMap<FRHISampler,   FRHIResourceHandle> Samplers;
        TSlotMap<FRHIBuffer,    FRHIResourceHandle> Buffers;

        TArray<FRHIResourceHandle> RecycleBin;

        TUniqueRef<FVulkanDriver>  Driver;
        TArray<FVulkanCommandBuffer<EVulkanQueueFamily::Transfer>>  TransferCommandBuffers;

    public:
        FRHIRegistry() = delete;
        FRHIRegistry(TUniqueRef<FVulkanDriver> I_Driver)
        : Driver(I_Driver)
        {
            TransferCommandBuffers.resize(Driver->GetSwapChain().Images.size());
            for (auto& TransferCommandBuffer : TransferCommandBuffers)
            {
                //TransferCommandBuffer = Driver->CreateCommandBuffer(EVulkanQueue::Transfer);
            }
        }
    };

    FRHIResourceHandle FRHIRegistry::
    Register(const FRHITextureCreateDesc& I_ImageDesc)
    {
        auto& Desc = I_ImageDesc;
        VISERA_UNIMPLEMENTED_API;
        return {};
    }

    void FRHIRegistry::
    ClearRecycleBin()
    {
        for (auto Resource : RecycleBin)
        {
            auto Type = Resource.GetType();
            switch (Type)
            {
            case ERHIResourceType::Texture:
                {

                } break;
            case ERHIResourceType::Sampler:
                {

                } break;
            case ERHIResourceType::Buffer:
                {

                } break;
            default:
                LOG_ERROR("Unknown Resource Type : {} (handle:{})", Type, Resource);
            }
        }
    }
}
