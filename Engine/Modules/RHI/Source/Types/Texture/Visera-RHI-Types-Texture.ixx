module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.Texture;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Types.DescriptorSet;
import Visera.RHI.Types.Sampler;
import Visera.RHI.Types.ImageView;
import Visera.RHI.Common;
import Visera.Runtime.Log;
import Visera.Runtime.Name;
import Visera.Core.Math.Hash.GoldenRatio;

namespace Visera
{
    export class VISERA_RHI_API FRHITextureDesc
    {
    public:
        FName            Name;
        ERHIImageType    Type        = ERHIImageType::Image2D;
        ERHIFormat       Format      = ERHIFormat::R8G8B8A8_UNorm;
        UInt32           Width       = 1;
        UInt32           Height      = 1;
        UInt8            Depth       = 1;
        UInt8            MipLevels   = 1;
        UInt8            ArrayLayers = 1;
        ERHISamplingRate SampleCount = ERHISamplingRate::X1;
        ERHIImageUsage   Usage       = ERHIImageUsage::ShaderResource;

        [[nodiscard]] inline Bool
        IsUAV() const { return (Usage & ERHIImageUsage::UnorderedAccess); }

        [[nodiscard]] inline UInt64
        Hash() const
        {
            return Math::GoldenRatioHashCombine(
                0,
                Type, Format, Width, Height, Depth, MipLevels, ArrayLayers, SampleCount, Usage);
        }
    };
}