module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Vulkan.Pipeline.Cache;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.OS.Memory;
import Visera.Core.Types.Optional;
import Visera.Core.Containers.Array;
import Visera.Core.Log;
import Visera.Platform;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkanPipelineCache
    {
    public:
        [[nodiscard]] inline const vk::raii::PipelineCache&
        GetHandle() const { return Handle; }

    private:
        vk::raii::PipelineCache Handle {nullptr};
        FPath Path;

    public:
        FVulkanPipelineCache() = delete;
        FVulkanPipelineCache(const vk::raii::PhysicalDevice& I_GPU,
                             const vk::raii::Device&         I_Device,
                             const FPath&                    I_Path);
        ~FVulkanPipelineCache();
    };

    FVulkanPipelineCache::
    FVulkanPipelineCache(const vk::raii::PhysicalDevice& I_GPU,
                         const vk::raii::Device&         I_Device,
                         const FPath&                    I_Path)
    : Path{ I_Path }
    {
        TArray<FByte> CacheData;
        if (auto Data = FPlatform::ReadFile(Path); Data.HasValue() && !Data->IsEmpty())
        {
            CacheData = std::move(Data.GetValue());

            Bool bExpired = CacheData.GetSize() < sizeof(vk::PipelineCacheHeaderVersionOne);
            if (!bExpired)
            {
                auto* CacheHeader = reinterpret_cast<vk::PipelineCacheHeaderVersionOne*>(CacheData.Data());
                auto  GPUProperties = I_GPU.getProperties();

                bExpired = CacheHeader->deviceID != GPUProperties.deviceID ||
                           CacheHeader->vendorID != GPUProperties.vendorID ||
                           Memory::Memcmp(CacheHeader->pipelineCacheUUID,
                                          GPUProperties.pipelineCacheUUID,
                                          vk::UuidSize) != 0;
            }

            if (bExpired)
            {
                LOG_DEBUG("Vulkan Pipeline Cache expired!");
                CacheData.Clear();
            }
        }

        auto CreateInfo = vk::PipelineCacheCreateInfo()
            .setInitialDataSize(CacheData.GetSize())
            .setPInitialData   (CacheData.Data());
        auto Result = I_Device.createPipelineCache(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create the Vulkan Pipeline Cache from {}!", Path); }
        else
        { Handle = std::move(*Result); }

        LOG_DEBUG("Loaded Vulkan Pipeline Cache (bytes:{}) from {}.", CacheData.GetSize(), Path);
    }

    FVulkanPipelineCache::
    ~FVulkanPipelineCache()
    {
        if (auto Result = Handle.getData(); Result.has_value())
        {
            auto& CacheData = *Result;
            LOG_DEBUG("Caching Vulkan Pipeline Data (bytes:{}) at {}.", CacheData.size(), Path);

            const auto Status = FPlatform::AtomicWriteFile(Path, CacheData.data(), CacheData.size());
            if (Status != EPlatformIOStatus::Success)
            { LOG_ERROR("Failed to save Vulkan Pipeline Cache at {}: {}", Path, Status); }
        }
        else { LOG_ERROR("Failed to get Vulkan Pipeline Cache data, skipped to save the cache!"); }
    }
}