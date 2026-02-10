module;
#include <Visera-Core.hpp>
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include <stb_image_resize2.h>
export module Visera.Core.Image;
#define VISERA_MODULE_NAME "Core.Image"
export import Visera.Core.Image.Pixel;
export import Visera.Core.Image.Common;
       import Visera.Core.Math.Color;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.Half;
       import Visera.Core.Types.Optional;
       import Visera.Core.OS.Memory;
       import Visera.Core.Math.Arithmetic.Operation;
       import Visera.Core.Math.Arithmetic.Interval;
       import Visera.Core.Math.Geometry.Point;

export namespace Visera
{
    // Forward declarations
    class FImage;
    class FImageView2D;
    class FImageView3D;

    /**
     * Iterator for a 2D view (rectangular region) of FImage.
     * Only iterates over pixels within the specified X and Y intervals on a given layer.
     * Provides read-write access to pixels.
     */
    class VISERA_CORE_API FImageViewIterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = FPixel;
        using difference_type = Int64;
        using pointer = FPixel*;
        using reference = FPixel&;

        FImageViewIterator() = default;
        explicit FImageViewIterator(
            FImageView2D* I_View,
            UInt64 I_Index = 0)
            : View{I_View}
            , PixelIndex{I_Index}
        {
        }

        [[nodiscard]] FPixel&
        operator*()
        {
            UpdatePixel();
            return CurrentPixel;
        }

        [[nodiscard]] const FPixel&
        operator*() const
        {
            UpdatePixel();
            return CurrentPixel;
        }

        FImageViewIterator&
        operator++()
        {
            ++PixelIndex;
            return *this;
        }

        FImageViewIterator
        operator++(Int32)
        {
            FImageViewIterator Temp = *this;
            ++(*this);
            return Temp;
        }

        FImageViewIterator&
        operator--()
        {
            --PixelIndex;
            return *this;
        }

        FImageViewIterator
        operator--(Int32)
        {
            FImageViewIterator Temp = *this;
            --(*this);
            return Temp;
        }

        FImageViewIterator&
        operator+=(difference_type I_N)
        {
            PixelIndex += I_N;
            return *this;
        }

        FImageViewIterator&
        operator-=(difference_type I_N)
        {
            PixelIndex -= I_N;
            return *this;
        }

        [[nodiscard]] FImageViewIterator
        operator+(difference_type I_N) const
        {
            FImageViewIterator Temp = *this;
            Temp += I_N;
            return Temp;
        }

        [[nodiscard]] FImageViewIterator
        operator-(difference_type I_N) const
        {
            FImageViewIterator Temp = *this;
            Temp -= I_N;
            return Temp;
        }

        [[nodiscard]] difference_type
        operator-(const FImageViewIterator& I_Other) const
        {
            return static_cast<difference_type>(PixelIndex) - static_cast<difference_type>(I_Other.PixelIndex);
        }

        [[nodiscard]] Bool
        operator==(const FImageViewIterator& I_Other) const
        {
            return View == I_Other.View &&
                   PixelIndex == I_Other.PixelIndex;
        }

        [[nodiscard]] Bool
        operator!=(const FImageViewIterator& I_Other) const
        {
            return !(*this == I_Other);
        }

        [[nodiscard]] Bool
        operator<(const FImageViewIterator& I_Other) const
        {
            return PixelIndex < I_Other.PixelIndex;
        }

        [[nodiscard]] Bool
        operator>(const FImageViewIterator& I_Other) const
        {
            return PixelIndex > I_Other.PixelIndex;
        }

        [[nodiscard]] Bool
        operator<=(const FImageViewIterator& I_Other) const
        {
            return PixelIndex <= I_Other.PixelIndex;
        }

        [[nodiscard]] Bool
        operator>=(const FImageViewIterator& I_Other) const
        {
            return PixelIndex >= I_Other.PixelIndex;
        }

    private:
        FImageView2D* View = nullptr;
        UInt64 PixelIndex = 0;
        mutable FPixel CurrentPixel{nullptr, EPixelFormat::Invalid, 0};
        mutable UInt64 CachedPixelIndex = UInt64(-1);

    private:
        [[nodiscard]] UInt64
        GetPixelCount() const;

        void
        UpdatePixel() const;
    };

    class VISERA_CORE_API FImageView2D
    {
        friend class FImageViewIterator;
    public:
        FImageView2D(const FImage& I_Image,
                     UInt32        I_Layer,
                     const FPoint2U& I_Min,
                     const FPoint2U& I_Max);

        [[nodiscard]] inline FImageViewIterator
        begin() const { return FImageViewIterator{const_cast<FImageView2D*>(this), 0}; }

        [[nodiscard]] FImageViewIterator
        end() const;

        template<Concepts::Color TColor = FColor>
        [[nodiscard]] Bool
        SetPixel(const FPoint2U& I_Pos, const TColor& I_Color);

        /** Returns FPixel at view-local (I_X, I_Y); use .GetColor() to read, .SetColor() or = to write. Enables View(x,y) = Color. */
        [[nodiscard]] FPixel
        operator()(UInt32 I_X, UInt32 I_Y);
        [[nodiscard]] FPixel
        operator()(UInt32 I_X, UInt32 I_Y) const;

        [[nodiscard]] inline UInt32 GetLayer() const { return Layer; }
        [[nodiscard]] inline TClosedInterval<UInt32> GetIntervalX() const { return IntervalX; }
        [[nodiscard]] inline TClosedInterval<UInt32> GetIntervalY() const { return IntervalY; }
        /** @return Pointer to the source image (for FImage construction from view). */
        [[nodiscard]] inline const FImage* GetImage() const { return Image; }
        /** @return 2D subview of rectangular region (I_Min, I_Max in image coords). */
        [[nodiscard]] inline FImageView2D
        Subview2D(const FPoint2U& I_Min, const FPoint2U& I_Max) const { return FImageView2D{*Image, Layer, I_Min, I_Max}; }

    private:
        FImage* Image = nullptr;
        TClosedInterval<UInt32> IntervalX{0, 0};
        TClosedInterval<UInt32> IntervalY{0, 0};
        UInt32 Layer = 0;
    };

    /**
     * Iterator for a 3D view (across multiple layers) of FImage.
     * Iterates over all pixels across all layers in the view.
     * Provides read-write access to pixels.
     */
    class VISERA_CORE_API FImageView3DIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = FPixel;
        using difference_type = Int64;
        using pointer = FPixel*;
        using reference = FPixel&;

        FImageView3DIterator() = default;
        explicit FImageView3DIterator(
            FImageView3D* I_View,
            UInt64 I_Index = 0)
            : View{I_View}
            , PixelIndex{I_Index}
        {
        }

        [[nodiscard]] FPixel&
        operator*()
        {
            UpdatePixel();
            return CurrentPixel;
        }

        [[nodiscard]] const FPixel&
        operator*() const
        {
            UpdatePixel();
            return CurrentPixel;
        }

        FImageView3DIterator&
        operator++()
        {
            ++PixelIndex;
            return *this;
        }

        FImageView3DIterator
        operator++(Int32)
        {
            FImageView3DIterator Temp = *this;
            ++(*this);
            return Temp;
        }

        [[nodiscard]] Bool
        operator==(const FImageView3DIterator& I_Other) const
        {
            return View == I_Other.View &&
                   PixelIndex == I_Other.PixelIndex;
        }

        [[nodiscard]] Bool
        operator!=(const FImageView3DIterator& I_Other) const
        {
            return !(*this == I_Other);
        }

    private:
        FImageView3D* View = nullptr;
        UInt64 PixelIndex = 0;
        mutable FPixel CurrentPixel{nullptr, EPixelFormat::Invalid, 0};
        mutable UInt64 CachedPixelIndex = UInt64(-1);

    private:
        [[nodiscard]] UInt64
        GetPixelCount() const;

        void
        UpdatePixel() const;
    };

    class VISERA_CORE_API FImageView3D
    {
        friend class FImageView3DIterator;
    public:
        FImageView3D(const FImage& I_Image, const FPoint3U& I_Min, const FPoint3U& I_Max);

        /** @return 2D subview of layer I_Layer in region (I_Min, I_Max). */
        [[nodiscard]] inline FImageView2D
        Subview2D(UInt32 I_Layer, const FPoint2U& I_Min, const FPoint2U& I_Max) const { return FImageView2D{*Image, I_Layer, I_Min, I_Max}; }
        /** @return 3D subview of region (I_Min, I_Max). */
        [[nodiscard]] inline FImageView3D
        Subview3D(const FPoint3U& I_Min, const FPoint3U& I_Max) const { return FImageView3D{*Image, I_Min, I_Max}; }

        [[nodiscard]] inline FImageView3DIterator
        begin() const { return FImageView3DIterator{const_cast<FImageView3D*>(this), 0}; }

        [[nodiscard]] FImageView3DIterator
        end() const;

        /** Returns FPixel at view-local (I_X, I_Y, I_Z); use .GetColor() to read, .SetColor() or = to write. Enables View(x,y,z) = Color. */
        [[nodiscard]] FPixel
        operator()(UInt32 I_X, UInt32 I_Y, UInt32 I_Z);
        [[nodiscard]] FPixel
        operator()(UInt32 I_X, UInt32 I_Y, UInt32 I_Z) const;

        [[nodiscard]] inline UInt32 GetMinLayer() const { return MinLayer; }
        [[nodiscard]] inline UInt32 GetMaxLayer() const { return MaxLayer; }
        [[nodiscard]] inline TClosedInterval<UInt32> GetIntervalX() const { return IntervalX; }
        [[nodiscard]] inline TClosedInterval<UInt32> GetIntervalY() const { return IntervalY; }

    private:
        FImage* Image = nullptr;
        TClosedInterval<UInt32> IntervalX{0, 0};
        TClosedInterval<UInt32> IntervalY{0, 0};
        UInt32 MinLayer = 0;
        UInt32 MaxLayer = 0;

        [[nodiscard]] UInt64
        GetPixelCount() const;
    };

    /**
     * Minimal image data container for raw pixel storage.
     */
    class VISERA_CORE_API FImage
    {
    public:
        struct FCreateInfo
        {
            UInt32          Width          {0};
            UInt32          Height         {0};
            UInt32          Depth          {1};
            EPixelFormat    PixelFormat    {EPixelFormat::RGBA8_UNorm};
            EColorSpace     ColorSpace     {EColorSpace::Linear};
            UInt32          RowPitchBytes  {0}; // Auto-calculated if 0 (tight packing)
            UInt32          SlicePitchBytes{0}; // Auto-calculated if 0 (RowPitchBytes * Height)

            std::pmr::memory_resource*
            MemoryArena = std::pmr::get_default_resource();
        };

        [[nodiscard]] inline FByte*
        AccessData() { return Data.Data(); }
        [[nodiscard]] inline const FByte*
        GetData() const { return Data.Data(); }
        [[nodiscard]] inline UInt32
        GetWidth() const { return Info.Width; }
        [[nodiscard]] inline UInt32
        GetHeight() const { return Info.Height; }
        [[nodiscard]] inline UInt32
        GetDepth() const { return Info.Depth; }
        [[nodiscard]] inline EPixelFormat
        GetPixelFormat() const { return Info.PixelFormat; }
        [[nodiscard]] inline EColorSpace
        GetColorSpace() const { return Info.ColorSpace; }
        [[nodiscard]] inline UInt32
        GetRowPitchBytes() const { return Info.RowPitchBytes > 0 ? Info.RowPitchBytes : Info.Width * FPixel::GetByteSize(Info.PixelFormat); }
        [[nodiscard]] inline UInt32
        GetSlicePitchBytes() const { return Info.SlicePitchBytes > 0 ? Info.SlicePitchBytes : GetRowPitchBytes() * Info.Height; }
        [[nodiscard]] inline UInt8
        GetBytesPerPixel() const { return FPixel::GetByteSize(Info.PixelFormat); }
        [[nodiscard]] inline UInt8
        GetChannelCount() const { return FPixel::GetChannelCount(Info.PixelFormat); }
        [[nodiscard]] inline UInt64
        GetSizeInBytes() const { return static_cast<UInt64>(GetSlicePitchBytes()) * Info.Depth; }

        [[nodiscard]] Bool
        Resize(UInt32 I_NewWidth, UInt32 I_NewHeight);

        [[nodiscard]] FImage
        Clone(EColorSpace I_NewColorSpace = EColorSpace::Unknown,
              EPixelFormat I_NewPixelFormat = EPixelFormat::Invalid,
              UInt32 I_NewWidth = ~0U,
              UInt32 I_NewHeight = ~0U,
              UInt32 I_NewDepth = ~0U) const;

        FImage(const FImageView3D& I_View, std::pmr::memory_resource* I_MemoryArena = std::pmr::get_default_resource());

        FImage(const FImageView2D& I_View, std::pmr::memory_resource* I_MemoryArena = std::pmr::get_default_resource());

        [[nodiscard]] TOptional<FPixel>
        GetPixel(const FPoint3U& I_Pos) const;

        [[nodiscard]] FImage
        Slice(const FPoint3U& I_Min, const FPoint3U& I_Max) const;

        [[nodiscard]] inline Bool
        IsRGBA() const { return Info.PixelFormat == EPixelFormat::RGBA8_UNorm || Info.PixelFormat == EPixelFormat::RGBA16_UNorm || Info.PixelFormat == EPixelFormat::RGBA16_Float || Info.PixelFormat == EPixelFormat::RGBA32_Float; }
        [[nodiscard]] inline Bool
        IsBGRA() const { return Info.PixelFormat == EPixelFormat::BGRA8_UNorm; }
        [[nodiscard]] inline Bool
        HasAlpha() const { return FPixel::HasAlpha(Info.PixelFormat); }
        [[nodiscard]] inline Bool
        IsFloatFormat() const { return FPixel::IsFloatFormat(Info.PixelFormat); }
        [[nodiscard]] inline Bool
        IsHalfFloatFormat() const { return Info.PixelFormat == EPixelFormat::R16_Float || Info.PixelFormat == EPixelFormat::RG16_Float || Info.PixelFormat == EPixelFormat::RGB16_Float || Info.PixelFormat == EPixelFormat::RGBA16_Float; }
        [[nodiscard]] inline Bool
        IsUNorm16Format() const { return Info.PixelFormat == EPixelFormat::R16_UNorm || Info.PixelFormat == EPixelFormat::RG16_UNorm || Info.PixelFormat == EPixelFormat::RGB16_UNorm || Info.PixelFormat == EPixelFormat::RGBA16_UNorm; }

        /**
         * Iterator for FImage that allows pixel-by-pixel access.
         * This iterator provides a view over the image data as pixels.
         */
        class VISERA_CORE_API FImagePixelIterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = FPixel;
            using difference_type = Int64;
            using pointer = FPixel*;
            using reference = FPixel&;

            FImagePixelIterator() = default;
            explicit FImagePixelIterator(FImage* I_Image, UInt64 I_Index = 0)
                : Image{I_Image}
                , PixelIndex{I_Index}
            {
            }

            [[nodiscard]] FPixel&
            operator*()
            {
                UpdatePixel();
                return CurrentPixel;
            }

            [[nodiscard]] const FPixel&
            operator*() const
            {
                UpdatePixel();
                return CurrentPixel;
            }

            FImagePixelIterator&
            operator++()
            {
                ++PixelIndex;
                return *this;
            }

            FImagePixelIterator
            operator++(Int32)
            {
                FImagePixelIterator Temp = *this;
                ++(*this);
                return Temp;
            }

            FImagePixelIterator&
            operator--()
            {
                --PixelIndex;
                return *this;
            }

            FImagePixelIterator
            operator--(Int32)
            {
                FImagePixelIterator Temp = *this;
                --(*this);
                return Temp;
            }

            FImagePixelIterator&
            operator+=(difference_type I_N)
            {
                PixelIndex += I_N;
                return *this;
            }

            FImagePixelIterator&
            operator-=(difference_type I_N)
            {
                PixelIndex -= I_N;
                return *this;
            }

            [[nodiscard]] FImagePixelIterator
            operator+(difference_type I_N) const
            {
                FImagePixelIterator Temp = *this;
                Temp += I_N;
                return Temp;
            }

            [[nodiscard]] FImagePixelIterator
            operator-(difference_type I_N) const
            {
                FImagePixelIterator Temp = *this;
                Temp -= I_N;
                return Temp;
            }

            [[nodiscard]] difference_type
            operator-(const FImagePixelIterator& I_Other) const
            {
                return static_cast<difference_type>(PixelIndex) - static_cast<difference_type>(I_Other.PixelIndex);
            }

            [[nodiscard]] Bool
            operator==(const FImagePixelIterator& I_Other) const
            {
                return Image == I_Other.Image && PixelIndex == I_Other.PixelIndex;
            }

            [[nodiscard]] Bool
            operator!=(const FImagePixelIterator& I_Other) const
            {
                return !(*this == I_Other);
            }

            [[nodiscard]] Bool
            operator<(const FImagePixelIterator& I_Other) const
            {
                return PixelIndex < I_Other.PixelIndex;
            }

            [[nodiscard]] Bool
            operator>(const FImagePixelIterator& I_Other) const
            {
                return PixelIndex > I_Other.PixelIndex;
            }

            [[nodiscard]] Bool
            operator<=(const FImagePixelIterator& I_Other) const
            {
                return PixelIndex <= I_Other.PixelIndex;
            }

            [[nodiscard]] Bool
            operator>=(const FImagePixelIterator& I_Other) const
            {
                return PixelIndex >= I_Other.PixelIndex;
            }

        private:
            FImage*        Image = nullptr;
            UInt64         PixelIndex = 0;
            mutable FPixel CurrentPixel{nullptr, EPixelFormat::Invalid, 0};
            mutable UInt64 CachedPixelIndex = UInt64(-1);

        private:
            [[nodiscard]] UInt64
            GetPixelCount() const
            {
                if (!Image) { return 0; }
                return static_cast<UInt64>(Image->GetWidth()) * static_cast<UInt64>(Image->GetHeight()) * static_cast<UInt64>(Image->GetDepth());
            }

            void
            UpdatePixel() const
            {
                if (CachedPixelIndex == PixelIndex && CurrentPixel.GetData() != nullptr)
                { return; }

                if (!Image || PixelIndex >= GetPixelCount())
                {
                    CurrentPixel = FPixel{nullptr, EPixelFormat::Invalid, 0};
                    CachedPixelIndex = PixelIndex;
                    return;
                }

                // Support for Depth > 1: calculate slice index and local pixel index
                VISERA_ASSERT(Image->GetDepth() == 1 && "FImagePixelIterator currently only supports Depth == 1");
                const UInt32 Width = Image->GetWidth();
                const UInt32 Height = Image->GetHeight();
                const UInt32 Depth = Image->GetDepth();
                
                const UInt32 PixelsPerSlice = Width * Height;
                const UInt32 SliceIndex = static_cast<UInt32>(PixelIndex / PixelsPerSlice);
                const UInt32 LocalPixelIndex = static_cast<UInt32>(PixelIndex % PixelsPerSlice);
                const UInt32 X = LocalPixelIndex % Width;
                const UInt32 Y = LocalPixelIndex / Width;

                const UInt32 BytesPerPixel = Image->GetBytesPerPixel();
                const UInt32 RowPitch = Image->GetRowPitchBytes();
                const UInt32 SlicePitch = Image->GetSlicePitchBytes();
                const UInt32 ClampedSlice = Math::Min(SliceIndex, Depth > 0 ? Depth - 1 : 0);
                FByte* PixelData = Image->AccessData() + (ClampedSlice * SlicePitch) + (Y * RowPitch + X * BytesPerPixel);
                CurrentPixel = FPixel{PixelData, Image->GetPixelFormat(), static_cast<UInt8>(BytesPerPixel)};
                CachedPixelIndex = PixelIndex;
            }
        };

        /**
         * Const iterator for a 2D view (rectangular region) of FImage.
         * Only iterates over pixels within the specified X and Y intervals on a given layer.
         * Provides read-only access to pixels.
         */
        class VISERA_CORE_API FConstImageViewIterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = const FPixel;
            using difference_type = Int64;
            using pointer = const FPixel*;
            using reference = const FPixel&;

            FConstImageViewIterator() = default;
            explicit FConstImageViewIterator(
                const FImage* I_Image,
                TClosedInterval<UInt32> I_IntervalX,
                TClosedInterval<UInt32> I_IntervalY,
                UInt32 I_Layer = 0,
                UInt64 I_Index = 0)
                : Image{I_Image}
                , IntervalX{I_IntervalX}
                , IntervalY{I_IntervalY}
                , Layer{I_Layer}
                , PixelIndex{I_Index}
            {
            }

            [[nodiscard]] const FPixel&
            operator*() const
            {
                UpdatePixel();
                return CurrentPixel;
            }

            FConstImageViewIterator&
            operator++()
            {
                ++PixelIndex;
                return *this;
            }

            FConstImageViewIterator
            operator++(Int32)
            {
                FConstImageViewIterator Temp = *this;
                ++(*this);
                return Temp;
            }

            FConstImageViewIterator&
            operator--()
            {
                --PixelIndex;
                return *this;
            }

            FConstImageViewIterator
            operator--(Int32)
            {
                FConstImageViewIterator Temp = *this;
                --(*this);
                return Temp;
            }

            FConstImageViewIterator&
            operator+=(difference_type I_N)
            {
                PixelIndex += I_N;
                return *this;
            }

            FConstImageViewIterator&
            operator-=(difference_type I_N)
            {
                PixelIndex -= I_N;
                return *this;
            }

            [[nodiscard]] FConstImageViewIterator
            operator+(difference_type I_N) const
            {
                FConstImageViewIterator Temp = *this;
                Temp += I_N;
                return Temp;
            }

            [[nodiscard]] FConstImageViewIterator
            operator-(difference_type I_N) const
            {
                FConstImageViewIterator Temp = *this;
                Temp -= I_N;
                return Temp;
            }

            [[nodiscard]] difference_type
            operator-(const FConstImageViewIterator& I_Other) const
            {
                return static_cast<difference_type>(PixelIndex) - static_cast<difference_type>(I_Other.PixelIndex);
            }

            [[nodiscard]] Bool
            operator==(const FConstImageViewIterator& I_Other) const
            {
                return Image == I_Other.Image &&
                       IntervalX == I_Other.IntervalX &&
                       IntervalY == I_Other.IntervalY &&
                       Layer == I_Other.Layer &&
                       PixelIndex == I_Other.PixelIndex;
            }

            [[nodiscard]] Bool
            operator!=(const FConstImageViewIterator& I_Other) const
            {
                return !(*this == I_Other);
            }

            [[nodiscard]] Bool
            operator<(const FConstImageViewIterator& I_Other) const
            {
                return PixelIndex < I_Other.PixelIndex;
            }

            [[nodiscard]] Bool
            operator>(const FConstImageViewIterator& I_Other) const
            {
                return PixelIndex > I_Other.PixelIndex;
            }

            [[nodiscard]] Bool
            operator<=(const FConstImageViewIterator& I_Other) const
            {
                return PixelIndex <= I_Other.PixelIndex;
            }

            [[nodiscard]] Bool
            operator>=(const FConstImageViewIterator& I_Other) const
            {
                return PixelIndex >= I_Other.PixelIndex;
            }

        private:
            const FImage* Image = nullptr;
            TClosedInterval<UInt32> IntervalX{0, 0};
            TClosedInterval<UInt32> IntervalY{0, 0};
            UInt32 Layer = 0;
            UInt64 PixelIndex = 0;
            mutable FPixel CurrentPixel{nullptr, EPixelFormat::Invalid, 0};
            mutable UInt64 CachedPixelIndex = UInt64(-1);

        private:
            [[nodiscard]] UInt64
            GetPixelCount() const
            {
                if (!Image)
                { return 0; }
                const UInt32 RegionWidth = IntervalX.Length() + 1;
                const UInt32 RegionHeight = IntervalY.Length() + 1;
                return static_cast<UInt64>(RegionWidth) * static_cast<UInt64>(RegionHeight);
            }

            void
            UpdatePixel() const
            {
                if (CachedPixelIndex == PixelIndex && CurrentPixel.GetData() != nullptr)
                { return; }

                if (!Image || PixelIndex >= GetPixelCount())
                {
                    CurrentPixel = FPixel{nullptr, EPixelFormat::Invalid, 0};
                    CachedPixelIndex = PixelIndex;
                    return;
                }

                const UInt32 RegionWidth = IntervalX.Length() + 1;
                const UInt32 LocalX = static_cast<UInt32>(PixelIndex % RegionWidth);
                const UInt32 LocalY = static_cast<UInt32>(PixelIndex / RegionWidth);
                const UInt32 X = IntervalX.Left + LocalX;
                const UInt32 Y = IntervalY.Left + LocalY;

                if (X > IntervalX.Right || Y > IntervalY.Right)
                {
                    CurrentPixel = FPixel{nullptr, EPixelFormat::Invalid, 0};
                    CachedPixelIndex = PixelIndex;
                    return;
                }

                const UInt32 BytesPerPixel = Image->GetBytesPerPixel();
                const UInt32 RowPitch = Image->GetRowPitchBytes();
                const UInt32 SlicePitch = Image->GetSlicePitchBytes();
                const UInt32 ClampedLayer = Math::Min(Layer, Image->GetDepth() > 0 ? Image->GetDepth() - 1 : 0);
                const FByte* ConstPixelData = Image->GetData() + (ClampedLayer * SlicePitch) + (Y * RowPitch + X * BytesPerPixel);
                CurrentPixel = FPixel{ConstPixelData, Image->GetPixelFormat(), static_cast<UInt8>(BytesPerPixel)};
                CachedPixelIndex = PixelIndex;
            }
        };

        /**
         * Create a safe iterable 3D view of the image.
         * The view automatically clamps coordinates to valid image bounds.
         * Z coordinate represents the layer (depth).
         *
         * Usage:
         * auto View3D = Image.View3D({MinX, MinY, MinLayer}, {MaxX, MaxY, MaxLayer});
         * auto LayerView2D = View3D.Subview2D(LayerIndex, {MinX, MinY}, {MaxX, MaxY});
         * for (auto& Pixel : LayerView) { ... }
         * LayerView.SetPixel({X, Y}, FColor::Yellow());
         *
         * @param I_Min Minimum coordinates (X, Y, Z where Z is layer)
         * @param I_Max Maximum coordinates (X, Y, Z where Z is layer)
         * @return FImageView3D representing the safe view
         */
        [[nodiscard]] FImageView3D
        View3D(const FPoint3U& I_Min, const FPoint3U& I_Max)
        {
            return FImageView3D{*this, I_Min, I_Max};
        }
        [[nodiscard]] FImageView3D
        View3D()
        {
            const UInt32 MaxX = GetWidth()  > 0 ? GetWidth()  - 1 : 0;
            const UInt32 MaxY = GetHeight() > 0 ? GetHeight() - 1 : 0;
            const UInt32 MaxZ = GetDepth()  > 0 ? GetDepth()  - 1 : 0;
            return FImageView3D{*this, FPoint3U{0, 0, 0}, FPoint3U{MaxX, MaxY, MaxZ}};
        }

        [[nodiscard]] FImageView3D
        View3D(const FPoint3U& I_Min, const FPoint3U& I_Max) const;
        [[nodiscard]] FImageView3D
        View3D() const;
        /** @return Full 2D view of the given layer (for SaveImage etc.). */
        [[nodiscard]] FImageView2D
        View2D(UInt32 I_Layer = 0) const;

        /** Returns FPixel that points to (X,Y,Z); use .SetColor() to write. */
        [[nodiscard]] FPixel
        operator()(UInt32 I_X, UInt32 I_Y, UInt32 I_Z);

        /** Returns FPixel that points to (X,Y,Z); use .GetColor() to read. */
        [[nodiscard]] FPixel
        operator()(UInt32 I_X, UInt32 I_Y, UInt32 I_Z) const;

    private:
        TPMRArray<FByte> Data;
        FCreateInfo      Info;

    public:
        FImage()  = default;
        ~FImage() = default;
        FImage(const FCreateInfo& I_CreateInfo);
        
        /**
         * Copy constructor.
         */
        FImage(const FImage& I_Other) = default;
        
        /**
         * Move constructor.
         */
        FImage(FImage&& I_Other) noexcept = default;
        
        /**
         * Copy assignment operator.
         */
        FImage& operator=(const FImage& I_Other) = default;
        
        /**
         * Move assignment operator.
         */
        FImage& operator=(FImage&& I_Other) noexcept = default;

    private:
        [[nodiscard]] static inline stbir_pixel_layout
        MapPixelFormatToSTBIR(EPixelFormat I_Format) noexcept;
    };

    FImage::
    FImage(const FCreateInfo& I_CreateInfo)
    : Data  (I_CreateInfo.MemoryArena),
      Info  (I_CreateInfo)
    {
        VISERA_ASSERT(I_CreateInfo.PixelFormat != EPixelFormat::Invalid);
        VISERA_ASSERT(I_CreateInfo.Width > 0 && I_CreateInfo.Height > 0 && I_CreateInfo.Depth > 0);
        VISERA_ASSERT(FPixel::GetByteSize(I_CreateInfo.PixelFormat) > 0);

        // Normalize pitches
        if (Info.RowPitchBytes == 0)
        {
            Info.RowPitchBytes = Info.Width * FPixel::GetByteSize(Info.PixelFormat);
        }
        if (Info.SlicePitchBytes == 0)
        {
            Info.SlicePitchBytes = Info.RowPitchBytes * Info.Height;
        }

        const auto SizeInBytes = GetSizeInBytes();
        if (SizeInBytes > 0)
        {
            Data.Resize(SizeInBytes);
        }
    }

    /**
     * Create a safe iterable 3D view of the image (const version).
     * The view automatically clamps coordinates to valid image bounds.
     * Z coordinate represents the layer (depth).
     *
     * Usage:
     * auto View3D = ConstImage.View3D({MinX, MinY, MinLayer}, {MaxX, MaxY, MaxLayer});
     * auto LayerView2D = View3D.View2D(LayerIndex, {MinX, MinY}, {MaxX, MaxY});
     * for (const auto& Pixel : LayerView) { ... }
     *
     * @param I_Min Minimum coordinates (X, Y, Z where Z is layer)
     * @param I_Max Maximum coordinates (X, Y, Z where Z is layer)
     * @return FImageView3D representing the safe view
     */
    FImageView3D FImage::
    View3D(const FPoint3U& I_Min, const FPoint3U& I_Max) const
    {
        return FImageView3D{*this, I_Min, I_Max};
    }

    FImageView3D FImage::
    View3D() const
    {
        const UInt32 MaxX = GetWidth()  > 0 ? GetWidth()  - 1 : 0;
        const UInt32 MaxY = GetHeight() > 0 ? GetHeight() - 1 : 0;
        const UInt32 MaxZ = GetDepth()  > 0 ? GetDepth()  - 1 : 0;
        return FImageView3D{*this, FPoint3U{0, 0, 0}, FPoint3U{MaxX, MaxY, MaxZ}};
    }

    FImageView2D FImage::
    View2D(UInt32 I_Layer) const
    {
        const UInt32 MaxX = GetWidth()  > 0 ? GetWidth()  - 1 : 0;
        const UInt32 MaxY = GetHeight() > 0 ? GetHeight() - 1 : 0;
        return FImageView2D{*this, I_Layer, FPoint2U{0, 0}, FPoint2U{MaxX, MaxY}};
    }

    // Non-member operator for iterator arithmetic (allows I_N + iterator syntax)
    [[nodiscard]] inline FImage::FImagePixelIterator
    operator+(FImage::FImagePixelIterator::difference_type I_N, const FImage::FImagePixelIterator& I_It)
    {
        return I_It + I_N;
    }

    /**
     * Maps EPixelFormat to stbir_pixel_layout for use with stb_image_resize2.
     * @param I_Format The pixel format to map
     * @return The corresponding stbir_pixel_layout value
     */
    stbir_pixel_layout FImage::
    MapPixelFormatToSTBIR(EPixelFormat I_Format) noexcept
    {
        switch (I_Format)
        {
        case EPixelFormat::R8_UNorm:
        case EPixelFormat::R16_UNorm:
        case EPixelFormat::R16_Float:
        case EPixelFormat::R32_Float:
            return STBIR_1CHANNEL;

        case EPixelFormat::RG8_UNorm:
        case EPixelFormat::RG16_UNorm:
        case EPixelFormat::RG16_Float:
        case EPixelFormat::RG32_Float:
            return STBIR_2CHANNEL;

        case EPixelFormat::RGB8_UNorm:
        case EPixelFormat::RGB16_UNorm:
        case EPixelFormat::RGB16_Float:
        case EPixelFormat::RGB32_Float:
        case EPixelFormat::RGBE8_HDR:
            return STBIR_RGB;

        case EPixelFormat::RGBA8_UNorm:
        case EPixelFormat::RGBA16_UNorm:
        case EPixelFormat::RGBA16_Float:
        case EPixelFormat::RGBA32_Float:
            return STBIR_RGBA;

        case EPixelFormat::BGRA8_UNorm:
            return STBIR_BGRA;

        case EPixelFormat::Invalid:
        default:
            VISERA_ASSERT(False);
            return STBIR_1CHANNEL;
        }
    }

    Bool FImage::
    Resize(UInt32 I_NewWidth, UInt32 I_NewHeight)
    {
        // Early return if dimensions unchanged
        if (I_NewWidth == Info.Width && I_NewHeight == Info.Height) { return True; }
        if (I_NewWidth == 0 || I_NewHeight == 0) { return False; }
        const Bool bHasAlpha = HasAlpha();
        if (bHasAlpha) { VISERA_ASSERT(GetChannelCount() == 4); }

        // Resize is per-slice 2D operation
        VISERA_ASSERT(Info.Depth >= 1);

        const auto ChannelCount = GetChannelCount();
        // IMPORTANT: Float buffers are normalized to R,G,B,(A) order (even if source is BGRA).
        // So stbir float resize must use the layout implied by the *float buffer*, not by PixelFormat.
        const stbir_pixel_layout FloatLayout =
            (ChannelCount == 4) ? STBIR_RGBA :
            (ChannelCount == 3) ? STBIR_RGB  :
            (ChannelCount == 2) ? STBIR_2CHANNEL :
                                  STBIR_1CHANNEL;
        const auto InputPixelCount = Info.Width * Info.Height;
        const auto OutputPixelCount = I_NewWidth * I_NewHeight;
        const auto InputSlicePitch = GetSlicePitchBytes();
        const auto BytesPerPixel = GetBytesPerPixel();
        const auto OutputRowPitch = I_NewWidth * BytesPerPixel;
        const auto OutputSlicePitch = OutputRowPitch * I_NewHeight;
        const auto OutputSizeInBytes = static_cast<UInt64>(OutputSlicePitch) * Info.Depth;

        // Allocate output buffer
        TPMRArray<FByte> OutputBuffer(Info.MemoryArena);
        OutputBuffer.Resize(static_cast<TPMRArray<FByte>::SizeType>(OutputSizeInBytes));

        // Reusable float buffers (allocated once per resize, reused for each slice)
        TPMRArray<Float> InputFloatBuffer(Info.MemoryArena);
        TPMRArray<Float> OutputFloatBuffer(Info.MemoryArena);
        const auto FloatBufferSize = Math::Max(InputPixelCount, OutputPixelCount) * ChannelCount;
        InputFloatBuffer.Resize(FloatBufferSize);
        OutputFloatBuffer.Resize(FloatBufferSize);

        const Bool bIsHalfFormat = IsHalfFloatFormat();
        const Bool bIsUNorm16Format = IsUNorm16Format();
        const Bool bNativeFloatFormat  = bIsHalfFormat || bIsUNorm16Format || IsFloatFormat();

        Bool bSuccess = True;

        // Process each depth slice (2D resize per slice)
        for (UInt32 DepthSlice = 0; DepthSlice < Info.Depth; ++DepthSlice)
        {
            const auto* InputSliceData = Data.Data() + (DepthSlice * InputSlicePitch);
            auto* OutputSliceData = OutputBuffer.Data() + (DepthSlice * OutputSlicePitch);

            Bool bSliceSuccess = False;

            if (bNativeFloatFormat)
            {
                // Convert input to float32
                const auto InputRowPitchBytes = GetRowPitchBytes();
                const auto ExpectedRowPitchBytes = Info.Width * BytesPerPixel;
                const Bool bIsTightPacked = (InputRowPitchBytes == ExpectedRowPitchBytes);

                if (bIsHalfFormat)
                {
                    // Use FHalf for half-to-float conversion
                    if (bIsTightPacked)
                    {
                        const auto* InputHalfBits = reinterpret_cast<const UInt16*>(InputSliceData);
                        for (UInt32 i = 0; i < InputPixelCount * ChannelCount; ++i)
                        {
                            const FHalf HalfValue = FHalf::FromBits(InputHalfBits[i]);
                            InputFloatBuffer[i] = static_cast<Float>(HalfValue);
                        }
                    }
                    else
                    {
                        // Strided - copy row by row
                        for (UInt32 y = 0; y < Info.Height; ++y)
                        {
                            const auto* SrcRow = reinterpret_cast<const UInt16*>(InputSliceData + y * InputRowPitchBytes);
                            Float* DstRow = InputFloatBuffer.Data() + y * Info.Width * ChannelCount;
                            for (UInt32 x = 0; x < Info.Width * ChannelCount; ++x)
                            {
                                const FHalf HalfValue = FHalf::FromBits(SrcRow[x]);
                                DstRow[x] = static_cast<Float>(HalfValue);
                            }
                        }
                    }
                }
                else if (bIsUNorm16Format)
                {
                    // Convert 16-bit UNorm to float (normalize to 0.0-1.0)
                    if (bIsTightPacked)
                    {
                        const auto* InputU16 = reinterpret_cast<const UInt16*>(InputSliceData);
                        for (UInt32 i = 0; i < InputPixelCount * ChannelCount; ++i)
                        {
                            InputFloatBuffer[i] = static_cast<Float>(InputU16[i]) / 65535.0f;
                        }
                    }
                    else
                    {
                        // Strided - copy row by row
                        for (UInt32 y = 0; y < Info.Height; ++y)
                        {
                            const auto* SrcRow = reinterpret_cast<const UInt16*>(InputSliceData + y * InputRowPitchBytes);
                            Float* DstRow = InputFloatBuffer.Data() + y * Info.Width * ChannelCount;
                            for (UInt32 x = 0; x < Info.Width * ChannelCount; ++x)
                            {
                                DstRow[x] = static_cast<Float>(SrcRow[x]) / 65535.0f;
                            }
                        }
                    }
                }
                else
                {
                    // 32-bit float formats - copy directly
                    const auto* InputFloat = reinterpret_cast<const Float*>(InputSliceData);
                    const auto InputRowPitch = GetRowPitchBytes();
                    if (InputRowPitch == Info.Width * sizeof(Float) * ChannelCount)
                    {
                        // Tightly packed - can memcpy
                        Memory::Memcpy(InputFloatBuffer.Data(), InputFloat, InputPixelCount * ChannelCount * sizeof(Float));
                    }
                    else
                    {
                        // Strided - copy row by row
                        for (UInt32 y = 0; y < Info.Height; ++y)
                        {
                            const auto* SrcRow = reinterpret_cast<const Float*>(InputSliceData + y * InputRowPitch);
                            Float* DstRow = InputFloatBuffer.Data() + y * Info.Width * ChannelCount;
                            Memory::Memcpy(DstRow, SrcRow, Info.Width * ChannelCount * sizeof(Float));
                        }
                    }
                }

                // Pre-multiply alpha if needed
                if (bHasAlpha)
                {
                    for (UInt32 i = 0; i < InputPixelCount; ++i)
                    {
                        const UInt32 AlphaIdx = i * ChannelCount + 3;
                        const Float Alpha = InputFloatBuffer[AlphaIdx];
                        InputFloatBuffer[i * ChannelCount + 0] *= Alpha; // R
                        InputFloatBuffer[i * ChannelCount + 1] *= Alpha; // G
                        InputFloatBuffer[i * ChannelCount + 2] *= Alpha; // B
                    }
                }

                // Resize using float function (always linear, sRGB handled separately for uint8)
                bSliceSuccess = (stbir_resize_float_linear(
                    InputFloatBuffer.Data(), Info.Width, Info.Height, 0,
                    OutputFloatBuffer.Data(), I_NewWidth, I_NewHeight, 0,
                    FloatLayout) != nullptr);

                if (bSliceSuccess)
                {
                    // Un-premultiply alpha if needed (with threshold to avoid noise amplification)
                    if (bHasAlpha)
                    {
                        constexpr Float AlphaThreshold = 1e-6f; // Threshold to avoid division by very small alpha
                        for (UInt32 i = 0; i < OutputPixelCount; ++i)
                        {
                            const UInt32 BaseIdx = i * ChannelCount;
                            const Float Alpha = OutputFloatBuffer[BaseIdx + 3];
                            if (Alpha > AlphaThreshold)
                            {
                                const Float InvAlpha = 1.0f / Alpha;
                                OutputFloatBuffer[BaseIdx + 0] *= InvAlpha; // R
                                OutputFloatBuffer[BaseIdx + 1] *= InvAlpha; // G
                                OutputFloatBuffer[BaseIdx + 2] *= InvAlpha; // B
                                // Clamp RGB to [0,1] to avoid division amplification errors
                                OutputFloatBuffer[BaseIdx + 0] = Math::Clamp(OutputFloatBuffer[BaseIdx + 0], 0.0f, 1.0f);
                                OutputFloatBuffer[BaseIdx + 1] = Math::Clamp(OutputFloatBuffer[BaseIdx + 1], 0.0f, 1.0f);
                                OutputFloatBuffer[BaseIdx + 2] = Math::Clamp(OutputFloatBuffer[BaseIdx + 2], 0.0f, 1.0f);
                            }
                            else
                            {
                                // Alpha too small - set RGB to 0 to avoid noise
                                OutputFloatBuffer[BaseIdx + 0] = 0.0f; // R
                                OutputFloatBuffer[BaseIdx + 1] = 0.0f; // G
                                OutputFloatBuffer[BaseIdx + 2] = 0.0f; // B
                            }
                        }
                    }

                    // Convert output back from float32
                    // Note: Output buffer is always tightly packed (we allocate it ourselves)
                    // So we can directly use tight-packed path without checking stride

                    if (bIsHalfFormat)
                    {
                        // Use FHalf for float-to-half conversion
                        // Output buffer is always tightly packed
                        auto* OutputHalfBits = reinterpret_cast<UInt16*>(OutputSliceData);
                        for (UInt32 i = 0; i < OutputPixelCount * ChannelCount; ++i)
                        {
                            const FHalf HalfValue(OutputFloatBuffer[i]);
                            OutputHalfBits[i] = HalfValue.ToBits();
                        }
                    }
                    else if (bIsUNorm16Format)
                    {
                        // Convert float to 16-bit UNorm
                        // Output buffer is always tightly packed
                        auto* OutputU16 = reinterpret_cast<UInt16*>(OutputSliceData);
                        for (UInt32 i = 0; i < OutputPixelCount * ChannelCount; ++i)
                        {
                            const Float ClampedValue = Math::Clamp(OutputFloatBuffer[i], 0.0f, 1.0f);
                            OutputU16[i] = static_cast<UInt16>(ClampedValue * 65535.0f + 0.5f); // Round to nearest
                        }
                    }
                    else
                    {
                        // 32-bit float formats - copy directly
                        // Output buffer is always tightly packed
                        Memory::Memcpy(OutputSliceData, OutputFloatBuffer.Data(), OutputPixelCount * ChannelCount * sizeof(Float));
                    }
                }
            }
            else
            {
                // Uint8 formats
                const auto InputRowPitch = GetRowPitchBytes();
                const auto PixelLayout = MapPixelFormatToSTBIR(Info.PixelFormat);

                if (Info.ColorSpace == EColorSpace::sRGB && bHasAlpha)
                {
                    // sRGB with alpha: RGB in sRGB space, Alpha in linear space
                    // Convert to float using Color module, process RGB in sRGB, then convert back
                    const auto InputRowPitchBytes = GetRowPitchBytes();
                    const auto ExpectedRowPitchBytes = Info.Width * BytesPerPixel;
                    const Bool bIsTightPacked = (InputRowPitchBytes == ExpectedRowPitchBytes);
                    const Bool bIsBGRA = (Info.PixelFormat == EPixelFormat::BGRA8_UNorm);

                    // Convert uint8 to float: RGB using sRGB LUT, Alpha using linear
                    if (bIsTightPacked)
                    {
                        const auto* InputU8 = reinterpret_cast<const UInt8*>(InputSliceData);
                        for (UInt32 i = 0; i < InputPixelCount; ++i)
                        {
                            const UInt32 BaseIdx = i * ChannelCount;
                            if (bIsBGRA)
                            {
                                // BGRA format: B, G, R, A
                                InputFloatBuffer[BaseIdx + 0] = FLinearColor::SRGBToLinear(InputU8[BaseIdx + 0]); // dst.R = src.R
                                InputFloatBuffer[BaseIdx + 1] = FLinearColor::SRGBToLinear(InputU8[BaseIdx + 1]); // dst.G = src.G
                                InputFloatBuffer[BaseIdx + 2] = FLinearColor::SRGBToLinear(InputU8[BaseIdx + 2]); // dst.B = src.B
                                InputFloatBuffer[BaseIdx + 3] = static_cast<Float>(InputU8[BaseIdx + 3]) / 255.0f; // A
                            }
                            else
                            {
                                // RGBA format: R, G, B, A
                                InputFloatBuffer[BaseIdx + 0] = FLinearColor::SRGBToLinear(InputU8[BaseIdx + 0]); // R
                                InputFloatBuffer[BaseIdx + 1] = FLinearColor::SRGBToLinear(InputU8[BaseIdx + 1]); // G
                                InputFloatBuffer[BaseIdx + 2] = FLinearColor::SRGBToLinear(InputU8[BaseIdx + 2]); // B
                                InputFloatBuffer[BaseIdx + 3] = static_cast<Float>(InputU8[BaseIdx + 3]) / 255.0f; // A
                            }
                        }
                    }
                    else
                    {
                        // Strided - copy row by row
                        for (UInt32 y = 0; y < Info.Height; ++y)
                        {
                            const auto* SrcRow = reinterpret_cast<const UInt8*>(InputSliceData + y * InputRowPitchBytes);
                            Float* DstRow = InputFloatBuffer.Data() + y * Info.Width * ChannelCount;
                            for (UInt32 x = 0; x < Info.Width; ++x)
                            {
                                const UInt32 SrcBaseIdx = x * ChannelCount;
                                const UInt32 DstBaseIdx = x * ChannelCount;
                                if (bIsBGRA)
                                {
                                    // BGRA format: B, G, R, A
                                    DstRow[DstBaseIdx + 0] = FLinearColor::SRGBToLinear(SrcRow[SrcBaseIdx + 2]); // dst.R = src.R
                                    DstRow[DstBaseIdx + 1] = FLinearColor::SRGBToLinear(SrcRow[SrcBaseIdx + 1]); // dst.G = src.G
                                    DstRow[DstBaseIdx + 2] = FLinearColor::SRGBToLinear(SrcRow[SrcBaseIdx + 0]); // dst.B = src.B
                                    DstRow[DstBaseIdx + 3] = static_cast<Float>(SrcRow[SrcBaseIdx + 3]) / 255.0f; // A
                                }
                                else
                                {
                                    // RGBA format: R, G, B, A
                                    DstRow[DstBaseIdx + 0] = FLinearColor::SRGBToLinear(SrcRow[SrcBaseIdx + 0]); // R
                                    DstRow[DstBaseIdx + 1] = FLinearColor::SRGBToLinear(SrcRow[SrcBaseIdx + 1]); // G
                                    DstRow[DstBaseIdx + 2] = FLinearColor::SRGBToLinear(SrcRow[SrcBaseIdx + 2]); // B
                                    DstRow[DstBaseIdx + 3] = static_cast<Float>(SrcRow[SrcBaseIdx + 3]) / 255.0f; // A
                                }
                            }
                        }
                    }

                    // Pre-multiply alpha
                    for (UInt32 i = 0; i < InputPixelCount; ++i)
                    {
                        const UInt32 BaseIdx = i * ChannelCount;
                        const Float Alpha = InputFloatBuffer[BaseIdx + 3];
                        InputFloatBuffer[BaseIdx + 0] *= Alpha; // R
                        InputFloatBuffer[BaseIdx + 1] *= Alpha; // G
                        InputFloatBuffer[BaseIdx + 2] *= Alpha; // B
                    }

                    // Resize in linear space
                    bSliceSuccess = (stbir_resize_float_linear(
                        InputFloatBuffer.Data(), Info.Width, Info.Height, 0,
                        OutputFloatBuffer.Data(), I_NewWidth, I_NewHeight, 0,
                        FloatLayout) != nullptr);

                    if (bSliceSuccess)
                    {
                        // Un-premultiply alpha (with threshold to avoid noise amplification)
                        constexpr Float AlphaThreshold = 1.0f / 255.0f;
                        for (UInt32 i = 0; i < OutputPixelCount; ++i)
                        {
                            const UInt32 BaseIdx = i * ChannelCount;
                            const Float Alpha = OutputFloatBuffer[BaseIdx + 3];
                            if (Alpha > AlphaThreshold)
                            {
                                const Float InvAlpha = 1.0f / Alpha;
                                OutputFloatBuffer[BaseIdx + 0] *= InvAlpha; // R
                                OutputFloatBuffer[BaseIdx + 1] *= InvAlpha; // G
                                OutputFloatBuffer[BaseIdx + 2] *= InvAlpha; // B
                                // Clamp RGB to [0,1] to avoid division amplification errors
                                OutputFloatBuffer[BaseIdx + 0] = Math::Clamp(OutputFloatBuffer[BaseIdx + 0], 0.0f, 1.0f);
                                OutputFloatBuffer[BaseIdx + 1] = Math::Clamp(OutputFloatBuffer[BaseIdx + 1], 0.0f, 1.0f);
                                OutputFloatBuffer[BaseIdx + 2] = Math::Clamp(OutputFloatBuffer[BaseIdx + 2], 0.0f, 1.0f);
                            }
                            else
                            {
                                // Alpha too small - set RGB to 0 to avoid noise
                                OutputFloatBuffer[BaseIdx + 0] = 0.0f; // R
                                OutputFloatBuffer[BaseIdx + 1] = 0.0f; // G
                                OutputFloatBuffer[BaseIdx + 2] = 0.0f; // B
                            }
                        }

                        // Convert back to uint8: RGB using FColor::SRGB8ColorFromLinear, Alpha using /255
                        // Output buffer is always tightly packed
                        auto* OutputU8 = reinterpret_cast<UInt8*>(OutputSliceData);
                        for (UInt32 i = 0; i < OutputPixelCount; ++i)
                        {
                            const UInt32 BaseIdx = i * ChannelCount;
                            const FLinearColor LinearColor{
                                OutputFloatBuffer[BaseIdx + 0], // R
                                OutputFloatBuffer[BaseIdx + 1], // G
                                OutputFloatBuffer[BaseIdx + 2], // B
                                OutputFloatBuffer[BaseIdx + 3]  // A
                            };
                            const FColor SRGB8Color = FColor::SRGB8ColorFromLinear(LinearColor);
                            if (bIsBGRA)
                            {
                                // BGRA format: B, G, R, A
                                OutputU8[BaseIdx + 0] = SRGB8Color.B; // B
                                OutputU8[BaseIdx + 1] = SRGB8Color.G; // G
                                OutputU8[BaseIdx + 2] = SRGB8Color.R; // R
                                OutputU8[BaseIdx + 3] = SRGB8Color.A; // A
                            }
                            else
                            {
                                // RGBA format: R, G, B, A
                                OutputU8[BaseIdx + 0] = SRGB8Color.R; // R
                                OutputU8[BaseIdx + 1] = SRGB8Color.G; // G
                                OutputU8[BaseIdx + 2] = SRGB8Color.B; // B
                                OutputU8[BaseIdx + 3] = SRGB8Color.A; // A
                            }
                        }
                    }
                }
                else
                {
                    // Standard uint8 resize
                    // Note: sRGB formats without alpha also go through float path to avoid stbir_resize_uint8_srgb
                    if (Info.ColorSpace == EColorSpace::sRGB)
                    {
                        // sRGB without alpha: convert to float, resize, convert back
                        const auto InputRowPitchBytes = GetRowPitchBytes();
                        const auto ExpectedRowPitchBytes = Info.Width * BytesPerPixel;
                        const Bool bIsTightPacked = (InputRowPitchBytes == ExpectedRowPitchBytes);
                        const Bool bIsBGRA = (Info.PixelFormat == EPixelFormat::BGRA8_UNorm);

                        // Convert uint8 to float: RGB using sRGB LUT
                        if (bIsTightPacked)
                        {
                            const auto* InputU8 = reinterpret_cast<const UInt8*>(InputSliceData);
                            for (UInt32 i = 0; i < InputPixelCount * ChannelCount; ++i)
                            {
                                InputFloatBuffer[i] = FLinearColor::SRGBToLinear(InputU8[i]);
                            }
                        }
                        else
                        {
                            // Strided - copy row by row
                            for (UInt32 y = 0; y < Info.Height; ++y)
                            {
                                const auto* SrcRow = reinterpret_cast<const UInt8*>(InputSliceData + y * InputRowPitchBytes);
                                Float* DstRow = InputFloatBuffer.Data() + y * Info.Width * ChannelCount;
                                for (UInt32 x = 0; x < Info.Width * ChannelCount; ++x)
                                {
                                    DstRow[x] = FLinearColor::SRGBToLinear(SrcRow[x]);
                                }
                            }
                        }

                        // Resize in linear space
                        bSliceSuccess = (stbir_resize_float_linear(
                            InputFloatBuffer.Data(), Info.Width, Info.Height, 0,
                            OutputFloatBuffer.Data(), I_NewWidth, I_NewHeight, 0,
                            FloatLayout) != nullptr);

                        if (bSliceSuccess)
                        {
                            // Convert back to uint8 using FColor::SRGB8ColorFromLinear
                            // Output buffer is always tightly packed
                            auto* OutputU8 = reinterpret_cast<UInt8*>(OutputSliceData);
                            for (UInt32 i = 0; i < OutputPixelCount; ++i)
                            {
                                const UInt32 BaseIdx = i * ChannelCount;
                                FLinearColor LinearColor{};
                                // Fill channels based on format (R, RG, RGB, or RGBA)
                                if (ChannelCount >= 1) LinearColor.R = OutputFloatBuffer[BaseIdx + 0];
                                if (ChannelCount >= 2) LinearColor.G = OutputFloatBuffer[BaseIdx + 1];
                                if (ChannelCount >= 3) LinearColor.B = OutputFloatBuffer[BaseIdx + 2];
                                if (ChannelCount >= 4) LinearColor.A = OutputFloatBuffer[BaseIdx + 3];
                                
                                const FColor SRGB8Color = FColor::SRGB8ColorFromLinear(LinearColor);
                                
                                // Write back channels in correct order
                                if (bIsBGRA && ChannelCount >= 4)
                                {
                                    // BGRA format: B, G, R, A
                                    OutputU8[BaseIdx + 0] = SRGB8Color.B;
                                    OutputU8[BaseIdx + 1] = SRGB8Color.G;
                                    OutputU8[BaseIdx + 2] = SRGB8Color.R;
                                    OutputU8[BaseIdx + 3] = SRGB8Color.A;
                                }
                                else
                                {
                                    // RGBA/RGB/RG/R format: R, G, B, A
                                    if (ChannelCount >= 1) OutputU8[BaseIdx + 0] = SRGB8Color.R;
                                    if (ChannelCount >= 2) OutputU8[BaseIdx + 1] = SRGB8Color.G;
                                    if (ChannelCount >= 3) OutputU8[BaseIdx + 2] = SRGB8Color.B;
                                    if (ChannelCount >= 4) OutputU8[BaseIdx + 3] = SRGB8Color.A;
                                }
                            }
                        }
                    }
                    else
                    {
                        // Linear uint8: use stbir_resize_uint8_linear
                        bSliceSuccess = (stbir_resize_uint8_linear(
                            InputSliceData, Info.Width, Info.Height, InputRowPitch,
                            OutputSliceData, I_NewWidth, I_NewHeight, OutputRowPitch,
                            PixelLayout) != nullptr);
                    }
                }
            }

            if (!bSliceSuccess)
            { bSuccess = False; break; }
        }

        if (bSuccess)
        {
            // Update image info and swap buffers
            Info.Width           = I_NewWidth;
            Info.Height          = I_NewHeight;
            Info.RowPitchBytes   = OutputRowPitch;
            Info.SlicePitchBytes = OutputSlicePitch;
            Data = std::move(OutputBuffer);
        }

        return bSuccess;
    }

    /**
     * Constructs a new FImage from a 3D view.
     * Copies pixel data from the specified view region into a new tightly-packed image.
     * 
     * @param I_View The 3D view to copy from
     * @param I_MemoryArena Optional memory arena (defaults to default resource)
     */
    FImage::
    FImage(const FImageView3D& I_View, std::pmr::memory_resource* I_MemoryArena)
        : Data{I_MemoryArena}
    {
        const auto IntervalX = I_View.GetIntervalX();
        const auto IntervalY = I_View.GetIntervalY();
        const UInt32 ViewWidth = IntervalX.Length() + 1;
        const UInt32 ViewHeight = IntervalY.Length() + 1;
        const UInt32 ViewDepth = I_View.GetMaxLayer() - I_View.GetMinLayer() + 1;

        if (ViewWidth == 0 || ViewHeight == 0 || ViewDepth == 0)
        {
            // Empty view
            Info.Width = ViewWidth;
            Info.Height = ViewHeight;
            Info.Depth = ViewDepth;
            Info.PixelFormat = EPixelFormat::RGBA8_UNorm;
            Info.ColorSpace = EColorSpace::Linear;
            Info.MemoryArena = I_MemoryArena;
            return;
        }

        // Get first layer view to access image properties
        const auto FirstLayerView = I_View.Subview2D(I_View.GetMinLayer(), FPoint2U{0, 0}, FPoint2U{ViewWidth - 1, ViewHeight - 1});
        if (FirstLayerView.begin() == FirstLayerView.end())
        {
            // Empty view
            Info.Width = ViewWidth;
            Info.Height = ViewHeight;
            Info.Depth = ViewDepth;
            Info.PixelFormat = EPixelFormat::RGBA8_UNorm;
            Info.ColorSpace = EColorSpace::Linear;
            Info.MemoryArena = I_MemoryArena;
            return;
        }

        // Get pixel format from first pixel
        const auto& FirstPixel = *FirstLayerView.begin();
        Info.Width = ViewWidth;
        Info.Height = ViewHeight;
        Info.Depth = ViewDepth;
        Info.PixelFormat = FirstPixel.GetPixelFormat();
        Info.ColorSpace = EColorSpace::Linear;
        Info.MemoryArena = I_MemoryArena;
        Info.RowPitchBytes = 0; // Will be auto-calculated
        Info.SlicePitchBytes = 0; // Will be auto-calculated

        // Normalize pitches
        if (Info.RowPitchBytes == 0)
        {
            Info.RowPitchBytes = Info.Width * FPixel::GetByteSize(Info.PixelFormat);
        }
        if (Info.SlicePitchBytes == 0)
        {
            Info.SlicePitchBytes = Info.RowPitchBytes * Info.Height;
        }

        const auto SizeInBytes = GetSizeInBytes();
        if (SizeInBytes > 0)
        {
            Data.Resize(SizeInBytes);
        }

        // Copy pixel data layer by layer
        const UInt32 BytesPerPixel = GetBytesPerPixel();
        const UInt32 DstRowPitch = GetRowPitchBytes();
        const UInt32 DstSlicePitch = GetSlicePitchBytes();

        for (UInt32 LayerIdx = 0; LayerIdx < ViewDepth; ++LayerIdx)
        {
            const UInt32 SrcLayer = I_View.GetMinLayer() + LayerIdx;
            const auto LayerView = I_View.Subview2D(SrcLayer, FPoint2U{0, 0}, FPoint2U{ViewWidth - 1, ViewHeight - 1});
            
            FByte* DstSliceData = AccessData() + (LayerIdx * DstSlicePitch);
            
            UInt32 PixelIndex = 0;
            for (const auto& Pixel : LayerView)
            {
                const UInt32 X = PixelIndex % ViewWidth;
                const UInt32 Y = PixelIndex / ViewWidth;
                const UInt32 DstOffset = (Y * DstRowPitch) + (X * BytesPerPixel);
                Memory::Memcpy(DstSliceData + DstOffset, Pixel.GetData(), BytesPerPixel);
                ++PixelIndex;
            }
        }
    }

    /**
     * Constructs a new FImage from a 2D view.
     * Copies pixel data from the specified 2D view into a new tightly-packed image.
     * 
     * @param I_View The 2D view to copy from
     * @param I_MemoryArena Optional memory arena (defaults to default resource)
     */
    FImage::
    FImage(const FImageView2D& I_View, std::pmr::memory_resource* I_MemoryArena)
        : Data{I_MemoryArena}
    {
        const auto IntervalX = I_View.GetIntervalX();
        const auto IntervalY = I_View.GetIntervalY();
        const UInt32 ViewWidth = IntervalX.Length() + 1;
        const UInt32 ViewHeight = IntervalY.Length() + 1;

        if (ViewWidth == 0 || ViewHeight == 0)
        {
            // Empty view
            Info.Width = ViewWidth;
            Info.Height = ViewHeight;
            Info.Depth = 1;
            Info.PixelFormat = EPixelFormat::RGBA8_UNorm;
            Info.ColorSpace = EColorSpace::Linear;
            Info.MemoryArena = I_MemoryArena;
            return;
        }

        // Get pixel format from first pixel
        if (I_View.begin() == I_View.end())
        {
            // Empty view
            Info.Width = ViewWidth;
            Info.Height = ViewHeight;
            Info.Depth = 1;
            Info.PixelFormat = EPixelFormat::RGBA8_UNorm;
            Info.ColorSpace = EColorSpace::Linear;
            Info.MemoryArena = I_MemoryArena;
            return;
        }

        const auto& FirstPixel = *I_View.begin();
        const FImage* SrcImage = I_View.GetImage();
        Info.Width = ViewWidth;
        Info.Height = ViewHeight;
        Info.Depth = 1;
        Info.PixelFormat = FirstPixel.GetPixelFormat();
        Info.ColorSpace = SrcImage ? SrcImage->GetColorSpace() : EColorSpace::Linear;
        Info.MemoryArena = I_MemoryArena;
        Info.RowPitchBytes = 0; // Will be auto-calculated
        Info.SlicePitchBytes = 0; // Will be auto-calculated

        // Normalize pitches
        if (Info.RowPitchBytes == 0)
        {
            Info.RowPitchBytes = Info.Width * FPixel::GetByteSize(Info.PixelFormat);
        }
        if (Info.SlicePitchBytes == 0)
        {
            Info.SlicePitchBytes = Info.RowPitchBytes * Info.Height;
        }

        const auto SizeInBytes = GetSizeInBytes();
        if (SizeInBytes > 0)
        {
            Data.Resize(SizeInBytes);
        }

        // Copy pixel data
        const UInt32 BytesPerPixel = GetBytesPerPixel();
        const UInt32 DstRowPitch = GetRowPitchBytes();
        FByte* DstData = AccessData();

        UInt32 PixelIndex = 0;
        for (const auto& Pixel : I_View)
        {
            const UInt32 X = PixelIndex % ViewWidth;
            const UInt32 Y = PixelIndex / ViewWidth;
            const UInt32 DstOffset = (Y * DstRowPitch) + (X * BytesPerPixel);
            Memory::Memcpy(DstData + DstOffset, Pixel.GetData(), BytesPerPixel);
            ++PixelIndex;
        }
    }

    /**
     * Gets a pixel at the specified coordinates.
     * Returns an optional pixel - if coordinates are out of bounds, returns an empty optional.
     * 
     * @param I_Pos Position (X, Y, Z where Z is layer)
     * @return Optional pixel, empty if coordinates are out of bounds
     */
    TOptional<FPixel> FImage::
    GetPixel(const FPoint3U& I_Pos) const
    {
        // Check bounds (X=row, Y=col)
        if (I_Pos.X >= Info.Height || I_Pos.Y >= Info.Width || I_Pos.Z >= Info.Depth)
        {
            return TOptional<FPixel>{};
        }

        // Calculate pixel data pointer: (0,0)=top-left, X=row(down), Y=col(right); offset = row*RowPitch + col*BytesPerPixel
        const UInt32 BytesPerPixel = GetBytesPerPixel();
        const UInt32 RowPitch      = GetRowPitchBytes();
        const UInt32 SlicePitch    = GetSlicePitchBytes();
        const FByte* PixelData    = GetData() + (I_Pos.Z * SlicePitch) + (I_Pos.X * RowPitch) + (I_Pos.Y * BytesPerPixel);
        
        return TOptional<FPixel>{FPixel{PixelData, Info.PixelFormat, static_cast<UInt8>(BytesPerPixel)}};
    }

    FPixel FImage::
    operator()(UInt32 I_X, UInt32 I_Y, UInt32 I_Z)
    {
        const UInt32 BytesPerPixel = GetBytesPerPixel();
        const UInt32 RowPitch = GetRowPitchBytes();
        const UInt32 SlicePitch = GetSlicePitchBytes();
        FByte* PixelData = AccessData() + (I_Z * SlicePitch) + (I_X * RowPitch) + (I_Y * BytesPerPixel);
        return FPixel{PixelData, Info.PixelFormat, static_cast<UInt8>(BytesPerPixel)};
    }

    FPixel FImage::
    operator()(UInt32 I_X, UInt32 I_Y, UInt32 I_Z) const
    {
        const UInt32 BytesPerPixel = GetBytesPerPixel();
        const UInt32 RowPitch = GetRowPitchBytes();
        const UInt32 SlicePitch = GetSlicePitchBytes();
        const FByte* PixelData = GetData() + (I_Z * SlicePitch) + (I_X * RowPitch) + (I_Y * BytesPerPixel);
        return FPixel{PixelData, Info.PixelFormat, static_cast<UInt8>(BytesPerPixel)};
    }

    /**
     * Creates a new FImage by slicing a region from this image.
     * Interface is similar to View3D, but returns a new FImage instead of a view.
     * The returned image is tightly-packed and contains a copy of the specified region.
     * 
     * Usage:
     * auto SlicedImage = Image.Slice({MinX, MinY, MinLayer}, {MaxX, MaxY, MaxLayer});
     * 
     * @param I_Min Minimum coordinates (X, Y, Z where Z is layer)
     * @param I_Max Maximum coordinates (X, Y, Z where Z is layer)
     * @return A new FImage containing the sliced region
     */
    FImage FImage::
    Slice(const FPoint3U& I_Min, const FPoint3U& I_Max) const
    {
        const FImageView3D View3D = this->View3D(I_Min, I_Max);
        return FImage{View3D, Info.MemoryArena};
    }

    /**
     * Clones the image with optional format conversion and resizing.
     * When parameters are set to default values (Invalid/Unknown/~0U), uses original image values (1:1 clone).
     * 
     * @param I_NewColorSpace Target color space (default: EColorSpace::Unknown = use original)
     * @param I_NewPixelFormat Target pixel format (default: EPixelFormat::Invalid = use original)
     * @param I_NewWidth Target width (default: ~0U = use original)
     * @param I_NewHeight Target height (default: ~0U = use original)
     * @param I_NewDepth Target depth (default: ~0U = use original)
     * @return Cloned image with specified format and dimensions
     */
    FImage FImage::
    Clone(EColorSpace I_NewColorSpace,
          EPixelFormat I_NewPixelFormat,
          UInt32 I_NewWidth,
          UInt32 I_NewHeight,
          UInt32 I_NewDepth) const
    {
        // Use original values if defaults are provided
        const EColorSpace TargetColorSpace = (I_NewColorSpace == EColorSpace::Unknown) ? Info.ColorSpace : I_NewColorSpace;
        const EPixelFormat TargetPixelFormat = (I_NewPixelFormat == EPixelFormat::Invalid) ? Info.PixelFormat : I_NewPixelFormat;
        const UInt32 TargetWidth = (I_NewWidth == ~0U) ? Info.Width : I_NewWidth;
        const UInt32 TargetHeight = (I_NewHeight == ~0U) ? Info.Height : I_NewHeight;
        const UInt32 TargetDepth = (I_NewDepth == ~0U) ? Info.Depth : I_NewDepth;

        // If all parameters match original, return copy
        if (TargetColorSpace == Info.ColorSpace &&
            TargetPixelFormat == Info.PixelFormat &&
            TargetWidth == Info.Width &&
            TargetHeight == Info.Height &&
            TargetDepth == Info.Depth)
        {
            return *this;
        }

        // Create target image
        FImage::FCreateInfo CreateInfo
        {
            .Width = TargetWidth,
            .Height = TargetHeight,
            .Depth = TargetDepth,
            .PixelFormat = TargetPixelFormat,
            .ColorSpace = TargetColorSpace,
        };
        FImage ClonedImage(CreateInfo);

        // If dimensions changed, we need to resize first
        // For now, we'll only handle format/color space conversion with same dimensions
        // Resizing can be done separately with Resize() method
        if (TargetWidth != Info.Width || TargetHeight != Info.Height || TargetDepth != Info.Depth)
        {
            // For different dimensions, we'd need to resize first
            // For now, just copy what fits
            const UInt32 CopyWidth = Math::Min(TargetWidth, Info.Width);
            const UInt32 CopyHeight = Math::Min(TargetHeight, Info.Height);
            const UInt32 CopyDepth = Math::Min(TargetDepth, Info.Depth);

            const auto SrcView = View3D(FPoint3U{0, 0, 0}, FPoint3U{CopyWidth - 1, CopyHeight - 1, CopyDepth - 1});
            const auto DstView = ClonedImage.View3D(FPoint3U{0, 0, 0}, FPoint3U{CopyWidth - 1, CopyHeight - 1, CopyDepth - 1});

            auto SrcIt = SrcView.begin();
            auto DstIt = DstView.begin();
            const auto SrcEnd = SrcView.end();
            const auto DstEnd = DstView.end();

            // Convert color space if needed
            const Bool bNeedColorSpaceConversion = (TargetColorSpace != Info.ColorSpace);

            for (; SrcIt != SrcEnd && DstIt != DstEnd; ++SrcIt, ++DstIt)
            {
                FLinearColor LinearColor = (*SrcIt).GetColor<FLinearColor>();

                // Handle color space conversion
                if (bNeedColorSpaceConversion)
                {
                    if (Info.ColorSpace == EColorSpace::sRGB && TargetColorSpace == EColorSpace::Linear)
                    {
                        // sRGB to Linear: GetColor returns raw linear values
                    }
                    else if (Info.ColorSpace == EColorSpace::Linear && TargetColorSpace == EColorSpace::sRGB)
                    {
                        // Linear to sRGB: would need conversion (limitation)
                    }
                }

                // Clamp to 0-1 for UNorm formats, preserve HDR for Float formats
                const Bool IsHDRFormat = FPixel::IsFloatFormat(TargetPixelFormat);
                if (!IsHDRFormat)
                {
                    LinearColor.R = Math::Clamp(LinearColor.R, 0.0f, 1.0f);
                    LinearColor.G = Math::Clamp(LinearColor.G, 0.0f, 1.0f);
                    LinearColor.B = Math::Clamp(LinearColor.B, 0.0f, 1.0f);
                    LinearColor.A = Math::Clamp(LinearColor.A, 0.0f, 1.0f);
                }

                (*DstIt).SetColor(LinearColor);
            }
        }
        else
        {
            // Same dimensions, just convert format/color space
            const auto SrcView = View3D();
            auto DstView = ClonedImage.View3D();

            auto SrcIt = SrcView.begin();
            auto DstIt = DstView.begin();
            const auto SrcEnd = SrcView.end();
            const auto DstEnd = DstView.end();

            // Convert color space if needed
            const Bool bNeedColorSpaceConversion = (TargetColorSpace != Info.ColorSpace);

            for (; SrcIt != SrcEnd && DstIt != DstEnd; ++SrcIt, ++DstIt)
            {
                FLinearColor LinearColor = (*SrcIt).GetColor<FLinearColor>();

                // Handle color space conversion
                if (bNeedColorSpaceConversion)
                {
                    if (Info.ColorSpace == EColorSpace::sRGB && TargetColorSpace == EColorSpace::Linear)
                    {
                        // sRGB to Linear: GetColor returns raw linear values
                    }
                    else if (Info.ColorSpace == EColorSpace::Linear && TargetColorSpace == EColorSpace::sRGB)
                    {
                        // Linear to sRGB: would need conversion (limitation)
                    }
                }

                // Clamp to 0-1 for UNorm formats, preserve HDR for Float formats
                const Bool IsHDRFormat = FPixel::IsFloatFormat(TargetPixelFormat);
                if (!IsHDRFormat)
                {
                    LinearColor.R = Math::Clamp(LinearColor.R, 0.0f, 1.0f);
                    LinearColor.G = Math::Clamp(LinearColor.G, 0.0f, 1.0f);
                    LinearColor.B = Math::Clamp(LinearColor.B, 0.0f, 1.0f);
                    LinearColor.A = Math::Clamp(LinearColor.A, 0.0f, 1.0f);
                }

                (*DstIt).SetColor(LinearColor);
            }
        }

        return ClonedImage;
    }

    // Implementation of FImageView2D constructor
    /**
     * Represents a safe 2D view (rectangular region) of an image on a single layer.
     * Provides read-write access to pixels with bounds checking.
     * This is the building block for FImageView3D.
     */
    FImageView2D::
    FImageView2D(const FImage& I_Image,
                 UInt32        I_Layer,
                 const FPoint2U& I_Min,
                 const FPoint2U& I_Max)
        : Image{const_cast<FImage*>(&I_Image)}
        , Layer{I_Layer}
    {
        // Clamp coordinates to valid image bounds
        const UInt32 ImageMaxX = I_Image.GetWidth() > 0 ? I_Image.GetWidth() - 1 : 0;
        const UInt32 ImageMaxY = I_Image.GetHeight() > 0 ? I_Image.GetHeight() - 1 : 0;
        const UInt32 ImageMaxLayer = I_Image.GetDepth() > 0 ? I_Image.GetDepth() - 1 : 0;

        // Check if layer is valid
        if (I_Layer > ImageMaxLayer)
        {
            // Invalid layer - set Max < Min to make iteration fail
            IntervalX = TClosedInterval<UInt32>{1, 0};
            IntervalY = TClosedInterval<UInt32>{1, 0};
            return;
        }

        // Clamp X coordinates
        const UInt32 ClampedMinX = Math::Min(ImageMaxX, I_Min.X);
        const UInt32 ClampedMaxX = Math::Max(ClampedMinX, Math::Min(ImageMaxX, I_Max.X));
        
        // Clamp Y coordinates
        const UInt32 ClampedMinY = Math::Min(ImageMaxY, I_Min.Y);
        const UInt32 ClampedMaxY = Math::Max(ClampedMinY, Math::Min(ImageMaxY, I_Max.Y));

        // Check if the resulting interval is valid
        if (ClampedMaxX < ClampedMinX || ClampedMaxY < ClampedMinY)
        {
            // Invalid interval - set Max < Min to make iteration fail
            IntervalX = TClosedInterval<UInt32>{1, 0};
            IntervalY = TClosedInterval<UInt32>{1, 0};
        }
        else
        {
            IntervalX = TClosedInterval<UInt32>{ClampedMinX, ClampedMaxX};
            IntervalY = TClosedInterval<UInt32>{ClampedMinY, ClampedMaxY};
        }
    }

    /**
     * Represents a safe 3D view of an image across multiple layers.
     * This is essentially multiple FImageView2D objects concatenated together.
     * Provides iteration over all pixels across all layers in the view.
     * 
     * Creates a 3D view from Min to Max coordinates (inclusive).
     * The view is automatically clamped to valid image bounds.
     * 
     * @param I_Image The image to create a view of
     * @param I_Min Minimum coordinates (X, Y, Z where Z is layer)
     * @param I_Max Maximum coordinates (X, Y, Z where Z is layer)
     */
    FImageView3D::
    FImageView3D(const FImage& I_Image, const FPoint3U& I_Min, const FPoint3U& I_Max)
        : Image{const_cast<FImage*>(&I_Image)}
    {
        // Clamp coordinates to valid image bounds
        const UInt32 ImageMaxLayer = I_Image.GetDepth() > 0 ? I_Image.GetDepth() - 1 : 0;
        const UInt32 ImageMaxX = I_Image.GetWidth() > 0 ? I_Image.GetWidth() - 1 : 0;
        const UInt32 ImageMaxY = I_Image.GetHeight() > 0 ? I_Image.GetHeight() - 1 : 0;

        MinLayer = Math::Min(ImageMaxLayer, I_Min.Z);
        MaxLayer = Math::Max(MinLayer, Math::Min(ImageMaxLayer, I_Max.Z));
        
        const UInt32 ClampedMinX = Math::Min(ImageMaxX, I_Min.X);
        const UInt32 ClampedMaxX = Math::Max(ClampedMinX, Math::Min(ImageMaxX, I_Max.X));
        const UInt32 ClampedMinY = Math::Min(ImageMaxY, I_Min.Y);
        const UInt32 ClampedMaxY = Math::Max(ClampedMinY, Math::Min(ImageMaxY, I_Max.Y));

        IntervalX = TClosedInterval<UInt32>{ClampedMinX, ClampedMaxX};
        IntervalY = TClosedInterval<UInt32>{ClampedMinY, ClampedMaxY};
    }

    // Implementation of FImageViewIterator::GetPixelCount
    UInt64 FImageViewIterator::
    GetPixelCount() const
    {
        if (!View)
        { return 0; }
        const UInt32 RegionWidth = View->IntervalX.Length() + 1;
        const UInt32 RegionHeight = View->IntervalY.Length() + 1;
        return static_cast<UInt64>(RegionWidth) * static_cast<UInt64>(RegionHeight);
    }

    // Implementation of FImageViewIterator::UpdatePixel
    void FImageViewIterator::
    UpdatePixel() const
    {
        if (CachedPixelIndex == PixelIndex && CurrentPixel.GetData() != nullptr)
        { return; }

        if (!View || PixelIndex >= GetPixelCount())
        {
            CurrentPixel = FPixel{nullptr, EPixelFormat::Invalid, 0};
            CachedPixelIndex = PixelIndex;
            return;
        }

        const UInt32 RegionWidth = View->IntervalX.Length() + 1;
        const UInt32 LocalX = static_cast<UInt32>(PixelIndex % RegionWidth);
        const UInt32 LocalY = static_cast<UInt32>(PixelIndex / RegionWidth);
        const UInt32 X = View->IntervalX.Left + LocalX;
        const UInt32 Y = View->IntervalY.Left + LocalY;

        if (X > View->IntervalX.Right || Y > View->IntervalY.Right)
        {
            CurrentPixel = FPixel{nullptr, EPixelFormat::Invalid, 0};
            CachedPixelIndex = PixelIndex;
            return;
        }

        const UInt32 BytesPerPixel = View->Image->GetBytesPerPixel();
        const UInt32 RowPitch = View->Image->GetRowPitchBytes();
        const UInt32 SlicePitch = View->Image->GetSlicePitchBytes();
        const UInt32 ClampedLayer = Math::Min(View->Layer, View->Image->GetDepth() > 0 ? View->Image->GetDepth() - 1 : 0);
        FByte* PixelData = View->Image->AccessData() + (ClampedLayer * SlicePitch) + (Y * RowPitch + X * BytesPerPixel);
        CurrentPixel = FPixel{PixelData, View->Image->GetPixelFormat(), static_cast<UInt8>(BytesPerPixel)};
        CachedPixelIndex = PixelIndex;
    }

    FImageViewIterator FImageView2D::
    end() const
    {
        const UInt32 RegionWidth = IntervalX.Length() + 1;
        const UInt32 RegionHeight = IntervalY.Length() + 1;
        const UInt64 TotalPixels = static_cast<UInt64>(RegionWidth) * static_cast<UInt64>(RegionHeight);
        return FImageViewIterator{const_cast<FImageView2D*>(this), TotalPixels};
    }

    /**
     * Sets a pixel at the specified coordinates with bounds checking.
     * Coordinates are relative to the view's interval, not the image.
     * 
     * @param I_Pos Position relative to the view (X relative to IntervalX.Left, Y relative to IntervalY.Left)
     * @param I_Color Color to set
     * @return True if the pixel was set successfully, False if coordinates are out of bounds
     */
    template<Concepts::Color TColor>
    Bool FImageView2D::
    SetPixel(const FPoint2U& I_Pos, const TColor& I_Color)
    {
        // Check bounds relative to the view
        const UInt32 RegionWidth = IntervalX.Length() + 1;
        const UInt32 RegionHeight = IntervalY.Length() + 1;
        
        if (I_Pos.X >= RegionWidth || I_Pos.Y >= RegionHeight)
        {
            return False;
        }

        // Convert to absolute coordinates
        const UInt32 AbsX = IntervalX.Left + I_Pos.X;
        const UInt32 AbsY = IntervalY.Left + I_Pos.Y;

        // Double-check absolute bounds (should always pass if view is valid)
        if (AbsX > IntervalX.Right || AbsY > IntervalY.Right)
        {
            return False;
        }

        const UInt32 BytesPerPixel = Image->GetBytesPerPixel();
        const UInt32 RowPitch = Image->GetRowPitchBytes();
        const UInt32 SlicePitch = Image->GetSlicePitchBytes();
        FByte* PixelData = Image->AccessData() + (Layer * SlicePitch) + (AbsY * RowPitch) + (AbsX * BytesPerPixel);
        FPixel Pixel{PixelData, Image->GetPixelFormat(), static_cast<UInt8>(BytesPerPixel)};
        Pixel.SetColor(I_Color);
        return True;
    }

    FPixel FImageView2D::
    operator()(UInt32 I_X, UInt32 I_Y)
    {
        const UInt32 AbsX = IntervalX.Left + I_X;
        const UInt32 AbsY = IntervalY.Left + I_Y;
        return Image->operator()(AbsY, AbsX, Layer);
    }

    FPixel FImageView2D::
    operator()(UInt32 I_X, UInt32 I_Y) const
    {
        const UInt32 AbsX = IntervalX.Left + I_X;
        const UInt32 AbsY = IntervalY.Left + I_Y;
        return Image->operator()(AbsY, AbsX, Layer);
    }

    // Implementation of FImageView3DIterator::GetPixelCount
    UInt64 FImageView3DIterator::
    GetPixelCount() const
    {
        if (!View)
        { return 0; }
        return View->GetPixelCount();
    }

    // Implementation of FImageView3DIterator::UpdatePixel
    void FImageView3DIterator::
    UpdatePixel() const
    {
        if (CachedPixelIndex == PixelIndex && CurrentPixel.GetData() != nullptr)
        { return; }

        if (!View || PixelIndex >= GetPixelCount())
        {
            CurrentPixel = FPixel{nullptr, EPixelFormat::Invalid, 0};
            CachedPixelIndex = PixelIndex;
            return;
        }

        const UInt32 RegionWidth = View->IntervalX.Length() + 1;
        const UInt32 RegionHeight = View->IntervalY.Length() + 1;
        const UInt32 PixelsPerLayer = RegionWidth * RegionHeight;
        
        const UInt32 LayerIndex = static_cast<UInt32>(PixelIndex / PixelsPerLayer);
        const UInt32 LocalPixelIndex = static_cast<UInt32>(PixelIndex % PixelsPerLayer);
        const UInt32 X = LocalPixelIndex % RegionWidth;
        const UInt32 Y = LocalPixelIndex / RegionWidth;
        
        const UInt32 AbsLayer = View->MinLayer + LayerIndex;
        const UInt32 AbsX = View->IntervalX.Left + X;
        const UInt32 AbsY = View->IntervalY.Left + Y;

        if (AbsLayer > View->MaxLayer || AbsX > View->IntervalX.Right || AbsY > View->IntervalY.Right)
        {
            CurrentPixel = FPixel{nullptr, EPixelFormat::Invalid, 0};
            CachedPixelIndex = PixelIndex;
            return;
        }

        const UInt32 BytesPerPixel = View->Image->GetBytesPerPixel();
        const UInt32 RowPitch = View->Image->GetRowPitchBytes();
        const UInt32 SlicePitch = View->Image->GetSlicePitchBytes();
        FByte* PixelData = View->Image->AccessData() + (AbsLayer * SlicePitch) + (AbsY * RowPitch) + (AbsX * BytesPerPixel);
        CurrentPixel = FPixel{PixelData, View->Image->GetPixelFormat(), static_cast<UInt8>(BytesPerPixel)};
        CachedPixelIndex = PixelIndex;
    }

    FImageView3DIterator FImageView3D::
    end() const
    {
        return FImageView3DIterator{const_cast<FImageView3D*>(this), GetPixelCount()};
    }

    UInt64 FImageView3D::
    GetPixelCount() const
    {
        if (!Image)
        { return 0; }
        const UInt32 RegionWidth = IntervalX.Length() + 1;
        const UInt32 RegionHeight = IntervalY.Length() + 1;
        const UInt32 RegionDepth = MaxLayer - MinLayer + 1;
        return static_cast<UInt64>(RegionWidth) * static_cast<UInt64>(RegionHeight) * static_cast<UInt64>(RegionDepth);
    }

    FPixel FImageView3D::
    operator()(UInt32 I_X, UInt32 I_Y, UInt32 I_Z)
    {
        const UInt32 AbsX = IntervalX.Left + I_X;
        const UInt32 AbsY = IntervalY.Left + I_Y;
        const UInt32 AbsZ = MinLayer + I_Z;
        return Image->operator()(AbsY, AbsX, AbsZ);
    }

    FPixel FImageView3D::
    operator()(UInt32 I_X, UInt32 I_Y, UInt32 I_Z) const
    {
        const UInt32 AbsX = IntervalX.Left + I_X;
        const UInt32 AbsY = IntervalY.Left + I_Y;
        const UInt32 AbsZ = MinLayer + I_Z;
        return Image->operator()(AbsY, AbsX, AbsZ);
    }
}