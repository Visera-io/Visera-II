module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Material;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Core.Types.Pointer;
	   import Visera.Core.Types.JSON;
	   import Visera.Core.Containers.Array;
	   import Visera.Core.Types.String;
	   import Visera.Runtime.RHI;
	   import Visera.Core.Log;

export namespace Visera
{
	enum class ESurfaceType : UInt8
	{
		Opaque,
		Masked,
		Transparent,
	};

	class VISERA_RUNTIME_API FMaterial
	{
	public:
		[[nodiscard]] const FRHIRenderPassID&
		GetRenderPass() const noexcept { return RenderPass; }
		[[nodiscard]] const FRHIDescriptorSetID&
		GetDescriptorSet() const noexcept { return DescriptorSet; }
		[[nodiscard]] ESurfaceType
		GetSurface() const noexcept { return Surface; }
		[[nodiscard]] Bool
		IsValid() const noexcept { return bValid; }

		FMaterial(FRHIRenderPassID    I_RenderPass,
		          FRHIDescriptorSetID I_DescriptorSet,
		          FRHISamplerID       I_Sampler,
		          FRHITextureID       I_BaseColor,
		          ESurfaceType        I_Surface)
			: RenderPass    (std::move(I_RenderPass))
			, DescriptorSet (std::move(I_DescriptorSet))
			, Sampler       (std::move(I_Sampler))
			, BaseColor     (std::move(I_BaseColor))
			, Surface       (I_Surface)
		{}

	private:
		FRHIRenderPassID    RenderPass;
		FRHIDescriptorSetID DescriptorSet;
		FRHISamplerID       Sampler;
		FRHITextureID       BaseColor;
		ESurfaceType        Surface {ESurfaceType::Opaque};
		Bool                bValid  {True};

	public:
		~FMaterial() = default;
		FMaterial(const FMaterial&) = default;
		FMaterial& operator=(const FMaterial&) = default;
		FMaterial(FMaterial&&) = default;
		FMaterial& operator=(FMaterial&&) = default;
	};
}
