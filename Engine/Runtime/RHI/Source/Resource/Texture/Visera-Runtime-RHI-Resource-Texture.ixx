module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Resource.Texture;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry.Item;
import Visera.Runtime.RHI.Vulkan.Image;
import Visera.Core.Types.Array;
import Visera.Core.Math.Arithmetic.Interval;

export namespace Visera
{
    /**
     * Texture = Image + ImageView
     */
    class VISERA_RUNTIME_API FRHITexture : public IRHIRegistryItem
    {
    public:
        struct VISERA_RUNTIME_API FCreateInfo
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

            Bool operator==(const FCreateInfo&) const = default;
            Bool IsCompatibleWith(const FCreateInfo& I_Other) const
            { return *this == I_Other; }
        };

        [[nodiscard]] const FCreateInfo&
        GetInfo() const { return Info; }
        [[nodiscard]] FVulkanImage*
        GetVulkanImage()     { return &Image; }
        [[nodiscard]] FVulkanImageView*
        GetVulkanImageView() { return &ImageView; }

    private:
        const FCreateInfo Info;
        FVulkanImage                Image;
        FVulkanImageView            ImageView;

    public:
        FRHITexture() = delete;
        FRHITexture(FCreateInfo&&       I_CreateInfo,
                    FVulkanImage&&      I_Image,
                    FVulkanImageView&&  I_ImageView)
        : Info      {std::move(I_CreateInfo)},
          Image     {std::move(I_Image)},
          ImageView {std::move(I_ImageView)}
        {

        }
    };

    using FRHITextureCreateInfo = FRHITexture::FCreateInfo;
}