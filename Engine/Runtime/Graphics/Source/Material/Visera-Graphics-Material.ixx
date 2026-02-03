module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.Material;
#define VISERA_MODULE_NAME "Graphics.Material"
import Visera.Core.Types.JSON;
import Visera.Core.Types.Array;
import Visera.Core.OS.FileSystem;
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
		[[nodiscard]] UInt8
		GetVersion() const noexcept { return Version; }
		[[nodiscard]] const FPath&
		GetShader() const noexcept { return Shader; }
		[[nodiscard]] ESurfaceType
		GetSurface() const noexcept { return Surface; }
		[[nodiscard]] const FPath&
		GetBaseColorPath() const noexcept { return BaseColorPath; }
		[[nodiscard]] FRHITextureHandle
		GetBaseColorHandle() const noexcept { return BaseColorHandle; }

		void
		SetBaseColorHandle(FRHITextureHandle I_Handle) noexcept { BaseColorHandle = I_Handle; }

		[[nodiscard]] Bool
		IsValid() const noexcept { return !Shader.IsEmpty() && !BaseColorPath.IsEmpty(); }

	private:
		UInt8             Version         {1};
		FPath             Shader          {};
		ESurfaceType      Surface         {ESurfaceType::Opaque};
		FPath             BaseColorPath   {};
		FRHITextureHandle BaseColorHandle {};

	public:
		FMaterial() = default;
		FMaterial(const FPath& I_Path) { (void)Parse(I_Path); }

		[[nodiscard]] Bool
		Parse(const FPath& I_Path)
		{
			auto File = FFileSystem::OpenFile(I_Path, EIOMode::Read);
			if (!File || !File->IsOpen())
			{
				LOG_ERROR("Failed to open material \"{}\".", I_Path.GetUTF8Path());
				return False;
			}

			TArray<FByte> Raw = File->ReadAll();
			if (Raw.IsEmpty())
			{
				LOG_ERROR("Material file is empty \"{}\".", I_Path.GetUTF8Path());
				return False;
			}

			FJSON Desc{Raw};

			Version = static_cast<UInt8>(Desc.GetNumber("Version", 1));
			Shader  = Desc.GetString("Shader");
			BaseColorPath = Desc.GetString("Textures.BaseColor");

			FString SurfaceStr = Desc.GetString("Surface");
			if      (SurfaceStr == "Opaque")	  { Surface = ESurfaceType::Opaque; }
			else if (SurfaceStr == "Masked")	  { Surface = ESurfaceType::Masked; }
			else if (SurfaceStr == "Transparent") { Surface = ESurfaceType::Transparent; }
			else
			{
				LOG_ERROR("Material \"{}\" invalid Surface \"{}\"; expected Opaque, Masked, or Transparent.", I_Path.GetUTF8Path(), SurfaceStr);
				return False;
			}

			return True;
		}
	};
}
