module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub.Font;
#define VISERA_MODULE_NAME "AssetHub.Font"
export import Visera.AssetHub.Font.Common;
import Visera.AssetHub.Font.FreeType;
import Visera.Core.Types.Path;
import Visera.Core.Types.Pointer;
import Visera.Core.Types.Array;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Global.Log;
import Visera.Core.OS.FileSystem;

export namespace Visera
{
	/**
	 * FreeType font face wrapper.
	 * Provides access to font loading and glyph rendering functionality.
	 */
	class VISERA_ASSETHUB_API FFont
	{
	public:
		/**
		 * Loads a font face from a file path.
		 * @param I_Path Path to the font file
		 * @param I_FaceIndex Face index in the font file (0 for single-face fonts)
		 * @return True if successful, false otherwise
		 */
		[[nodiscard]] Bool
		LoadFromFile(const FPath& I_Path, Int32 I_FaceIndex = 0);

		/**
		 * Loads a font face from memory.
		 * @param I_Data Font file data
		 * @param I_FaceIndex Face index in the font file (0 for single-face fonts)
		 * @return True if successful, false otherwise
		 */
		[[nodiscard]] Bool
		LoadFromMemory(const TArray<FByte>& I_Data, Int32 I_FaceIndex = 0);

		/**
		 * Sets the font size in pixels.
		 * @param I_Size Font size in pixels
		 * @return True if successful, false otherwise
		 */
		[[nodiscard]] Bool
		SetPixelSize(UInt32 I_Size);

		/**
		 * Sets the font size in points (72 DPI).
		 * @param I_Size Font size in points
		 * @return True if successful, false otherwise
		 */
		[[nodiscard]] Bool
		SetPointSize(UInt32 I_Size);

		/**
		 * Renders a glyph for the given Unicode code point.
		 * @param I_CodePoint Unicode code point
		 * @return Glyph data, or nullopt on failure
		 */
		[[nodiscard]] TOptional<FGlyphData>
		RenderGlyph(UInt32 I_CodePoint);

		/**
		 * Gets glyph metrics for the given Unicode code point without rendering.
		 * @param I_CodePoint Unicode code point
		 * @return Glyph metrics, or nullopt on failure
		 */
		[[nodiscard]] TOptional<FGlyphMetrics>
		GetGlyphMetrics(UInt32 I_CodePoint);

		/**
		 * Gets font face information.
		 * @return Font face info
		 */
		[[nodiscard]] FFontFaceInfo
		GetFaceInfo() const;

		/**
		 * Checks if a glyph exists for the given Unicode code point.
		 * @param I_CodePoint Unicode code point
		 * @return True if glyph exists, false otherwise
		 */
		[[nodiscard]] Bool
		HasGlyph(UInt32 I_CodePoint) const;

		/**
		 * Loads a glyph outline for the given Unicode code point.
		 * The outline is loaded without scaling or hinting, suitable for MSDF generation.
		 * @param I_CodePoint Unicode code point
		 * @return Glyph outline data, or nullopt on failure
		 */
		[[nodiscard]] TOptional<FGlyphOutline>
		LoadGlyphOutline(UInt32 I_CodePoint);

		/**
		 * Checks if the face is loaded.
		 * @return True if loaded, false otherwise
		 */
		[[nodiscard]] Bool
		IsLoaded() const { return Face != nullptr; }

		/**
		 * Gets the underlying FreeType face handle.
		 * For advanced usage, e.g., accessing units_per_EM for coordinate scaling.
		 * @return FreeType face handle, or nullptr if not loaded
		 */
		[[nodiscard]] FFreeType::FFace
		GetFreeTypeFace() const { return Face; }

	public:
		FFont();
		~FFont();

		// Non-copyable
		FFont(const FFont&) = delete;
		FFont& operator=(const FFont&) = delete;

		// Movable
		FFont(FFont&& I_Other) noexcept;
		FFont& operator=(FFont&& I_Other) noexcept;

	private:
		FFreeType::FFace Face{nullptr};
		FFontFaceInfo FaceInfo;
		/** Persistent buffer for memory-loaded fonts. */
		TArray<FByte> MemoryBuffer;

		/**
		 * Cleans up the face.
		 */
		void
		Cleanup();
	};

	// FFont implementation
	FFont::
	FFont()
	{
	}

	FFont::
	~FFont()
	{
		Cleanup();
	}

	FFont::
	FFont(FFont&& I_Other) noexcept
		: Face{I_Other.Face}
		, FaceInfo{std::move(I_Other.FaceInfo)}
		, MemoryBuffer{std::move(I_Other.MemoryBuffer)}
	{
		I_Other.Face = nullptr;
	}

	FFont& FFont::
	operator=(FFont&& I_Other) noexcept
	{
		if (this != &I_Other)
		{
			Cleanup();
			Face = I_Other.Face;
			FaceInfo = std::move(I_Other.FaceInfo);
			MemoryBuffer = std::move(I_Other.MemoryBuffer);
			I_Other.Face = nullptr;
		}
		return *this;
	}

	void FFont::
	Cleanup()
	{
		if (Face != nullptr)
		{
			FFreeType::DoneFace(Face);
			Face = nullptr;
		}
		MemoryBuffer.Clear();
	}


	Bool FFont::
	LoadFromFile(const FPath& I_Path, Int32 I_FaceIndex)
	{
		Cleanup();

		auto FaceInfoOpt = FFreeType::Load(I_Path, I_FaceIndex, Face);
		if (!FaceInfoOpt.HasValue())
		{
			return False;
		}

		FaceInfo = FaceInfoOpt.GetValue();
		return True;
	}

	Bool FFont::
	LoadFromMemory(const TArray<FByte>& I_Data, Int32 I_FaceIndex)
	{
		Cleanup();

		auto FaceInfoOpt = FFreeType::Load(I_Data, I_FaceIndex, Face, MemoryBuffer);
		if (!FaceInfoOpt.HasValue())
		{
			return False;
		}

		FaceInfo = FaceInfoOpt.GetValue();
		return True;
	}

	Bool FFont::
	SetPixelSize(UInt32 I_Size)
	{
		if (Face == nullptr)
		{
			LOG_ERROR("Font face is not loaded!");
			return False;
		}

		if (!FFreeType::SetPixelSizes(Face, I_Size))
		{
			LOG_ERROR("Failed to set pixel size: {}", I_Size);
			return False;
		}

		return True;
	}

	Bool FFont::
	SetPointSize(UInt32 I_Size)
	{
		if (Face == nullptr)
		{
			LOG_ERROR("Font face is not loaded!");
			return False;
		}

		if (!FFreeType::SetCharSize(Face, I_Size))
		{
			LOG_ERROR("Failed to set point size: {}", I_Size);
			return False;
		}

		return True;
	}

	TOptional<FGlyphMetrics> FFont::
	GetGlyphMetrics(UInt32 I_CodePoint)
	{
		if (Face == nullptr)
		{
			LOG_ERROR("Font face is not loaded!");
			return NullOpt;
		}

		const FFreeType::UInt GlyphIndex = FFreeType::GetCharIndex(Face, I_CodePoint);
		if (GlyphIndex == 0)
		{
			LOG_WARN("Glyph not found for code point: {}", I_CodePoint);
			return NullOpt;
		}

		if (!FFreeType::LoadGlyph(Face, GlyphIndex, FFreeType::LoadDefault))
		{
			LOG_ERROR("Failed to load glyph for code point: {}", I_CodePoint);
			return NullOpt;
		}

		return TOptional<FGlyphMetrics>(FFreeType::GetGlyphMetrics(Face));
	}

	TOptional<FGlyphData> FFont::
	RenderGlyph(UInt32 I_CodePoint)
	{
		if (Face == nullptr)
		{
			LOG_ERROR("Font face is not loaded!");
			return NullOpt;
		}

		const FFreeType::UInt GlyphIndex = FFreeType::GetCharIndex(Face, I_CodePoint);
		if (GlyphIndex == 0)
		{
			LOG_WARN("Glyph not found for code point: {}", I_CodePoint);
			return NullOpt;
		}

		if (!FFreeType::LoadGlyph(Face, GlyphIndex, FFreeType::LoadRender))
		{
			LOG_ERROR("Failed to load and render glyph for code point: {}", I_CodePoint);
			return NullOpt;
		}

		FGlyphData GlyphData;
		GlyphData.Metrics = FFreeType::GetGlyphMetrics(Face);

		// Get bitmap data
		if (!FFreeType::GetGlyphBitmap(Face, GlyphData.BitmapData, GlyphData.BitmapWidth, GlyphData.BitmapHeight, GlyphData.BitmapPitch))
		{
			// Bitmap may be empty, which is fine
			GlyphData.BitmapWidth = 0;
			GlyphData.BitmapHeight = 0;
			GlyphData.BitmapPitch = 0;
		}

		return TOptional<FGlyphData>(GlyphData);
	}

	FFontFaceInfo FFont::
	GetFaceInfo() const
	{
		return FaceInfo;
	}

	Bool FFont::
	HasGlyph(UInt32 I_CodePoint) const
	{
		if (Face == nullptr)
		{ return False; }

		const FFreeType::UInt GlyphIndex = FFreeType::GetCharIndex(Face, I_CodePoint);
		return GlyphIndex != 0;
	}

	TOptional<FGlyphOutline> FFont::
	LoadGlyphOutline(UInt32 I_CodePoint)
	{
		if (Face == nullptr)
		{
			LOG_ERROR("Font face is not loaded!");
			return NullOpt;
		}

		const FFreeType::UInt GlyphIndex = FFreeType::GetCharIndex(Face, I_CodePoint);
		if (GlyphIndex == 0)
		{
			LOG_WARN("Glyph not found for code point: {}", I_CodePoint);
			return NullOpt;
		}

		if (!FFreeType::LoadGlyph(Face, GlyphIndex, FFreeType::LoadNoScale | FFreeType::LoadNoHinting))
		{
			LOG_ERROR("Failed to load glyph for code point: {}", I_CodePoint);
			return NullOpt;
		}

		if (FFreeType::GetGlyphFormat(Face) != FFreeType::GlyphFormatOutline)
		{
			LOG_ERROR("Glyph is not an outline format!");
			return NullOpt;
		}

		return FFreeType::GetGlyphOutline(Face);
	}
}
