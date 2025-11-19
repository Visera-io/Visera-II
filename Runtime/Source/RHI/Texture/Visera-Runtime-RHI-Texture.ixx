module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Texture;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Core.Log;

namespace Visera::RHI
{
    /*
     * Only one descriptor set.
     */
    export class VISERA_RUNTIME_API FStaticTexture2D
    {
    public:
        inline void
        Write(TSharedRef<FImageView> I_ImageView);

    private:
        TSharedPtr<FImageView>     ImageView;
        TSharedPtr<FSampler>       Sampler;
        TSharedPtr<FDescriptorSet> DescriptorSet;
        UInt8                      Binding;

    public:
        FStaticTexture2D() = delete;
        FStaticTexture2D(TSharedRef<FImageView>     I_ImageView,
                         TSharedRef<FSampler>       I_Sampler,
                         TSharedRef<FDescriptorSet> I_DescriptorSet,
                         UInt8                      I_Binding);
    };

    FStaticTexture2D::
    FStaticTexture2D(TSharedRef<FImageView>     I_ImageView,
                     TSharedRef<FSampler>       I_Sampler,
                     TSharedRef<FDescriptorSet> I_DescriptorSet,
                     UInt8                      I_Binding)
    : ImageView     {I_ImageView},
      Sampler       {I_Sampler},
      DescriptorSet {I_DescriptorSet},
      Binding       {I_Binding}
    {
        DescriptorSet->WriteImage(Binding, ImageView, Sampler);
    }

    void FStaticTexture2D::
    Write(TSharedRef<FImageView> I_ImageView)
    {
        ImageView = I_ImageView;
        DescriptorSet->WriteImage(Binding, ImageView, Sampler);
    }
}