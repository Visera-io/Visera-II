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

    public:
        FVulkanPipelineCache() = delete;
        FVulkanPipelineCache(const FPath& I_Name);
        ~FVulkanPipelineCache();
    };

    FVulkanPipelineCache::
    FVulkanPipelineCache(const FPath& I_Name)
    {
        auto FileSystem = FFileSystem{ GPlatform->GetExecutableDirectory() };
        if(FileSystem.SearchFile(I_Name).IsEmpty())
        {
            LOG_FATAL("Failed to create the Vulkan Pipeline Cache!");
        }
    }

    FVulkanPipelineCache::
    ~FVulkanPipelineCache()
    {

    }
}