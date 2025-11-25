module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.Texture;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Types.DescriptorSet;
import Visera.Runtime.RHI.Types.Sampler;
import Visera.Runtime.RHI.Types.Image;
import Visera.Core.Log;

namespace Visera
{
    /*
     * Only one descriptor set.
     */
    export class VISERA_RUNTIME_API FRHIStaticTexture
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
}