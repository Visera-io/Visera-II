module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Resource.Sampler;
#define VISERA_MODULE_NAME "RHI.Resource"
import Visera.RHI.Common;
import Visera.RHI.Vulkan.Sampler;

export namespace Visera
{
    struct VISERA_RHI_API FRHISamplerCreateDesc
    {
        ERHISamplerType        Type;
        ERHISamplerAddressMode AddressMode {ERHISamplerAddressMode::Repeat}; // All directions (isotropic)

        Bool operator==(const FRHISamplerCreateDesc&) const = default;
        Bool IsCompatibleWith(const FRHISamplerCreateDesc& I_Other) const
        { return *this == I_Other; }
    };

    class VISERA_RHI_API FRHISampler
    {
    public:
        [[nodiscard]] const FRHISamplerCreateDesc&
        GetInfo() const { return Info; }
        [[nodiscard]] FVulkanSampler*
        GetVulkanSampler() { return &Sampler; }

    private:
        FRHISamplerCreateDesc Info;
        FVulkanSampler        Sampler;

    public:
        FRHISampler() noexcept = delete;
        FRHISampler(FRHISamplerCreateDesc&& I_CreateDesc, FVulkanSampler&& I_VulkanSampler)
        : Info      {std::move(I_CreateDesc)},
          Sampler   {std::move(I_VulkanSampler)} {}
    };
}