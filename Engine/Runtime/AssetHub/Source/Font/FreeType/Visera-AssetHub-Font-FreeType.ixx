module;
#include <Visera-AssetHub.hpp>
#include <freetype/freetype.h>
#include <freetype/ftfntfmt.h>
#include <freetype/ftoutln.h>
export module Visera.AssetHub.Font.FreeType;
#define VISERA_MODULE_NAME "AssetHub.Font"
import Visera.Core.Font;
import Visera.Core.Types.Path;
import Visera.Core.Types.Array;
import Visera.Core.Types.Optional;
import Visera.Global.Log;
import Visera.Core.OS.FileSystem;

export namespace Visera
{
	/**
	 * FreeType library singleton manager.
	 * Manages the global FreeType library instance.
	 */
	class VISERA_ASSETHUB_API FFreeType
	{
	public:
		// FreeType type aliases for external use (avoiding direct includes)
		using FLibrary = FT_Library;
		using FFace = FT_Face;
		using FByte = FT_Byte;
		using Error = FT_Error;
		using UInt = FT_UInt;
		using Long = FT_Long;
		using FVector = FT_Vector;
		using FOutline = FT_Outline;
		using FOutlineFuncs = FT_Outline_Funcs;
		using FOutlineMoveToFunc = FT_Outline_MoveToFunc;
		using FOutlineLineToFunc = FT_Outline_LineToFunc;
		using FOutlineConicToFunc = FT_Outline_ConicToFunc;
		using FOutlineCubicToFunc = FT_Outline_CubicToFunc;
		
		// FreeType constants
		static inline constexpr Error ErrOk = FT_Err_Ok;
		static inline constexpr Int32 GlyphFormatOutline = FT_GLYPH_FORMAT_OUTLINE;
		static inline constexpr UInt32 LoadDefault = FT_LOAD_DEFAULT;
		static inline constexpr UInt32 LoadNoScale = FT_LOAD_NO_SCALE;
		static inline constexpr UInt32 LoadNoHinting = FT_LOAD_NO_HINTING;
		static inline constexpr UInt32 LoadRender = FT_LOAD_RENDER;

		/**
		 * Gets the singleton instance.
		 * @return Reference to the singleton instance
		 */
		[[nodiscard]] static FFreeType&
		Get();

		/**
		 * Gets the FreeType library handle.
		 * @return FreeType library handle
		 */
		[[nodiscard]] static FLibrary
		GetLibrary();

		/**
		 * Checks if the library is initialized.
		 * @return True if initialized, false otherwise
		 */
		[[nodiscard]] static Bool
		IsInitialized();

		/**
		 * Loads a font face from a file path.
		 * @param I_Path Path to the font file
		 * @param I_FaceIndex Face index in the font file (0 for single-face fonts)
		 * @return Font face info, or nullopt on failure
		 */
		[[nodiscard]] static TOptional<FFontFaceInfo>
		Load(const FPath& I_Path, Int32 I_FaceIndex, FFace& I_OutFace);

		/**
		 * Loads a font face from memory.
		 * @param I_Data Font file data
		 * @param I_FaceIndex Face index in the font file (0 for single-face fonts)
		 * @param I_OutFace Output face handle
		 * @return Font face info, or nullopt on failure
		 */
		[[nodiscard]] static TOptional<FFontFaceInfo>
		Load(const TArray<FByte>& I_Data, Int32 I_FaceIndex, FFace& I_OutFace, TArray<FByte>& I_OutMemoryBuffer);

		/**
		 * Releases a font face.
		 * @param I_Face Face handle to release
		 */
		static void
		DoneFace(FFace I_Face);

		/**
		 * Sets the font size in pixels.
		 * @param I_Face Face handle
		 * @param I_Size Font size in pixels
		 * @return True if successful, false otherwise
		 */
		[[nodiscard]] static Bool
		SetPixelSizes(FFace I_Face, UInt32 I_Size);

		/**
		 * Sets the font size in points (72 DPI).
		 * @param I_Face Face handle
		 * @param I_Size Font size in points
		 * @return True if successful, false otherwise
		 */
		[[nodiscard]] static Bool
		SetCharSize(FFace I_Face, UInt32 I_Size);

		/**
		 * Gets the glyph index for a Unicode code point.
		 * @param I_Face Face handle
		 * @param I_CodePoint Unicode code point
		 * @return Glyph index, or 0 if not found
		 */
		[[nodiscard]] static UInt
		GetCharIndex(FFace I_Face, UInt32 I_CodePoint);

		/**
		 * Loads a glyph into the face's glyph slot.
		 * @param I_Face Face handle
		 * @param I_GlyphIndex Glyph index
		 * @param I_LoadFlags Load flags (e.g., LoadDefault, LoadRender, etc.)
		 * @return True if successful, false otherwise
		 */
		[[nodiscard]] static Bool
		LoadGlyph(FFace I_Face, UInt I_GlyphIndex, UInt32 I_LoadFlags);

		/**
		 * Gets glyph metrics from the currently loaded glyph.
		 * @param I_Face Face handle (must have a glyph loaded)
		 * @return Glyph metrics
		 */
		[[nodiscard]] static FGlyphMetrics
		GetGlyphMetrics(FFace I_Face);

		/**
		 * Gets glyph bitmap from the currently loaded glyph.
		 * @param I_Face Face handle (must have a glyph loaded and rendered)
		 * @param I_OutBitmapData Output bitmap data
		 * @param I_OutWidth Output bitmap width
		 * @param I_OutHeight Output bitmap height
		 * @param I_OutPitch Output bitmap pitch
		 * @return True if bitmap is available, false otherwise
		 */
		[[nodiscard]] static Bool
		GetGlyphBitmap(FFace I_Face, TArray<FByte>& I_OutBitmapData, UInt32& I_OutWidth, UInt32& I_OutHeight, UInt32& I_OutPitch);

		/**
		 * Gets glyph outline from the currently loaded glyph.
		 * @param I_Face Face handle (must have a glyph loaded)
		 * @return Glyph outline, or nullopt if not available
		 */
		[[nodiscard]] static TOptional<FGlyphOutline>
		GetGlyphOutline(FFace I_Face);

		/**
		 * Gets the format of the currently loaded glyph.
		 * @param I_Face Face handle (must have a glyph loaded)
		 * @return Glyph format (e.g., GlyphFormatOutline), or -1 if invalid
		 */
		[[nodiscard]] static Int32
		GetGlyphFormat(FFace I_Face);

		/**
		 * Gets the units per EM from a face.
		 * @param I_Face Face handle
		 * @return Units per EM, or 0 if invalid
		 */
		[[nodiscard]] static UInt32
		GetUnitsPerEM(FFace I_Face);

		/**
		 * Gets the outline from the currently loaded glyph.
		 * @param I_Face Face handle (must have a glyph loaded)
		 * @return Pointer to outline, or nullptr if not available
		 */
		[[nodiscard]] static const FOutline*
		GetGlyphOutlinePtr(FFace I_Face);

		/**
		 * Decomposes a FreeType outline using callback functions.
		 * This is a wrapper around FT_Outline_Decompose.
		 * @param I_Outline Outline to decompose
		 * @param I_MoveTo Move-to callback function
		 * @param I_LineTo Line-to callback function
		 * @param I_ConicTo Conic-to callback function
		 * @param I_CubicTo Cubic-to callback function
		 * @param I_User User data passed to callbacks
		 * @return True if successful, false otherwise
		 */
		[[nodiscard]] static Bool
		DecomposeOutline(
			const FOutline* I_Outline,
			FOutlineMoveToFunc I_MoveTo,
			FOutlineLineToFunc I_LineTo,
			FOutlineConicToFunc I_ConicTo,
			FOutlineCubicToFunc I_CubicTo,
			void* I_User
		);


	private:
		FFreeType();
		~FFreeType();

		// Non-copyable, non-movable
		FFreeType(const FFreeType&) = delete;
		FFreeType& operator=(const FFreeType&) = delete;
		FFreeType(FFreeType&&) = delete;
		FFreeType& operator=(FFreeType&&) = delete;

		FLibrary Library{nullptr};
	};

	// FFreeType implementation
	FFreeType& FFreeType::
	Get()
	{
		static FFreeType Instance;
		return Instance;
	}

	FFreeType::
	FFreeType()
	{
		const Error InitError = FT_Init_FreeType(&Library);
		if (InitError != ErrOk)
		{
			LOG_ERROR("Failed to initialize FreeType library: {}", InitError);
			Library = nullptr;
		}
	}

	FFreeType::
	~FFreeType()
	{
		if (Library != nullptr)
		{
			FT_Done_FreeType(Library);
			Library = nullptr;
		}
	}

	FFreeType::FLibrary FFreeType::
	GetLibrary()
	{
		return Get().Library;
	}

	Bool FFreeType::
	IsInitialized()
	{
		return Get().Library != nullptr;
	}

	TOptional<FFontFaceInfo> FFreeType::
	Load(const FPath& I_Path, Int32 I_FaceIndex, FFace& I_OutFace)
	{
		FLibrary Library = GetLibrary();
		if (Library == nullptr)
		{
			LOG_ERROR("FreeType library is not initialized!");
			return NullOpt;
		}

		const Error LoadError = FT_New_Face(Library, I_Path.GetString().Data(), I_FaceIndex, &I_OutFace);
		if (LoadError != ErrOk)
		{
			LOG_ERROR("Failed to load font face from file {}: {}", I_Path, LoadError);
			return NullOpt;
		}

		FFontFaceInfo FaceInfo;
		FaceInfo.FamilyName = I_OutFace->family_name ? I_OutFace->family_name : "";
		FaceInfo.StyleName = I_OutFace->style_name ? I_OutFace->style_name : "";
		FaceInfo.FaceCount = I_OutFace->num_faces;
		FaceInfo.FaceIndex = I_OutFace->face_index;
		
		LOG_INFO("Font face loaded: FaceCount={}, FaceIndex={}", FaceInfo.FaceCount, FaceInfo.FaceIndex);

		// Detect font format using FreeType's public API
		const char* FormatName = FT_Get_Font_Format(I_OutFace);
		if (FormatName != nullptr)
		{
			const FString FormatStr = FormatName;
			if (FormatStr == "TrueType" || FormatStr == "TTF")
			{ FaceInfo.Format = EFontFormat::TrueType; }
			else if (FormatStr == "Type 1" || FormatStr == "PFA" || FormatStr == "PFB")
			{ FaceInfo.Format = EFontFormat::Type1; }
			else if (FormatStr == "CFF")
			{ FaceInfo.Format = EFontFormat::CFF; }
			else if (FormatStr == "CID Type 1" || FormatStr == "CID Type 2")
			{ FaceInfo.Format = EFontFormat::CID; }
			else if (FormatStr == "BDF")
			{ FaceInfo.Format = EFontFormat::BDF; }
			else if (FormatStr == "PCF")
			{ FaceInfo.Format = EFontFormat::PCF; }
			else if (FormatStr == "Windows FNT")
			{ FaceInfo.Format = EFontFormat::WindowsFNT; }
			else if (FormatStr == "OpenType" || FormatStr == "OTF")
			{ FaceInfo.Format = EFontFormat::OpenType; }
			else if (FormatStr == "SFNT")
			{ FaceInfo.Format = EFontFormat::SFNT; }
			else
			{ FaceInfo.Format = EFontFormat::Invalid; }
		}
		else
		{
			FaceInfo.Format = EFontFormat::Invalid;
		}

		return TOptional<FFontFaceInfo>(FaceInfo);
	}

	TOptional<FFontFaceInfo> FFreeType::
	Load(const TArray<FByte>& I_Data, Int32 I_FaceIndex, FFace& I_OutFace, TArray<FByte>& I_OutMemoryBuffer)
	{
		FLibrary Library = GetLibrary();
		if (Library == nullptr)
		{
			LOG_ERROR("FreeType library is not initialized!");
			return NullOpt;
		}

		// FreeType requires the data to remain valid for the lifetime of the face
		// Copy the data to the provided buffer
		I_OutMemoryBuffer = I_Data;

		const Error LoadError = FT_New_Memory_Face(
			Library,
			reinterpret_cast<const FByte*>(I_OutMemoryBuffer.Data()),
			static_cast<Long>(I_OutMemoryBuffer.GetSize()),
			I_FaceIndex,
			&I_OutFace
		);

		if (LoadError != ErrOk)
		{
			LOG_ERROR("Failed to load font face from memory: {}", LoadError);
			I_OutMemoryBuffer.Clear();
			return NullOpt;
		}

		FFontFaceInfo FaceInfo;
		FaceInfo.FamilyName = I_OutFace->family_name ? I_OutFace->family_name : "";
		FaceInfo.StyleName = I_OutFace->style_name ? I_OutFace->style_name : "";
		FaceInfo.FaceCount = I_OutFace->num_faces;
		FaceInfo.FaceIndex = I_OutFace->face_index;
		
		LOG_INFO("Font face loaded: FaceCount={}, FaceIndex={}", FaceInfo.FaceCount, FaceInfo.FaceIndex);

		// Detect font format using FreeType's public API
		const char* FormatName = FT_Get_Font_Format(I_OutFace);
		if (FormatName != nullptr)
		{
			const FString FormatStr = FormatName;
			if (FormatStr == "TrueType" || FormatStr == "TTF")
			{ FaceInfo.Format = EFontFormat::TrueType; }
			else if (FormatStr == "Type 1" || FormatStr == "PFA" || FormatStr == "PFB")
			{ FaceInfo.Format = EFontFormat::Type1; }
			else if (FormatStr == "CFF")
			{ FaceInfo.Format = EFontFormat::CFF; }
			else if (FormatStr == "CID Type 1" || FormatStr == "CID Type 2")
			{ FaceInfo.Format = EFontFormat::CID; }
			else if (FormatStr == "BDF")
			{ FaceInfo.Format = EFontFormat::BDF; }
			else if (FormatStr == "PCF")
			{ FaceInfo.Format = EFontFormat::PCF; }
			else if (FormatStr == "Windows FNT")
			{ FaceInfo.Format = EFontFormat::WindowsFNT; }
			else if (FormatStr == "OpenType" || FormatStr == "OTF")
			{ FaceInfo.Format = EFontFormat::OpenType; }
			else if (FormatStr == "SFNT")
			{ FaceInfo.Format = EFontFormat::SFNT; }
			else
			{ FaceInfo.Format = EFontFormat::Invalid; }
		}
		else
		{
			FaceInfo.Format = EFontFormat::Invalid;
		}

		return TOptional<FFontFaceInfo>(FaceInfo);
	}

	void FFreeType::
	DoneFace(FFace I_Face)
	{
		if (I_Face != nullptr)
		{
			FT_Done_Face(I_Face);
		}
	}

	Bool FFreeType::
	SetPixelSizes(FFace I_Face, UInt32 I_Size)
	{
		if (I_Face == nullptr)
		{
			return False;
		}

		const Error SetError = FT_Set_Pixel_Sizes(I_Face, I_Size, I_Size);
		return SetError == ErrOk;
	}

	Bool FFreeType::
	SetCharSize(FFace I_Face, UInt32 I_Size)
	{
		if (I_Face == nullptr)
		{
			return False;
		}

		const Error SetError = FT_Set_Char_Size(I_Face, I_Size * 64, I_Size * 64, 72, 72);
		return SetError == ErrOk;
	}

	FFreeType::UInt FFreeType::
	GetCharIndex(FFace I_Face, UInt32 I_CodePoint)
	{
		if (I_Face == nullptr)
		{
			return 0;
		}

		return FT_Get_Char_Index(I_Face, I_CodePoint);
	}

	Bool FFreeType::
	LoadGlyph(FFace I_Face, UInt I_GlyphIndex, UInt32 I_LoadFlags)
	{
		if (I_Face == nullptr)
		{
			return False;
		}

		const Error LoadError = FT_Load_Glyph(I_Face, I_GlyphIndex, I_LoadFlags);
		return LoadError == ErrOk;
	}

	FGlyphMetrics FFreeType::
	GetGlyphMetrics(FFace I_Face)
	{
		FGlyphMetrics Metrics{};

		if (I_Face == nullptr || I_Face->glyph == nullptr)
		{
			return Metrics;
		}

		Metrics.Width = static_cast<UInt32>(I_Face->glyph->metrics.width >> 6);
		Metrics.Height = static_cast<UInt32>(I_Face->glyph->metrics.height >> 6);
		Metrics.BearingX = I_Face->glyph->metrics.horiBearingX >> 6;
		Metrics.BearingY = I_Face->glyph->metrics.horiBearingY >> 6;
		Metrics.AdvanceX = I_Face->glyph->metrics.horiAdvance >> 6;
		Metrics.AdvanceY = I_Face->glyph->metrics.vertAdvance >> 6;

		return Metrics;
	}

	Bool FFreeType::
	GetGlyphBitmap(FFace I_Face, TArray<FByte>& I_OutBitmapData, UInt32& I_OutWidth, UInt32& I_OutHeight, UInt32& I_OutPitch)
	{
		if (I_Face == nullptr || I_Face->glyph == nullptr)
		{
			return False;
		}

		const FT_Bitmap& Bitmap = I_Face->glyph->bitmap;
		I_OutWidth = Bitmap.width;
		I_OutHeight = Bitmap.rows;
		I_OutPitch = Bitmap.pitch;

		if (Bitmap.buffer != nullptr && Bitmap.width > 0 && Bitmap.rows > 0)
		{
			const UInt32 DataSize = static_cast<UInt32>(Bitmap.pitch) * Bitmap.rows;
			I_OutBitmapData.Resize(DataSize);
			std::memcpy(I_OutBitmapData.Data(), Bitmap.buffer, DataSize);
			return True;
		}

		return False;
	}

	TOptional<FGlyphOutline> FFreeType::
	GetGlyphOutline(FFace I_Face)
	{
		if (I_Face == nullptr || I_Face->glyph == nullptr)
		{
			return NullOpt;
		}

		const FT_Outline& Outline = I_Face->glyph->outline;
		FGlyphOutline Result;

		// Copy points
		Result.Points.Resize(Outline.n_points);
		for (Int32 i = 0; i < Outline.n_points; ++i)
		{
			Result.Points[i].X = Outline.points[i].x;
			Result.Points[i].Y = Outline.points[i].y;
			Result.Points[i].Tag = Outline.tags[i];
		}

		// Copy contour ends
		Result.ContourEnds.Resize(Outline.n_contours);
		for (Int32 i = 0; i < Outline.n_contours; ++i)
		{
			Result.ContourEnds[i] = Outline.contours[i];
		}

		return TOptional<FGlyphOutline>(Result);
	}

	Int32 FFreeType::
	GetGlyphFormat(FFace I_Face)
	{
		if (I_Face == nullptr || I_Face->glyph == nullptr)
		{
			return -1;
		}

		return static_cast<Int32>(I_Face->glyph->format);
	}

	UInt32 FFreeType::
	GetUnitsPerEM(FFace I_Face)
	{
		if (I_Face == nullptr)
		{
			return 0;
		}

		return I_Face->units_per_EM;
	}

	const FFreeType::FOutline* FFreeType::
	GetGlyphOutlinePtr(FFace I_Face)
	{
		if (I_Face == nullptr || I_Face->glyph == nullptr)
		{
			return nullptr;
		}

		return &I_Face->glyph->outline;
	}

	Bool FFreeType::
	DecomposeOutline(
		const FOutline* I_Outline,
		FOutlineMoveToFunc I_MoveTo,
		FOutlineLineToFunc I_LineTo,
		FOutlineConicToFunc I_ConicTo,
		FOutlineCubicToFunc I_CubicTo,
		void* I_User
	)
	{
		if (!I_Outline || !I_MoveTo || !I_LineTo || !I_ConicTo || !I_CubicTo)
		{
			return False;
		}

		// Setup FT_Outline_Funcs structure
		FOutlineFuncs Funcs;
		Funcs.move_to = I_MoveTo;
		Funcs.line_to = I_LineTo;
		Funcs.conic_to = I_ConicTo;
		Funcs.cubic_to = I_CubicTo;
		Funcs.shift = 0;  // No shift (coordinates are in font units)
		Funcs.delta = 0;   // No delta

		// Decompose outline using FreeType's official API
		// Note: We need to cast away const because FT_Outline_Decompose takes non-const,
		// but it doesn't modify the outline structure
		const Error DecomposeError = FT_Outline_Decompose(const_cast<FOutline*>(I_Outline), &Funcs, I_User);
		return DecomposeError == ErrOk;
	}
}
