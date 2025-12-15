module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Vulkan.Pipeline.Cache;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.OS.FileSystem;
import Visera.Core.OS.Memory;
import Visera.Core.Log;
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
        if(!FFileSystem::Exists(Path))
        {
            if (auto File = FFileSystem::OpenOStream(Path, EIOMode::Binary))
            {
                LOG_DEBUG("Created a new Vulkan Pipeline Cache file at \"{}\".", Path);
            }
            else { LOG_FATAL("Failed to create the Vulkan Pipeline Cache at \"{}\"!", Path); }
        }

        // Read from the file
        if (auto File = FFileSystem::OpenIStream(Path, EIOMode::Binary))
        {
            File->seekg(0, std::ios::end);
            Int64 Size = File->tellg();
            File->seekg(0, std::ios::beg);

            std::vector<char> CacheData(Size);
            if (Size > 0 && !File->read(CacheData.data(), Size))
            {
                LOG_ERROR("Failed to read Vulkan Pipeline Cache data from \"{}\".", Path);
                return;
            }

            Bool bExpired = CacheData.size() < sizeof(vk::PipelineCacheHeaderVersionOne);
            if (!bExpired)
            {
                auto* CacheHeader = reinterpret_cast<vk::PipelineCacheHeaderVersionOne*>(CacheData.data());
                auto  GPUProperties = I_GPU.getProperties();

                bExpired = CacheData.empty() ||
                           CacheHeader->deviceID != GPUProperties.deviceID ||
                           CacheHeader->vendorID != GPUProperties.vendorID ||
                           Memory::Memcmp(CacheHeader->pipelineCacheUUID,
                                          GPUProperties.pipelineCacheUUID,
                                          vk::UuidSize) != 0;
            }

            if (bExpired)
            {
                LOG_DEBUG("Vulkan Pipeline Cache expired!");
                CacheData.clear();
            }

            auto CreateInfo = vk::PipelineCacheCreateInfo()
                .setInitialDataSize (CacheData.size())
                .setPInitialData    (CacheData.data())
            ;
            auto Result = I_Device.createPipelineCache(CreateInfo);
            if (!Result.has_value())
            { LOG_FATAL("Failed to create the Vulkan Pipeline Cache from \"{}\"!", Path); }
            else
            { Handle = std::move(*Result); }

            LOG_DEBUG("Loaded Vulkan Pipeline Cache (bytes:{}) from \"{}\".", Size, Path);
        }
        else { LOG_FATAL("Failed to open the Vulkan Pipeline Cache at \"{}\"!", Path); }
    }

    FVulkanPipelineCache::
    ~FVulkanPipelineCache()
    {
        if (auto Result = Handle.getData(); Result.has_value())
        {
            auto CacheData {std::move(*Result)};
            LOG_DEBUG("Caching Vulkan Pipeline Data (bytes:{}) at \"{}\".", CacheData.size(), Path);

            if (auto File = FFileSystem::OpenOStream(Path, EIOMode::Binary))
            {
                File->write(reinterpret_cast<char*>(CacheData.data()),
                           static_cast<std::streamsize>(CacheData.size()));
            }
            else { LOG_ERROR("Failed to open the Vulkan Pipeline Data at \"{}\"!", Path); }
        }
        else { LOG_ERROR("Failed to get Vulkan Pipeline Cache data, skipped to save the cache!"); }
    }
}