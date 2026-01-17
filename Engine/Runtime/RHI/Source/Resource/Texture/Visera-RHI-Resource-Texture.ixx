module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Resource.Texture;
#define VISERA_MODULE_NAME "RHI.Resource"
import Visera.RHI.Common;
import Visera.RHI.Vulkan.Image;
import Visera.Core.Types.Array;
import Visera.Core.Math.Arithmetic.Interval;

export namespace Visera
{
    /**
     * Texture = Image + ImageView
     */
    struct VISERA_RHI_API FRHITextureCreateDesc
    {
        UInt32            Width    {0};
        UInt32            Height   {0};
        UInt32            Depth    {0};
        ERHIFormat        Format   {ERHIFormat::Undefined};
        ERHIImageType     Type     {ERHIImageType::Undefined};
        ERHIImageUsage    Usages   {ERHIImageUsage::None};
        ERHIImageViewType ViewType {ERHIImageViewType::Undefined};

        TClosedInterval<UInt8>
        MipLevelRange       {0, 0};
        TClosedInterval<UInt8>
        ArrayLayerRange     {0, 0};
        ERHISamplingRate
        SampleCount         {ERHISamplingRate::X1};

        Bool operator==(const FRHITextureCreateDesc&) const = default;
    };

    class VISERA_RHI_API FRHITexture
    {
    public:
        [[nodiscard]] auto&
        GetInfo() const { return Info; }
        [[nodiscard]] Bool
        IsSameLayout(const FRHITexture& I_Other) const;

    private:
        const FRHITextureCreateDesc Info;
        FVulkanImage                Image;
        FVulkanImageView            ImageView;

    public:
        FRHITexture() = delete;
        FRHITexture(FRHITextureCreateDesc&& I_TextureDesc,
                    FVulkanImage&&          I_Image,
                    FVulkanImageView&&      I_ImageView)
        : Info      {std::move(I_TextureDesc)},
          Image     {std::move(I_Image)},
          ImageView {std::move(I_ImageView)}
        {

        }
    };
}