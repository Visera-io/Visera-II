module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Texture;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Types;
import Visera.Core.Log;

namespace Visera
{
    /*
     * Only one descriptor set.
     */
    export class VISERA_RUNTIME_API FStaticTexture2D
    {
    public:
        inline void
        WriteTo(TSharedRef<FDescriptorSet> I_DescriptorSet, UInt8 I_Binding);

    private:
        TSharedPtr<FImageView>     ImageView;
        TSharedPtr<FSampler>       Sampler;

    public:
        FStaticTexture2D() = delete;
        FStaticTexture2D(TSharedRef<FImageView>     I_ImageView,
                         TSharedRef<FSampler>       I_Sampler);
    };

    FStaticTexture2D::
    FStaticTexture2D(TSharedRef<FImageView>     I_ImageView,
                     TSharedRef<FSampler>       I_Sampler)
    : ImageView     {I_ImageView},
      Sampler       {I_Sampler}
    {

    }

    void FStaticTexture2D::
    WriteTo(TSharedRef<FDescriptorSet> I_DescriptorSet, UInt8 I_Binding)
    {
        I_DescriptorSet->WriteImage(I_Binding, ImageView, Sampler);
    }
}