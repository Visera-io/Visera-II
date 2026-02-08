module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub.Font;
#define VISERA_MODULE_NAME "AssetHub.Font"
export import Visera.AssetHub.Font.FreeType;
import Visera.AssetHub.Asset;
import Visera.Core.Font;
import Visera.Core.Types.Array;
import Visera.Core.Types.Optional;
import Visera.Global.Log;

export namespace Visera
{
	/**
	 * Font face asset: read-only IAsset, holds Core FFont + backend face. Created via FFreeType::Load then ctor; no public Load/Set API.
	 */
	class VISERA_ASSETHUB_API FFontAsset : public IAsset
	{
	public:
		[[nodiscard]] const FFont&
		GetFont() const { return Font; }
		[[nodiscard]] UInt64
		GetByteSize() const override { return Font.GetByteSize(); }
		[[nodiscard]] FFontFaceInfo
		GetFaceInfo() const { return Font.GetFaceInfo(); }
		[[nodiscard]] TOptional<FGlyphData>
		RenderGlyph(UInt32 I_CodePoint);
		[[nodiscard]] TOptional<FGlyphMetrics>
		GetGlyphMetrics(UInt32 I_CodePoint);
		[[nodiscard]] Bool
		HasGlyph(UInt32 I_CodePoint) const;
		[[nodiscard]] TOptional<FGlyphOutline>
		LoadGlyphOutline(UInt32 I_CodePoint);
		[[nodiscard]] Bool
		IsLoaded() const { return Face != nullptr; }
		[[nodiscard]] FFreeType::FFace
		GetFreeTypeFace() const { return Face; }

	private:
		FFont Font;
		FFreeType::FFace Face{nullptr};
		void Cleanup();

	public:
		FFontAsset() = default;
		/** Construct from Core FFont + backend face (size must be set via FFreeType::SetPixelSizes before wrapping if needed). */
		FFontAsset(FFont I_Font, FFreeType::FFace I_Face)
			: Font{std::move(I_Font)}, Face{I_Face} {}
		~FFontAsset();
		FFontAsset(const FFontAsset&) = delete;
		FFontAsset& operator=(const FFontAsset&) = delete;
		FFontAsset(FFontAsset&& I_Other) noexcept;
		FFontAsset& operator=(FFontAsset&& I_Other) noexcept;
	};

	FFontAsset::
	~FFontAsset()
	{
		Cleanup();
	}

	FFontAsset::
	FFontAsset(FFontAsset&& I_Other) noexcept
		: Font{std::move(I_Other.Font)}
		, Face{I_Other.Face}
	{
		I_Other.Face = nullptr;
	}

	FFontAsset& FFontAsset::
	operator=(FFontAsset&& I_Other) noexcept
	{
		if (this != &I_Other)
		{
			Cleanup();
			Font = std::move(I_Other.Font);
			Face = I_Other.Face;
			I_Other.Face = nullptr;
		}
		return *this;
	}

	void FFontAsset::
	Cleanup()
	{
		if (Face != nullptr)
		{
			FFreeType::DoneFace(Face);
			Face = nullptr;
		}
	}

	TOptional<FGlyphMetrics> FFontAsset::
	GetGlyphMetrics(UInt32 I_CodePoint)
	{
		if (Face == nullptr) { LOG_ERROR("Font face is not loaded!"); return NullOpt; }
		const FFreeType::UInt GlyphIndex = FFreeType::GetCharIndex(Face, I_CodePoint);
		if (GlyphIndex == 0) { LOG_WARN("Glyph not found for code point: {}", I_CodePoint); return NullOpt; }
		if (!FFreeType::LoadGlyph(Face, GlyphIndex, FFreeType::LoadDefault))
		{ LOG_ERROR("Failed to load glyph for code point: {}", I_CodePoint); return NullOpt; }
		return TOptional<FGlyphMetrics>(FFreeType::GetGlyphMetrics(Face));
	}

	TOptional<FGlyphData> FFontAsset::
	RenderGlyph(UInt32 I_CodePoint)
	{
		if (Face == nullptr) { LOG_ERROR("Font face is not loaded!"); return NullOpt; }
		const FFreeType::UInt GlyphIndex = FFreeType::GetCharIndex(Face, I_CodePoint);
		if (GlyphIndex == 0) { LOG_WARN("Glyph not found for code point: {}", I_CodePoint); return NullOpt; }
		if (!FFreeType::LoadGlyph(Face, GlyphIndex, FFreeType::LoadRender))
		{ LOG_ERROR("Failed to load and render glyph for code point: {}", I_CodePoint); return NullOpt; }
		FGlyphData GlyphData;
		GlyphData.Metrics = FFreeType::GetGlyphMetrics(Face);
		if (!FFreeType::GetGlyphBitmap(Face, GlyphData.BitmapData, GlyphData.BitmapWidth, GlyphData.BitmapHeight, GlyphData.BitmapPitch))
		{
			GlyphData.BitmapWidth = 0;
			GlyphData.BitmapHeight = 0;
			GlyphData.BitmapPitch = 0;
		}
		return TOptional<FGlyphData>(GlyphData);
	}

	Bool FFontAsset::
	HasGlyph(UInt32 I_CodePoint) const
	{
		if (Face == nullptr) return False;
		return FFreeType::GetCharIndex(Face, I_CodePoint) != 0;
	}

	TOptional<FGlyphOutline> FFontAsset::
	LoadGlyphOutline(UInt32 I_CodePoint)
	{
		if (Face == nullptr) { LOG_ERROR("Font face is not loaded!"); return NullOpt; }
		const FFreeType::UInt GlyphIndex = FFreeType::GetCharIndex(Face, I_CodePoint);
		if (GlyphIndex == 0) { LOG_WARN("Glyph not found for code point: {}", I_CodePoint); return NullOpt; }
		if (!FFreeType::LoadGlyph(Face, GlyphIndex, FFreeType::LoadNoScale | FFreeType::LoadNoHinting))
		{ LOG_ERROR("Failed to load glyph for code point: {}", I_CodePoint); return NullOpt; }
		if (FFreeType::GetGlyphFormat(Face) != FFreeType::GlyphFormatOutline)
		{ LOG_ERROR("Glyph is not an outline format!"); return NullOpt; }
		return FFreeType::GetGlyphOutline(Face);
	}
}
