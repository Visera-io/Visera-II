module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Renderer.Sprite;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.Graphics.Material;

export namespace Visera
{
	/** Minimal Sprite Renderer: uses FMaterial for Shader + BaseColor texture (Sprite.slang + BaseColor). */
	class VISERA_RUNTIME_API FSpriteRenderer
	{
	public:
		void
		SetMaterial(TSharedPtr<FMaterial> I_Material) noexcept { Material = std::move(I_Material); }
		[[nodiscard]] TSharedPtr<FMaterial>
		GetMaterial() const noexcept { return Material; }

	private:
		TSharedPtr<FMaterial> Material {};
	};
}