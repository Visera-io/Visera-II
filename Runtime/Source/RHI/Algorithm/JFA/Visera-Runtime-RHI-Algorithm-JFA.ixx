module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Algorithm.JFA;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FJumpFlooding
    {
    public:

    private:
        FVulkanImageView PingImage;
        FVulkanImageView PongImage;
    };
}