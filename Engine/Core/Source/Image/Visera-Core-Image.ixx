module;
#include <Visera-Core.hpp>
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include <stb_image_resize2.h>
export module Visera.Core.Image;
#define VISERA_MODULE_NAME "Core.Image"
export import Visera.Core.Image.Pixel;
export import Visera.Core.Image.Common;
       import Visera.Core.Math.Color;
       import Visera.Core.Types.Array;
       import Visera.Core.Types.Half;
       import Visera.Core.OS.Memory;
       import Visera.Core.Math.Arithmetic.Operation;
       import Visera.Core.Math.Arithmetic.Interval;

export namespace Visera
{
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
        GetRowPitchBytes() const { return Info.RowPitchBytes > 0? Info.RowPitchBytes : Info.Width * FPixel::GetByteSize(Info.PixelFormat); }
        [[nodiscard]] inline UInt32
        GetSlicePitchBytes() const { return Info.SlicePitchBytes > 0? Info.SlicePitchBytes : GetRowPitchBytes() * Info.Height; }
        [[nodiscard]] inline UInt8
        GetBytesPerPixel() const { return FPixel::GetByteSize(Info.PixelFormat); }
        [[nodiscard]] inline UInt8
        GetChannelCount() const { return FPixel::GetChannelCount(Info.PixelFormat); }
        [[nodiscard]] inline UInt64
        GetSizeInBytes() const { return static_cast<UInt64>(GetSlicePitchBytes()) * Info.Depth; }

        [[nodiscard]] Bool
        Resize(UInt32 I_NewWidth, UInt32 I_NewHeight);

        [[nodiscard]] inline Bool
        IsRGBA() const { return Info.PixelFormat == EPixelFormat::RGBA8_UNorm || Info.PixelFormat == EPixelFormat::RGBA16_UNorm || Info.PixelFormat == EPixelFormat::RGBA16_Float || Info.PixelFormat == EPixelFormat::RGBA32_Float; }
        [[nodiscard]] inline Bool
        IsBGRA() const { return Info.PixelFormat == EPixelFormat::BGRA8_UNorm; }
        [[nodiscard]] inline Bool
        HasAlpha() const { return FPixel::HasAlpha(Info.PixelFormat); }
        [[nodiscard]] inline Bool
        IsFloatFormat() const { return FPixel::IsFloatFormat(Info.PixelFormat); }

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
                // Use const FByte* constructor to create a read-only pixel view
                const FByte* ConstPixelData = Image->GetData() + (ClampedLayer * SlicePitch) + (Y * RowPitch + X * BytesPerPixel);
                CurrentPixel = FPixel{ConstPixelData, Image->GetPixelFormat(), static_cast<UInt8>(BytesPerPixel)};
                CachedPixelIndex = PixelIndex;
            }
        };

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
                FImage* I_Image,
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
                return Image == I_Other.Image &&
                       IntervalX == I_Other.IntervalX &&
                       IntervalY == I_Other.IntervalY &&
                       Layer == I_Other.Layer &&
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
            FImage* Image = nullptr;
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
                FByte* PixelData = Image->AccessData() + (ClampedLayer * SlicePitch) + (Y * RowPitch + X * BytesPerPixel);
                CurrentPixel = FPixel{PixelData, Image->GetPixelFormat(), static_cast<UInt8>(BytesPerPixel)};
                CachedPixelIndex = PixelIndex;
            }
        };

        /**
         * Represents a 2D view (rectangular region) of an image that can be iterated.
         * Provides read-write access to pixels.
         */
        class VISERA_CORE_API FImageView
        {
        public:
            FImageView(FImage*                  I_Image,
                       UInt32                   I_Layer,
                       TClosedInterval<UInt32>  I_IntervalX,
                       TClosedInterval<UInt32>  I_IntervalY)
                : Image     {I_Image}
                , Layer     {I_Layer}
                , IntervalX {I_IntervalX}
                , IntervalY {I_IntervalY}
            {
            }

            [[nodiscard]] FImageViewIterator
            begin()
            {
                return FImageViewIterator{Image, IntervalX, IntervalY, Layer, 0};
            }

            [[nodiscard]] FImageViewIterator
            end()
            {
                const UInt32 RegionWidth = IntervalX.Length() + 1;
                const UInt32 RegionHeight = IntervalY.Length() + 1;
                const UInt64 TotalPixels = static_cast<UInt64>(RegionWidth) * static_cast<UInt64>(RegionHeight);
                return FImageViewIterator{Image, IntervalX, IntervalY, Layer, TotalPixels};
            }

        private:
            FImage* Image = nullptr;
            TClosedInterval<UInt32> IntervalX{0, 0};
            TClosedInterval<UInt32> IntervalY{0, 0};
            UInt32 Layer = 0;
        };

        /**
         * Represents a const 2D view (rectangular region) of an image that can be iterated.
         * Provides read-only access to pixels.
         */
        class VISERA_CORE_API FConstImageView
        {
        public:
            FConstImageView(const FImage*            I_Image,
                            UInt32                   I_Layer,
                            TClosedInterval<UInt32>  I_IntervalX,
                            TClosedInterval<UInt32>  I_IntervalY)
                : Image     {I_Image}
                , Layer     {I_Layer}
                , IntervalX {I_IntervalX}
                , IntervalY {I_IntervalY}
            {
            }

            [[nodiscard]] FConstImageViewIterator
            begin() const
            {
                return FConstImageViewIterator{Image, IntervalX, IntervalY, Layer, 0};
            }

            [[nodiscard]] FConstImageViewIterator
            end() const
            {
                const UInt32 RegionWidth = IntervalX.Length() + 1;
                const UInt32 RegionHeight = IntervalY.Length() + 1;
                const UInt64 TotalPixels = static_cast<UInt64>(RegionWidth) * static_cast<UInt64>(RegionHeight);
                return FConstImageViewIterator{Image, IntervalX, IntervalY, Layer, TotalPixels};
            }

        private:
            const FImage* Image = nullptr;
            TClosedInterval<UInt32> IntervalX{0, 0};
            TClosedInterval<UInt32> IntervalY{0, 0};
            UInt32 Layer = 0;
        };

        /**
         * Create a safe iterable 2D view of the image.
         * The view automatically clamps X/Y intervals and Layer to valid ranges.
         *
         * Usage:
         * for (auto& Pixel : Image.View({MinX, MaxX}, {MinY, MaxY}, Layer 0)) { ... }
         *
         * Passing default-constructed intervals ({}) yields full-range views.
         */
        [[nodiscard]] FImageView
        View(
            UInt32                  I_Layer     = 0,
            TClosedInterval<UInt32> I_IntervalX = {0, Math::UpperBound<UInt32>()},
            TClosedInterval<UInt32> I_IntervalY = {0, Math::UpperBound<UInt32>()})
        {
            const UInt32 MaxLayer = Info.Depth > 0 ? Info.Depth - 1 : 0;
            if (I_Layer > MaxLayer)
            {
                VISERA_ASSERT(False);
                // Return an empty view: Right < Left means zero pixels
                return FImageView{this, I_Layer, {1, 0}, {1, 0}};
            }

            const UInt32 MaxX = Info.Width > 0? Info.Width - 1 : 0;
            const UInt32 MaxY = Info.Height > 0? Info.Height - 1 : 0;

            I_IntervalX.Right = Math::Min(I_IntervalX.Right, MaxX);
            I_IntervalX.Left  = Math::Min(I_IntervalX.Left, I_IntervalX.Right);
            I_IntervalY.Right = Math::Min(I_IntervalY.Right, MaxY);
            I_IntervalY.Left  = Math::Min(I_IntervalY.Left, I_IntervalY.Right);

            return FImageView{this, I_Layer, I_IntervalX, I_IntervalY};
        }

        /**
         * Create a safe iterable const 2D view of the image.
         * The view automatically clamps X/Y intervals and Layer to valid ranges.
         * Provides read-only access to pixels.
         *
         * Usage:
         * for (const auto& Pixel : ConstImage.View({MinX, MaxX}, {MinY, MaxY}, Layer 0)) { ... }
         *
         * Passing default-constructed intervals ({}) yields full-range views.
         */
        [[nodiscard]] FConstImageView
        View(UInt32                  I_Layer     = 0,
             TClosedInterval<UInt32> I_IntervalX = {0, Math::UpperBound<UInt32>()},
             TClosedInterval<UInt32> I_IntervalY = {0, Math::UpperBound<UInt32>()}) const
        {
            const UInt32 MaxLayer = Info.Depth > 0 ? Info.Depth - 1 : 0;
            if (I_Layer > MaxLayer)
            {
                VISERA_ASSERT(False);
                // Return an empty view: Right < Left means zero pixels
                return FConstImageView{this, I_Layer, {1, 0}, {1, 0}};
            }

            const UInt32 MaxX = Info.Width > 0? Info.Width - 1 : 0;
            const UInt32 MaxY = Info.Height > 0? Info.Height - 1 : 0;

            I_IntervalX.Right = Math::Min(I_IntervalX.Right, MaxX);
            I_IntervalX.Left  = Math::Min(I_IntervalX.Left, I_IntervalX.Right);
            I_IntervalY.Right = Math::Min(I_IntervalY.Right, MaxY);
            I_IntervalY.Left  = Math::Min(I_IntervalY.Left, I_IntervalY.Right);

            return FConstImageView{this, I_Layer, I_IntervalX, I_IntervalY};
        }

    private:
        TPMRArray<FByte> Data;
        FCreateInfo      Info;

    public:
        FImage()  = default;
        ~FImage() = default;
        FImage(const FCreateInfo& I_CreateInfo);

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

        const Bool bIsHalfFormat = (Info.PixelFormat == EPixelFormat::R16_Float   ||
                                    Info.PixelFormat == EPixelFormat::RG16_Float  ||
                                    Info.PixelFormat == EPixelFormat::RGB16_Float ||
                                    Info.PixelFormat == EPixelFormat::RGBA16_Float);
        const Bool bIsUNorm16Format = (Info.PixelFormat == EPixelFormat::R16_UNorm ||
                                       Info.PixelFormat == EPixelFormat::RG16_UNorm ||
                                       Info.PixelFormat == EPixelFormat::RGB16_UNorm ||
                                       Info.PixelFormat == EPixelFormat::RGBA16_UNorm);
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
}