module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.PipelineCache;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.Platform;
import Visera.Core.OS.FileSystem;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanPipelineCache
    {
    public:
        [[nodiscard]] inline const vk::raii::PipelineCache&
        GetHandle() const { return Handle; }

    private:
        vk::raii::PipelineCache Handle {nullptr};
        FPath Path;
        Bool  bExpired {False};

    public:
        FVulkanPipelineCache() = delete;
        FVulkanPipelineCache(const vk::raii::Device& I_Device,
                             const FPath&            I_Path);
        ~FVulkanPipelineCache();
    };

    FVulkanPipelineCache::
    FVulkanPipelineCache(const vk::raii::Device& I_Device,
                         const FPath&            I_Path)
    : Path{ GPlatform->GetExecutableDirectory() / I_Path }
    {
        LOG_DEBUG("Loading the Vulkan Pipeline Cache from \"{}\".", Path);

        if(!FFileSystem::Exists(Path))
        {
            // Try to create a new file
            std::ofstream File(Path.GetNativePath(), std::ios::binary);
            if (!File)
            {
                LOG_FATAL("Failed to create the Vulkan Pipeline Cache at \"{}\"!", Path);
            }
            LOG_DEBUG("Created a new Vulkan Pipeline Cache file at \"{}\".", Path);
        }

        // Read from the file
        // At this point file exists → try to read
        std::ifstream File(Path.GetNativePath(), std::ios::binary | std::ios::ate);
        if (!File)
        {
            LOG_FATAL("Failed to open the Vulkan Pipeline Cache at \"{}\"!", Path);
            return;
        }

        UInt64 Size = File.tellg();
        File.seekg(0, std::ios::beg);

        std::vector<char> Data(static_cast<size_t>(Size));
        if (Size > 0 && !File.read(Data.data(), Size))
        {
            LOG_ERROR("Failed to read Vulkan Pipeline Cache data from \"{}\".", Path);
            return;
        }

        // // TODO: pass Data to vk::raii::PipelineCache creation
        // vk::PipelineCacheCreateInfo CacheInfo({}, Data.size(),
        //                                     Data.empty() ? nullptr : Data.data());

        // Handle = vk::raii::PipelineCache{ I_Device, CacheInfo };
        LOG_DEBUG("Loaded Vulkan Pipeline Cache (bytes:{}).", Size);
    }

    FVulkanPipelineCache::
    ~FVulkanPipelineCache()
    {
        // Save the Cache
        LOG_DEBUG("Caching the \"{}\"", Path);
    }
}