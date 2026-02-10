module;
#include <Visera-Core.hpp>
export module Visera.Core.Font;
#define VISERA_MODULE_NAME "Core.Font"
import Visera.Core.Containers.Array;
import Visera.Core.Types.String;

export namespace Visera
{
	/**
	 * Enumerates the types of font file formats supported.
	 */
	enum class EFontFormat : Int8
	{
		/** Invalid or unrecognized format. */
		Invalid = -1,
		/** TrueType font (.ttf). */
		TrueType = 0,
		/** OpenType font (.otf). */
		OpenType,
		/** Type 1 font (.pfa, .pfb). */
		Type1,
		/** CID font. */
		CID,
		/** CFF font. */
		CFF,
		/** Windows FNT font. */
		WindowsFNT,
		/** BDF font. */
		BDF,
		/** PCF font. */
		PCF,
		/** SFNT font. */
		SFNT,
	};

	/**
	 * Represents a glyph's metrics and rendering information.
	 */
	struct VISERA_CORE_API FGlyphMetrics
	{
		/** Width of the glyph in pixels. */
		UInt32 Width{0};
		/** Height of the glyph in pixels. */
		UInt32 Height{0};
		/** Horizontal bearing X (left side bearing). */
		Int32 BearingX{0};
		/** Horizontal bearing Y (top side bearing). */
		Int32 BearingY{0};
		/** Horizontal advance (how much to move cursor after this glyph). */
		Int32 AdvanceX{0};
		/** Vertical advance (for vertical text). */
		Int32 AdvanceY{0};
	};

	/**
	 * Represents a glyph's rendering data and metrics.
	 */
	struct VISERA_CORE_API FGlyphData
	{
		/** Glyph metrics. */
		FGlyphMetrics Metrics;
		/** Rendered glyph bitmap data (grayscale, 8-bit per pixel). */
		TArray<FByte> BitmapData;
		/** Bitmap width. */
		UInt32 BitmapWidth{0};
		/** Bitmap height. */
		UInt32 BitmapHeight{0};
		/** Bitmap pitch (bytes per row). */
		UInt32 BitmapPitch{0};
	};

	/**
	 * Represents font face information (pure data).
	 */
	struct VISERA_CORE_API FFontFaceInfo
	{
		/** Font family name. */
		FString FamilyName;
		/** Font style name. */
		FString StyleName;
		/** Number of faces in the font file. */
		Int32 FaceCount{0};
		/** Current face index. */
		Int32 FaceIndex{0};
		/** Font format. */
		EFontFormat Format{EFontFormat::Invalid};
	};

	/**
	 * Represents a point in a glyph outline.
	 */
	struct VISERA_CORE_API FGlyphOutlinePoint
	{
		/** X coordinate (in font units, typically 1/64 pixel). */
		Int32 X{0};
		/** Y coordinate (in font units, typically 1/64 pixel). */
		Int32 Y{0};
		/** Point tag (curve type flags). */
		UInt8 Tag{0};
	};

	/**
	 * Represents a glyph outline for conversion to other formats.
	 */
	struct VISERA_CORE_API FGlyphOutline
	{
		/** Array of outline points. */
		TArray<FGlyphOutlinePoint> Points;
		/** Array of contour end indices. */
		TArray<Int32> ContourEnds;
	};

	/**
	 * Pure font data: font file bytes + face info (like FImage for images).
	 * Backend (e.g. FreeType) opens this for rendering; asset holds FFont + backend handle.
	 */
	class VISERA_CORE_API FFont
	{
	public:
		[[nodiscard]] const TArray<FByte>&
		GetData() const { return Data; }
		[[nodiscard]] const FFontFaceInfo&
		GetFaceInfo() const { return FaceInfo; }
		[[nodiscard]] UInt64
		GetByteSize() const { return Data.GetSize(); }

	private:
		TArray<FByte> Data;
		FFontFaceInfo FaceInfo;

	public:
		FFont() = default;
		FFont(TArray<FByte> I_Data, FFontFaceInfo I_Info)
			: Data{std::move(I_Data)}, FaceInfo{std::move(I_Info)} {}
	};
}
