module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub.Font.Common;
#define VISERA_MODULE_NAME "AssetHub.Font"
import Visera.Core.Types.Array;
import Visera.Core.Types.String;
import Visera.Core.Image;

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
	struct VISERA_ASSETHUB_API FGlyphMetrics
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
	struct VISERA_ASSETHUB_API FGlyphData
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
	 * Represents font face information.
	 */
	struct VISERA_ASSETHUB_API FFontFaceInfo
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
	struct VISERA_ASSETHUB_API FGlyphOutlinePoint
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
	struct VISERA_ASSETHUB_API FGlyphOutline
	{
		/** Array of outline points. */
		TArray<FGlyphOutlinePoint> Points;
		/** Array of contour end indices. */
		TArray<Int32> ContourEnds;
	};

}

