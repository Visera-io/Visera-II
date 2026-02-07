module;
#include <Visera-Forge.hpp>
#include <msdfgen.h>
export module Visera.Forge.Baking.Font;
#define VISERA_MODULE_NAME "Forge.Baking"
import Visera.AssetHub.Font;
import Visera.AssetHub.Font.FreeType;
import Visera.Core.Image;
import Visera.Core.Types.Array;
import Visera.Core.Types.Pointer;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Algorithm.Ranges;
import Visera.Core.Math.Arithmetic.Operation;
import Visera.Global.Log;

export namespace Visera::Forge
{
	/**
	 * Configuration for MSDF font atlas generation.
	 */
	struct VISERA_FORGE_API FMSDFAtlasConfig
	{
		/** Font size in pixels. */
		Float FontSize{32.0f};
		/** Atlas width in pixels. */
		UInt32 AtlasWidth{1024};
		/** Atlas height in pixels. */
		UInt32 AtlasHeight{1024};
		/** Range for MSDF generation (distance field range in pixels). 
		 *  Recommended: 4-8 for 32px font size. Should be >= 2.
		 */
		Float Range{4.0f};
		/** Scale for MSDF generation. */
		Float Scale{1.0f};
		/** Characters to include in the atlas. */
		TArray<UInt32> CharacterSet;
		/** Border pixels around each glyph bitmap (for MSDF range safety margin).
		 *  Should be >= Range + 2 to prevent edge artifacts.
		 *  Recommended: Range + 4 for better safety margin.
		 *  This is added to the bitmap size during MSDF generation, not during packing.
		 */
		UInt32 BorderPx{8};
		/** Additional spacing between glyphs in atlas (optional, default 0).
		 *  This is added during packing to create gaps between glyphs.
		 *  Note: BorderPx already provides spacing, this is for extra spacing if needed.
		 */
		UInt32 SpacingPx{0};
		/** Angle threshold for edge coloring (in radians). */
		Float AngleThreshold{3.0};
	};

	/**
	 * Represents a glyph's position in the atlas.
	 */
	struct VISERA_FORGE_API FGlyphAtlasEntry
	{
		/** Unicode code point. */
		UInt32 CodePoint{0};
		/** X position in atlas (normalized 0-1). */
		Float AtlasU{0.0f};
		/** Y position in atlas (normalized 0-1). */
		Float AtlasV{0.0f};
		/** Width in atlas (normalized 0-1). */
		Float AtlasWidth{0.0f};
		/** Height in atlas (normalized 0-1). */
		Float AtlasHeight{0.0f};
		/** Glyph metrics. */
		FGlyphMetrics Metrics;
	};

	/**
	 * Result of MSDF font atlas generation.
	 */
	struct VISERA_FORGE_API FMSDFAtlasResult
	{
		/** Generated atlas image (MSDF format: RGBA32_Float). */
		TSharedPtr<FImage> AtlasImage;
		/** Glyph entries in the atlas. */
		TArray<FGlyphAtlasEntry> GlyphEntries;
		/** Font size used. */
		Float FontSize{0.0f};
		/** Range used for MSDF generation. */
		Float Range{0.0f};
	};

	/**
	 * Font baker for generating MSDF font atlases.
	 * Converts FreeType font glyphs to MSDF textures.
	 */
	class VISERA_FORGE_API FFontBaker
	{
	public:
		/**
		 * Generates an MSDF font atlas from a font face.
		 * @param I_Font Font face to generate atlas from
		 * @param I_Config Configuration for atlas generation
		 * @return MSDF atlas result, or nullopt on failure
		 */
		[[nodiscard]] TOptional<FMSDFAtlasResult>
		BakeAtlas(TSharedPtr<FFont> I_Font, const FMSDFAtlasConfig& I_Config);

	private:
		/**
		 * Converts FreeType outline to msdfgen Shape using FFreeType::DecomposeOutline.
		 * Matches official readFreetypeOutline implementation.
		 * @param I_Face Face handle
		 * @param I_CoordinateScale Scale to apply during conversion (font units to normalized units)
		 * @param I_Shape Output msdfgen shape
		 * @return True if successful, False otherwise
		 */
		[[nodiscard]] Bool
		ConvertOutlineToShape(const FFreeType::FFace I_Face, Double I_CoordinateScale, msdfgen::Shape& I_Shape);

		/**
		 * Generates MSDF for a single glyph.
		 * @param I_Font Font face
		 * @param I_CodePoint Unicode code point
		 * @param I_Config Configuration
		 * @param I_Output Output bitmap
		 * @param I_Metrics Output glyph metrics
		 * @return True if successful, False otherwise
		 */
		[[nodiscard]] Bool
		GenerateGlyphMSDF(
			FFont* I_Font,
			UInt32 I_CodePoint,
			const FMSDFAtlasConfig& I_Config,
			msdfgen::Bitmap<Float, 4>& I_Output,
			FGlyphMetrics& I_Metrics
		);

		/**
		 * Packs glyphs into an atlas using a simple bin-packing algorithm.
		 * @param I_GlyphSizes Array of glyph sizes (width, height)
		 * @param I_AtlasWidth Atlas width
		 * @param I_AtlasHeight Atlas height
		 * @param I_Spacing Additional spacing between glyphs (border is already included in bitmap sizes)
		 * @param I_Positions Output array of positions for each glyph
		 * @return True if all glyphs fit, False otherwise
		 */
		[[nodiscard]] Bool
		PackGlyphs(
			const TArray<msdfgen::Vector2>& I_GlyphSizes,
			UInt32 I_AtlasWidth,
			UInt32 I_AtlasHeight,
			UInt32 I_Spacing,
			TArray<msdfgen::Vector2>& I_Positions
		);
	};

	TOptional<FMSDFAtlasResult> FFontBaker::
	BakeAtlas(TSharedPtr<FFont> I_Font, const FMSDFAtlasConfig& I_Config)
	{
		if (!I_Font || !I_Font->IsLoaded())
		{
			LOG_ERROR("Invalid font face provided!");
			return NullOpt;
		}

		// Set font pixel size for metrics calculation
		// Note: We use no-hinting metrics (consistent with outline generation) to ensure
		// visual glyph and layout metrics match, avoiding "floating" or "squeezed" characters
		if (!I_Font->SetPixelSize(static_cast<UInt32>(I_Config.FontSize)))
		{
			LOG_ERROR("Failed to set font size!");
			return NullOpt;
		}

		// Prepare character set (default to ASCII printable if empty)
		TArray<UInt32> CharacterSet = I_Config.CharacterSet;
		if (CharacterSet.IsEmpty())
		{
			// Default to ASCII printable characters
			CharacterSet.Resize(95); // 32-126
			for (UInt32 i = 0; i < 95; ++i)
			{
				CharacterSet[i] = 32 + i;
			}
		}

		// Generate MSDF for each glyph
		TArray<msdfgen::Bitmap<Float, 4>> GlyphBitmaps;
		TArray<FGlyphMetrics> GlyphMetrics;
		TArray<msdfgen::Vector2> GlyphSizes;
		TArray<UInt32> GeneratedCodePoints;

		for (UInt32 CodePoint : CharacterSet)
		{
			// Check if glyph exists using FFont API
			if (!I_Font->HasGlyph(CodePoint))
			{ continue; }

			msdfgen::Bitmap<Float, 4> GlyphBitmap;
			FGlyphMetrics Metrics;

			if (!GenerateGlyphMSDF(I_Font.Get(), CodePoint, I_Config, GlyphBitmap, Metrics))
			{
				LOG_WARN("Failed to generate MSDF for code point: {}", CodePoint);
				continue;
			}

			GlyphBitmaps.EmplaceBack(std::move(GlyphBitmap));
			GlyphMetrics.EmplaceBack(Metrics);
			
			// Use actual bitmap dimensions for packing (includes padding added in GenerateGlyphMSDF)
			// This ensures pack positions match the actual bitmap size used during copy
			// Metrics.Width/Height are for layout (advance), not for atlas packing
			GlyphSizes.EmplaceBack(msdfgen::Vector2(
				GlyphBitmaps.Back().width(),
				GlyphBitmaps.Back().height())
			);
			GeneratedCodePoints.EmplaceBack(CodePoint);
		}

		if (GlyphBitmaps.IsEmpty())
		{
			LOG_ERROR("No glyphs were successfully generated!");
			return NullOpt;
		}

		// Sort glyphs by height (descending) before packing to improve atlas utilization
		// Create index array for sorting
		TArray<UInt32> GlyphIndices;
		GlyphIndices.Resize(GlyphSizes.GetSize());
		for (UInt32 i = 0; i < GlyphIndices.GetSize(); ++i)
		{
			GlyphIndices[i] = i;
		}

		// Sort indices by glyph height (descending)
		Algorithm::Sort(GlyphIndices, [&GlyphSizes](UInt32 I_A, UInt32 I_B)
		{
			return GlyphSizes[I_A].y > GlyphSizes[I_B].y;
		});

		// Create sorted arrays
		TArray<msdfgen::Bitmap<Float, 4>> SortedGlyphBitmaps;
		TArray<FGlyphMetrics> SortedGlyphMetrics;
		TArray<msdfgen::Vector2> SortedGlyphSizes;
		TArray<UInt32> SortedGeneratedCodePoints;
		SortedGlyphBitmaps.Resize(GlyphBitmaps.GetSize());
		SortedGlyphMetrics.Resize(GlyphMetrics.GetSize());
		SortedGlyphSizes.Resize(GlyphSizes.GetSize());
		SortedGeneratedCodePoints.Resize(GeneratedCodePoints.GetSize());

		for (UInt32 i = 0; i < GlyphIndices.GetSize(); ++i)
		{
			const UInt32 OriginalIndex = GlyphIndices[i];
			SortedGlyphBitmaps[i] = std::move(GlyphBitmaps[OriginalIndex]);
			SortedGlyphMetrics[i] = GlyphMetrics[OriginalIndex];
			SortedGlyphSizes[i] = GlyphSizes[OriginalIndex];
			SortedGeneratedCodePoints[i] = GeneratedCodePoints[OriginalIndex];
		}

		// Pack glyphs into atlas (now sorted by height)
		TArray<msdfgen::Vector2> GlyphPositions;
		if (!PackGlyphs(SortedGlyphSizes, I_Config.AtlasWidth, I_Config.AtlasHeight, I_Config.SpacingPx, GlyphPositions))
		{
			LOG_ERROR("Failed to pack glyphs into atlas!");
			return NullOpt;
		}

		// Create atlas image
		auto AtlasImage = MakeShared<FImage>(FImage::FCreateInfo
		{
			.Width = I_Config.AtlasWidth,
			.Height = I_Config.AtlasHeight,
			.Depth = 1,
			.PixelFormat = EPixelFormat::RGBA32_Float,
			.ColorSpace = EColorSpace::Linear,
		});

		// Clear atlas to zero
		std::memset(AtlasImage->AccessData(), 0, AtlasImage->GetSizeInBytes());

		// Copy glyphs into atlas
		FMSDFAtlasResult Result;
		Result.FontSize = I_Config.FontSize;
		Result.Range = I_Config.Range;
		Result.AtlasImage = AtlasImage;

		const Float InvAtlasWidth = 1.0f / static_cast<Float>(I_Config.AtlasWidth);
		const Float InvAtlasHeight = 1.0f / static_cast<Float>(I_Config.AtlasHeight);

		for (UInt32 i = 0; i < SortedGlyphBitmaps.GetSize(); ++i)
		{
			const msdfgen::Bitmap<Float, 4>& GlyphBitmap = SortedGlyphBitmaps[i];
			const msdfgen::Vector2& Position = GlyphPositions[i];
			const FGlyphMetrics& Metrics = SortedGlyphMetrics[i];

			const UInt32 GlyphWidth = GlyphBitmap.width();
			const UInt32 GlyphHeight = GlyphBitmap.height();

			// Skip empty glyphs (0x0) - they don't need pixels in atlas
			// Empty glyphs are layout-only (spaces, tabs, control characters)
			if (GlyphWidth == 0 || GlyphHeight == 0)
			{
				// Create atlas entry with zero size (no UV coordinates needed)
				FGlyphAtlasEntry Entry;
				Entry.CodePoint = SortedGeneratedCodePoints[i];
				Entry.AtlasU = 0.0f;
				Entry.AtlasV = 0.0f;
				Entry.AtlasWidth = 0.0f;
				Entry.AtlasHeight = 0.0f;
				Entry.Metrics = Metrics;
				Result.GlyphEntries.EmplaceBack(Entry);
				continue;
			}

			const UInt32 X = static_cast<UInt32>(Position.x);
			const UInt32 Y = static_cast<UInt32>(Position.y);

			// Assert that pack positions are correct - if this fails, it's a pack bug, not a copy bug
			// We should never silently clamp, as that hides the real problem
			VISERA_ASSERT(X + GlyphWidth <= I_Config.AtlasWidth);
			VISERA_ASSERT(Y + GlyphHeight <= I_Config.AtlasHeight);

			// Copy glyph bitmap to atlas (full copy, no clamping)
			for (UInt32 Gy = 0; Gy < GlyphHeight; ++Gy)
			{
				for (UInt32 Gx = 0; Gx < GlyphWidth; ++Gx)
				{
					const UInt32 AtlasX = X + Gx;
					const UInt32 AtlasY = Y + Gy;

					const auto* GlyphPixel = GlyphBitmap(Gx, Gy);
					Float* AtlasPixel = reinterpret_cast<Float*>(
						AtlasImage->AccessData() + 
						(AtlasY * I_Config.AtlasWidth + AtlasX) * sizeof(Float) * 4
					);
					AtlasPixel[0] = GlyphPixel[0]; // R
					AtlasPixel[1] = GlyphPixel[1]; // G
					AtlasPixel[2] = GlyphPixel[2]; // B
					AtlasPixel[3] = GlyphPixel[3]; // A (True distance)
				}
			}

			// Create atlas entry
			FGlyphAtlasEntry Entry;
			Entry.CodePoint = SortedGeneratedCodePoints[i];
			Entry.AtlasU = static_cast<Float>(X) * InvAtlasWidth;
			Entry.AtlasV = static_cast<Float>(Y) * InvAtlasHeight;
			Entry.AtlasWidth = static_cast<Float>(GlyphWidth) * InvAtlasWidth;
			Entry.AtlasHeight = static_cast<Float>(GlyphHeight) * InvAtlasHeight;
			Entry.Metrics = Metrics;
			Result.GlyphEntries.EmplaceBack(Entry);
		}

		return TOptional<FMSDFAtlasResult>(Result);
	}

	// Helper structure for outline decomposition callbacks
	// Matches official MSDFGen FtContext structure
	struct FOutlineDecomposeContext
	{
		Double Scale;  // Coordinate scaling (font units to normalized/legacy units)
		msdfgen::Point2 Position;
		msdfgen::Shape* Shape;
		msdfgen::Contour* Contour;
	};

namespace Private
{
	using namespace Visera;
	
	// Helper function to convert FT_Vector to Point2 with scaling (matches official ftPoint2)
	msdfgen::Point2
	FtPoint2(const FFreeType::FVector& I_Vector, Double I_Scale)
	{
		return msdfgen::Point2(I_Scale * I_Vector.x, I_Scale * I_Vector.y);
	}

	// Outline decomposition callback: move to (matches official ftMoveTo)
	Int32
	OutlineMoveTo(const FFreeType::FVector* I_To, void* I_User)
	{
		FOutlineDecomposeContext* Context = static_cast<FOutlineDecomposeContext*>(I_User);
		if (!Context || !I_To)
		{ return 1; }

		// Start a new contour only if the previous one is not empty
		// This matches the official MSDFGen implementation exactly
		if (!(Context->Contour && Context->Contour->edges.empty()))
		{
			Context->Contour = &Context->Shape->addContour();
		}
		Context->Position = FtPoint2(*I_To, Context->Scale);
		return 0;
	}

	// Outline decomposition callback: line to (matches official ftLineTo)
	Int32
	OutlineLineTo(const FFreeType::FVector* I_To, void* I_User)
	{
		FOutlineDecomposeContext* Context = static_cast<FOutlineDecomposeContext*>(I_User);
		if (!Context || !I_To || !Context->Contour)
		{ return 1; }

		msdfgen::Point2 Endpoint = FtPoint2(*I_To, Context->Scale);
		if (Endpoint != Context->Position)
		{
			Context->Contour->addEdge(msdfgen::EdgeHolder(Context->Position, Endpoint));
			Context->Position = Endpoint;
		}
		return 0;
	}

	// Outline decomposition callback: conic to (quadratic Bezier) (matches official ftConicTo)
	Int32
	OutlineConicTo(const FFreeType::FVector* I_Control, const FFreeType::FVector* I_To, void* I_User)
	{
		FOutlineDecomposeContext* Context = static_cast<FOutlineDecomposeContext*>(I_User);
		if (!Context || !I_Control || !I_To || !Context->Contour)
		{ return 1; }

		msdfgen::Point2 Endpoint = FtPoint2(*I_To, Context->Scale);
		if (Endpoint != Context->Position)
		{
			Context->Contour->addEdge(msdfgen::EdgeHolder(
				Context->Position,
				FtPoint2(*I_Control, Context->Scale),
				Endpoint
			));
			Context->Position = Endpoint;
		}
		return 0;
	}

	// Outline decomposition callback: cubic to (cubic Bezier) (matches official ftCubicTo)
	Int32
	OutlineCubicTo(const FFreeType::FVector* I_Control1, const FFreeType::FVector* I_Control2, const FFreeType::FVector* I_To, void* I_User)
	{
		FOutlineDecomposeContext* Context = static_cast<FOutlineDecomposeContext*>(I_User);
		if (!Context || !I_Control1 || !I_Control2 || !I_To || !Context->Contour)
		{ return 1; }

		msdfgen::Point2 Endpoint = FtPoint2(*I_To, Context->Scale);
		if (Endpoint != Context->Position || msdfgen::crossProduct(
			FtPoint2(*I_Control1, Context->Scale) - Endpoint,
			FtPoint2(*I_Control2, Context->Scale) - Endpoint))
		{
			Context->Contour->addEdge(msdfgen::EdgeHolder(
				Context->Position,
				FtPoint2(*I_Control1, Context->Scale),
				FtPoint2(*I_Control2, Context->Scale),
				Endpoint
			));
			Context->Position = Endpoint;
		}
		return 0;
	}
}

	/**
	 * Gets font coordinate scale (matches official getFontCoordinateScale).
	 * We use FONT_SCALING_NONE (scale = 1) to keep font units, then apply pixel scale in projection.
	 */
	Double
	GetFontCoordinateScale(UInt32 I_UnitsPerEM)
	{
		// Use FONT_SCALING_NONE: keep font units, apply pixel scale later in projection
		// This matches the approach where we want direct control over pixel scaling
		return 1.0;
	}

	Bool FFontBaker::
	ConvertOutlineToShape(const FFreeType::FFace I_Face, Double I_CoordinateScale, msdfgen::Shape& I_Shape)
	{
		if (!I_Face)
		{
			return False;
		}

		// Get outline pointer from FreeType
		const FFreeType::FOutline* Outline = FFreeType::GetGlyphOutlinePtr(I_Face);
		if (!Outline)
		{
			return False;
		}

		I_Shape.contours.clear();
		// Set Y-axis orientation to Y_UPWARD (matching official MSDFGen readFreetypeOutline)
		I_Shape.setYAxisOrientation(msdfgen::Y_UPWARD);

		// Allow empty outlines (e.g., space character) - return empty shape
		if (Outline->n_points == 0 || Outline->n_contours == 0)
		{
			return True;
		}

		// Setup context for callbacks (matches official FtContext)
		FOutlineDecomposeContext Context;
		Context.Scale = I_CoordinateScale;
		Context.Shape = &I_Shape;
		Context.Contour = nullptr;

		// Decompose outline using FFreeType wrapper
		if (!FFreeType::DecomposeOutline(Outline, Private::OutlineMoveTo, Private::OutlineLineTo, Private::OutlineConicTo, Private::OutlineCubicTo, &Context))
		{
			LOG_ERROR("Failed to decompose outline!");
			return False;
		}

		// Remove last empty contour if present (matching official MSDFGen readFreetypeOutline)
		if (!I_Shape.contours.empty() && I_Shape.contours.back().edges.empty())
		{
			I_Shape.contours.pop_back();
		}

		return True;
	}

	Bool FFontBaker::
	GenerateGlyphMSDF(
		FFont* I_Font,
		UInt32 I_CodePoint,
		const FMSDFAtlasConfig& I_Config,
		msdfgen::Bitmap<Float, 4>& I_Output,
		FGlyphMetrics& I_Metrics
	)
	{
		if (!I_Font || !I_Font->IsLoaded())
		{ return False; }

		// Initialize metrics
		I_Metrics = FGlyphMetrics{};

		// Get FreeType face
		auto Face = I_Font->GetFreeTypeFace();
		if (!Face)
		{
			LOG_ERROR("Failed to get FreeType face!");
			return False;
		}

		// Get units per EM
		const UInt32 UnitsPerEM = FFreeType::GetUnitsPerEM(Face);
		if (UnitsPerEM == 0)
		{
			LOG_ERROR("Invalid units per EM!");
			return False;
		}

		// Get glyph index
		const FFreeType::UInt GlyphIndex = FFreeType::GetCharIndex(Face, I_CodePoint);
		if (GlyphIndex == 0)
		{
			LOG_WARN("Glyph not found for code point: {}", I_CodePoint);
			return False;
		}

		// Load glyph with NO_SCALE (matches official loadGlyph)
		if (!FFreeType::LoadGlyph(Face, GlyphIndex, FFreeType::LoadNoScale | FFreeType::LoadNoHinting))
		{
			LOG_ERROR("Failed to load glyph for code point: {}", I_CodePoint);
			return False;
		}

		// Get coordinate scale (FONT_SCALING_NONE = 1, keep font units)
		const Double CoordinateScale = GetFontCoordinateScale(UnitsPerEM);

		// Convert outline to shape with coordinate scaling (matches official readFreetypeOutline)
		msdfgen::Shape Shape;
		if (!ConvertOutlineToShape(Face, CoordinateScale, Shape))
		{
			LOG_ERROR("Failed to convert outline to shape!");
			return False;
		}

		// Validate and normalize shape (matches official main flow)
		if (!Shape.validate())
		{
			LOG_ERROR("Invalid shape geometry for code point: {}", I_CodePoint);
			return False;
		}
		Shape.normalize();

		// Handle empty shapes
		if (Shape.contours.empty())
		{
			const TOptional<FGlyphMetrics> MetricsOpt = I_Font->GetGlyphMetrics(I_CodePoint);
			if (MetricsOpt.HasValue())
			{
				I_Metrics = MetricsOpt.GetValue();
				I_Output = msdfgen::Bitmap<Float, 4>(0, 0);
				return True;
			}
			else
			{
				LOG_WARN("Empty glyph and failed to get metrics for code point: {}", I_CodePoint);
				return False;
			}
		}

		// Edge coloring (matches official main flow)
		msdfgen::edgeColoringByDistance(Shape, I_Config.AngleThreshold, 0);

		// Get shape bounds (now in coordinate-scaled units, which is font units since scale=1)
		auto Bounds = Shape.getBounds();
		
		// Calculate glyph dimensions
		const Double GlyphWidth = Bounds.r - Bounds.l;
		const Double GlyphHeight = Bounds.t - Bounds.b;
		
		if (GlyphWidth <= 0 || GlyphHeight <= 0)
		{
			LOG_WARN("Glyph has zero or negative dimensions!");
			return False;
		}

		// Calculate pixel scale: convert from shape units to pixels
		// Since we use FONT_SCALING_NONE (scale=1), shape units = font units
		// Pixel scale = FontSize / UnitsPerEM
		const Double PixelScale = static_cast<Double>(I_Config.FontSize) / static_cast<Double>(UnitsPerEM);

		// Calculate bitmap size in pixels: scale shape units to pixels, add border
		const UInt32 GlyphWidthPx = static_cast<UInt32>(Math::Ceil(GlyphWidth * PixelScale)) + I_Config.BorderPx * 2;
		const UInt32 GlyphHeightPx = static_cast<UInt32>(Math::Ceil(GlyphHeight * PixelScale)) + I_Config.BorderPx * 2;

		// Update metrics
		I_Metrics.Width = static_cast<UInt32>(Math::Ceil(GlyphWidth * PixelScale));
		I_Metrics.Height = static_cast<UInt32>(Math::Ceil(GlyphHeight * PixelScale));
		
		// Get glyph metrics and scale to pixels
		FGlyphMetrics UnhintedMetrics = FFreeType::GetGlyphMetrics(Face);
		I_Metrics.AdvanceX = static_cast<Int32>(Math::Round(static_cast<Double>(UnhintedMetrics.AdvanceX) * PixelScale));
		I_Metrics.AdvanceY = static_cast<Int32>(Math::Round(static_cast<Double>(UnhintedMetrics.AdvanceY) * PixelScale));
		I_Metrics.BearingX = static_cast<Int32>(Math::Round(static_cast<Double>(UnhintedMetrics.BearingX) * PixelScale));
		I_Metrics.BearingY = static_cast<Int32>(Math::Round(static_cast<Double>(UnhintedMetrics.BearingY) * PixelScale));

		// Create bitmap
		I_Output = msdfgen::Bitmap<Float, 4>(GlyphWidthPx, GlyphHeightPx);

		// Calculate projection (matches official main flow)
		// Projection formula: pixel = scale * (coord + translate)
		// We need to map shape coordinates to pixel coordinates
		// Shape uses Y_UP orientation, bitmap uses Y_DOWN (top-left origin)
		
		// Scale: converts shape units to pixels
		const msdfgen::Vector2 Scale(PixelScale, PixelScale);
		
		// Translate: positions shape in bitmap (in shape units)
		// We want bounds.l to map to BorderPx, bounds.b to map to BorderPx
		// For Y: bounds.b (bottom in Y_UP) should map to BorderPx (top in Y_DOWN)
		// Since we're not flipping Y in scale, we need to account for Y_UP to Y_DOWN conversion
		// translateX = BorderPx/PixelScale - bounds.l
		// translateY = BorderPx/PixelScale - bounds.b
		// But wait, Projection uses: pixel = scale * (coord + translate)
		// So: BorderPx = PixelScale * (bounds.l + translateX)
		//     translateX = BorderPx/PixelScale - bounds.l
		const msdfgen::Vector2 Translate(
			static_cast<Double>(I_Config.BorderPx) / PixelScale - Bounds.l,
			static_cast<Double>(I_Config.BorderPx) / PixelScale - Bounds.b
		);

		// Create projection and transformation (matches official main flow)
		const msdfgen::Projection Projection(Scale, Translate);
		const msdfgen::Range Range(I_Config.Range);
		const msdfgen::SDFTransformation Transformation(Projection, Range);

		// Generate MSDF (matches official main flow)
		msdfgen::MSDFGeneratorConfig Config;
		Config.overlapSupport = True;
		msdfgen::generateMTSDF(I_Output, Shape, Transformation, Config);

		return True;
	}

	Bool FFontBaker::
	PackGlyphs(
		const TArray<msdfgen::Vector2>& I_GlyphSizes,
		UInt32 I_AtlasWidth,
		UInt32 I_AtlasHeight,
		UInt32 I_Spacing,
		TArray<msdfgen::Vector2>& I_Positions
	)
	{
		I_Positions.Clear();
		I_Positions.Resize(I_GlyphSizes.GetSize());

		// Simple bin-packing: place glyphs row by row
		UInt32 CurrentX = 0;
		UInt32 CurrentY = 0;
		UInt32 RowHeight = 0;

		for (UInt32 i = 0; i < I_GlyphSizes.GetSize(); ++i)
		{
			const msdfgen::Vector2& Size = I_GlyphSizes[i];
			const UInt32 GlyphWidth = static_cast<UInt32>(Size.x);
			const UInt32 GlyphHeight = static_cast<UInt32>(Size.y);

			// Skip empty glyphs (0x0) - they don't need positions in atlas
			// Position them at (0, 0) as a sentinel value
			if (GlyphWidth == 0 || GlyphHeight == 0)
			{
				I_Positions[i] = msdfgen::Vector2(0, 0);
				continue;
			}

			// Bitmap size already includes border from GenerateGlyphMSDF
			// Add additional spacing if specified
			const UInt32 GlyphWidthWithSpacing = GlyphWidth + I_Spacing;
			const UInt32 GlyphHeightWithSpacing = GlyphHeight + I_Spacing;

			// Check if glyph fits on current row
			if (CurrentX + GlyphWidthWithSpacing > I_AtlasWidth)
			{
				// Move to next row
				CurrentX = 0;
				CurrentY += RowHeight;
				RowHeight = 0;
			}

			// Check if glyph fits in atlas
			if (CurrentY + GlyphHeightWithSpacing > I_AtlasHeight)
			{
				LOG_ERROR("Atlas is too small to fit all glyphs!");
				return False;
			}

			// Position directly at CurrentX/CurrentY
			// Border is already in bitmap size, spacing is added between glyphs
			I_Positions[i] = msdfgen::Vector2(CurrentX, CurrentY);
			CurrentX += GlyphWidthWithSpacing;
			RowHeight = RowHeight >= GlyphHeightWithSpacing ? RowHeight : GlyphHeightWithSpacing;
		}

		return True;
	}
}
