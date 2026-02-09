module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Resource.Sampler;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry.Item;
import Visera.Runtime.RHI.Vulkan.Sampler;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHISampler : public IRHIRegistryItem
    {
    public:
        struct VISERA_RUNTIME_API FCreateInfo
        {
            ERHISamplerType        Type;
            ERHISamplerAddressMode AddressMode {ERHISamplerAddressMode::Repeat}; // All directions (isotropic)

            Bool operator==(const FCreateInfo&) const = default;
            Bool IsCompatibleWith(const FCreateInfo& I_Other) const
            { return *this == I_Other; }
        };

        [[nodiscard]] const FCreateInfo&
        GetInfo() const { return Info; }
        [[nodiscard]] FVulkanSampler*
        GetVulkanSampler() { return &Sampler; }

    private:
        const FCreateInfo Info;
        FVulkanSampler    Sampler;

    public:
        FRHISampler() noexcept = delete;
        FRHISampler(FCreateInfo&& I_CreateInfo, FVulkanSampler&& I_VulkanSampler)
        : Info   {std::move(I_CreateInfo)},
          Sampler{std::move(I_VulkanSampler)} {}
    };

    using FRHISamplerCreateInfo = FRHISampler::FCreateInfo;
}
