module;
#include <Visera-AssetHub.hpp>
export module Visera.Runtime.AssetHub.Image;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Runtime.AssetHub.Image.Common;
export import Visera.Runtime.AssetHub.Image.Wrapper;
export import Visera.Runtime.AssetHub.Image.PNG;
export import Visera.Runtime.AssetHub.Image.EXR;
import Visera.Runtime.AssetHub.Asset;
import Visera.Core.Image;

export namespace Visera
{
	/** Read-only image asset; implements IAsset. Use FAssetHub::Save(const FImage&, path) to write. */
	class VISERA_RUNTIME_API FImageAsset : public IAsset
	{
	public:
		[[nodiscard]] const FImage&
		GetImage() const { return Image; }
		[[nodiscard]] UInt64
		GetByteSize() const override { return Image.GetSizeInBytes(); }
		// Read-only passthrough (no GetData / AccessData to avoid write risk)
		[[nodiscard]] UInt32
		GetWidth() const { return Image.GetWidth(); }
		[[nodiscard]] UInt32
		GetHeight() const { return Image.GetHeight(); }
		[[nodiscard]] UInt32
		GetDepth() const { return Image.GetDepth(); }
		[[nodiscard]] UInt8 
		GetChannelCount() const { return Image.GetChannelCount(); }
		[[nodiscard]] EPixelFormat
		GetPixelFormat() const { return Image.GetPixelFormat(); }
		[[nodiscard]] EColorSpace
		GetColorSpace() const { return Image.GetColorSpace(); }
		[[nodiscard]] UInt32
		GetRowPitchBytes() const { return Image.GetRowPitchBytes(); }
		[[nodiscard]] UInt32
		GetSlicePitchBytes() const { return Image.GetSlicePitchBytes(); }
		[[nodiscard]] UInt8
		GetBytesPerPixel() const { return Image.GetBytesPerPixel(); }
		[[nodiscard]] UInt64
		GetSizeInBytes() const { return Image.GetSizeInBytes(); }
		[[nodiscard]] Bool
		IsRGBA() const { return Image.IsRGBA(); }
		[[nodiscard]] Bool
		IsBGRA() const { return Image.IsBGRA(); }
		[[nodiscard]] Bool
		HasAlpha() const { return Image.HasAlpha(); }
		[[nodiscard]] Bool
		IsFloatFormat() const { return Image.IsFloatFormat(); }

	private:
		FImage Image;

	public:
		explicit FImageAsset(FImage I_Image) : Image(std::move(I_Image)) {}
	};
}
