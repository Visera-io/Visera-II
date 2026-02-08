module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.Material;
#define VISERA_MODULE_NAME "Graphics.Material"
export import Visera.Core.Types.Pointer;
	   import Visera.Core.Types.JSON;
	   import Visera.Core.Types.Array;
	   import Visera.Core.Types.String;
	   import Visera.RHI;
	   import Visera.Global.Log;

export namespace Visera
{
	/** High-level surface type for material; matches Engine/Schemas/Material.schema.json "Surface" enum. */
	enum class ESurfaceType : UInt8
	{
		Opaque,
		Masked,
		Transparent,
	};

	/** Visera Material (minimal). Loads from .vmaterial JSON; supports Sprite Renderer via Shader + BaseColor.
	 *  See Engine/Schemas/Material.schema.json and Documents/docs/Fundamentals/Material.md.
	 */
	class VISERA_GRAPHICS_API FMaterial
	{
	public:
		[[nodiscard]] Bool
		IsValid() const noexcept { return !Shader.IsEmpty() && !BaseColorPath.IsEmpty(); }

	private:
		UInt8             Version         {1};
		FString           Shader          {};
		ESurfaceType      Surface         {ESurfaceType::Opaque};
		FString           BaseColorPath   {};

	public:
		FMaterial() = default;
		FMaterial(const FJSON& I_Description)
		{

		}
	};
}
