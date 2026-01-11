module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Resource.Texture;
#define VISERA_MODULE_NAME "RHI.Resource"
import Visera.RHI.Common;
import Visera.RHI.Vulkan.Image;
import Visera.Core.Types.Array;

export namespace Visera
{
    /**
     * Texture = Image + ImageView
     */
    struct VISERA_RHI_API FRHITextureCreateDesc
    {
        // Image Properties
        UInt32          Width  {0};
        UInt32          Height {0};
        UInt32          Depth  {0};
        ERHIFormat      Format {ERHIFormat::Undefined};
        ERHIImageType   Type   {ERHIImageType::Undefined};
        ERHIImageUsage  Usages {ERHIImageUsage::None};

        TArray<FByte>   Data; // Optional
    };

    class VISERA_RHI_API FRHITexture
    {
    public:

    private:
        FVulkanImage        Image;
        FVulkanImageView    ImageView;

    public:
        FRHITexture() = delete;
        FRHITexture(FVulkanImage&& I_Image, FVulkanImageView&& I_ImageView)
        : Image(std::move(I_Image)), ImageView(std::move(I_ImageView))
        {

        }
    };

}