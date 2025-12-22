module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.Texture;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Types.DescriptorSet;
import Visera.RHI.Types.Sampler;
import Visera.RHI.Types.Image;
import Visera.RHI.Common;
import Visera.Runtime.Log;
import Visera.Runtime.Name;

namespace Visera
{
    /*
     * Only one descriptor set.
     */
    export class VISERA_RHI_API FRHIStaticTexture
    {
    public:
        inline void
        WriteTo(TSharedRef<FRHIDescriptorSet> I_DescriptorSet, UInt8 I_Binding);

    private:
        TSharedPtr<FRHIImage>   Image;
        TSharedPtr<FRHISampler> Sampler;

    public:
        FRHIStaticTexture() = delete;
        FRHIStaticTexture(TSharedRef<FRHIImage>   I_Image,
                          TSharedRef<FRHISampler> I_Sampler);
    };

    FRHIStaticTexture::
    FRHIStaticTexture(TSharedRef<FRHIImage>   I_Image,
                        TSharedRef<FRHISampler> I_Sampler)
    : Image     {I_Image},
      Sampler   {I_Sampler}
    {

    }

    void FRHIStaticTexture::
    WriteTo(TSharedRef<FRHIDescriptorSet> I_DescriptorSet, UInt8 I_Binding)
    {
        I_DescriptorSet->WriteImage(I_Binding, Image, Sampler);
    }

    export class VISERA_RHI_API FTextureDesc
    {
    public:
        FName            Name;
        ERHITextureType  Type        = ERHITextureType::Texture2D;
        ERHIFormat       Format      = ERHIFormat::R8G8B8A8_UNorm;
        UInt32           Width       = 1;
        UInt32           Height      = 1;
        UInt32           Depth       = 1;
        UInt32           MipLevels   = 1;
        UInt32           ArrayLayers = 1;
        ERHISamplingRate SampleCount = ERHISamplingRate::X1;
        ERHITextureUsage Usage       = ERHITextureUsage::ShaderResource;

        [[nodiscard]] inline Bool
        IsUAV() const { return (Usage & ERHITextureUsage::UnorderedAccess); }
    };
}