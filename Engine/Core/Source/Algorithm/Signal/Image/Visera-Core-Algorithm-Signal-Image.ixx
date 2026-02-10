module;
#include <Visera-Core.hpp>
export module Visera.Core.Algorithm.Signal.Image;
#define VISERA_MODULE_NAME "Core.Algorithm"
import Visera.Core.Image;
import Visera.Core.Math.Kernel.Gaussian;
import Visera.Core.Math.Color.Linear;
import Visera.Core.Math.Arithmetic.Operation;
import Visera.Core.Containers.Array;

namespace Visera::Algorithm
{
    export enum class EAddressMode : UInt8
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder,
    };

    [[nodiscard]] VISERA_CORE_API inline UInt32
    ApplyAddressModeX(Int32 I_X, UInt32 I_Width, EAddressMode I_Mode)
    {
        if (I_Width == 0) return 0;
        switch (I_Mode)
        {
        case EAddressMode::ClampToEdge:
            return static_cast<UInt32>(Math::Clamp(I_X, 0, static_cast<Int32>(I_Width) - 1));
        case EAddressMode::Repeat:
        {
            Int32 X = I_X % static_cast<Int32>(I_Width);
            if (X < 0) X += static_cast<Int32>(I_Width);
            return static_cast<UInt32>(X);
        }
        case EAddressMode::MirroredRepeat:
        {
            const Int32 W = static_cast<Int32>(I_Width);
            const Int32 Period = 2 * W;
            Int32 X = (I_X % Period + Period) % Period;
            return static_cast<UInt32>(X >= W ? 2 * W - 1 - X : X);
        }
        case EAddressMode::ClampToBorder:
            return (I_X >= 0 && I_X < static_cast<Int32>(I_Width))
                ? static_cast<UInt32>(I_X)
                : UInt32(-1);  // Sentinel for border (transparent black)
        }
        return 0;
    }

    [[nodiscard]] VISERA_CORE_API inline UInt32
    ApplyAddressModeY(Int32 I_Y, UInt32 I_Height, EAddressMode I_Mode)
    {
        if (I_Height == 0) return 0;
        switch (I_Mode)
        {
        case EAddressMode::ClampToEdge:
            return static_cast<UInt32>(Math::Clamp(I_Y, 0, static_cast<Int32>(I_Height) - 1));
        case EAddressMode::Repeat:
        {
            Int32 Y = I_Y % static_cast<Int32>(I_Height);
            if (Y < 0) Y += static_cast<Int32>(I_Height);
            return static_cast<UInt32>(Y);
        }
        case EAddressMode::MirroredRepeat:
        {
            const Int32 H = static_cast<Int32>(I_Height);
            const Int32 Period = 2 * H;
            Int32 Y = (I_Y % Period + Period) % Period;
            return static_cast<UInt32>(Y >= H ? 2 * H - 1 - Y : Y);
        }
        case EAddressMode::ClampToBorder:
            return (I_Y >= 0 && I_Y < static_cast<Int32>(I_Height))
                ? static_cast<UInt32>(I_Y)
                : UInt32(-1);
        }
        return 0;
    }

    VISERA_CORE_API void
    ConvolveHorizontal(
        const FImageView2D& I_SrcView,
        FImageView2D&       O_DstView,
        const TArray<Float>& I_Kernel,
        UInt32              I_Radius,
        EAddressMode       I_Mode)
    {
        const UInt32 W = I_SrcView.GetIntervalX().Length() + 1;
        const UInt32 H = I_SrcView.GetIntervalY().Length() + 1;
        const UInt32 SrcLeftX = I_SrcView.GetIntervalX().Left;
        const UInt32 SrcLeftY = I_SrcView.GetIntervalY().Left;

        for (UInt32 y = 0; y < H; ++y)
        {
            for (UInt32 x = 0; x < W; ++x)
            {
                FLinearColor Acc{0.0f, 0.0f, 0.0f, 0.0f};
                Float WeightSum = 0;
                for (UInt32 k = 0; k < I_Kernel.GetSize(); ++k)
                {
                    const Int32 LocalX = static_cast<Int32>(x) - static_cast<Int32>(I_Radius) + static_cast<Int32>(k);
                    const UInt32 SampledX = ApplyAddressModeX(LocalX, W, I_Mode);
                    if (I_Mode == EAddressMode::ClampToBorder && SampledX == UInt32(-1))
                        continue;
                    const UInt32 AbsX = SrcLeftX + SampledX;
                    const UInt32 AbsY = SrcLeftY + y;
                    FLinearColor C = I_SrcView.GetImage()->operator()(AbsX, AbsY, I_SrcView.GetLayer()).GetColor<FLinearColor>();
                    const Float Wt = I_Kernel[k];
                    Acc.R += C.R * Wt; Acc.G += C.G * Wt; Acc.B += C.B * Wt; Acc.A += C.A * Wt;
                    WeightSum += Wt;
                }
                if (WeightSum > 0)
                {
                    Acc.R /= WeightSum; Acc.G /= WeightSum; Acc.B /= WeightSum; Acc.A /= WeightSum;
                }
                O_DstView(x, y) = Acc;
            }
        }
    }

    VISERA_CORE_API void
    ConvolveVertical(
        const FImageView2D& I_SrcView,
        FImageView2D&       O_DstView,
        const TArray<Float>& I_Kernel,
        UInt32              I_Radius,
        EAddressMode       I_Mode)
    {
        const UInt32 W = I_SrcView.GetIntervalX().Length() + 1;
        const UInt32 H = I_SrcView.GetIntervalY().Length() + 1;
        const UInt32 SrcLeftX = I_SrcView.GetIntervalX().Left;
        const UInt32 SrcLeftY = I_SrcView.GetIntervalY().Left;

        for (UInt32 y = 0; y < H; ++y)
        {
            for (UInt32 x = 0; x < W; ++x)
            {
                FLinearColor Acc{0.0f, 0.0f, 0.0f, 0.0f};
                Float WeightSum = 0;
                for (UInt32 k = 0; k < I_Kernel.GetSize(); ++k)
                {
                    const Int32 LocalY = static_cast<Int32>(y) - static_cast<Int32>(I_Radius) + static_cast<Int32>(k);
                    const UInt32 SampledY = ApplyAddressModeY(LocalY, H, I_Mode);
                    if (I_Mode == EAddressMode::ClampToBorder && SampledY == UInt32(-1))
                        continue;
                    const UInt32 AbsX = SrcLeftX + x;
                    const UInt32 AbsY = SrcLeftY + SampledY;
                    FLinearColor C = I_SrcView.GetImage()->operator()(AbsX, AbsY, I_SrcView.GetLayer()).GetColor<FLinearColor>();
                    const Float Wt = I_Kernel[k];
                    Acc.R += C.R * Wt; Acc.G += C.G * Wt; Acc.B += C.B * Wt; Acc.A += C.A * Wt;
                    WeightSum += Wt;
                }
                if (WeightSum > 0)
                {
                    Acc.R /= WeightSum; Acc.G /= WeightSum; Acc.B /= WeightSum; Acc.A /= WeightSum;
                }
                O_DstView(x, y) = Acc;
            }
        }
    }
}

export namespace Visera::Algorithm
{
    [[nodiscard]] VISERA_CORE_API FImage
    GaussianBlur(const FImageView2D& I_SrcView,
                 Float               I_Sigma,
                 UInt32              I_Radius,
                 EAddressMode        I_AddressMode)
    {
        const UInt32 W = I_SrcView.GetIntervalX().Length() + 1;
        const UInt32 H = I_SrcView.GetIntervalY().Length() + 1;

        FImage Result{{
            .Width = W,
            .Height = H,
            .Depth = 1,
            .PixelFormat = I_SrcView.GetImage()->GetPixelFormat(),
            .ColorSpace = I_SrcView.GetImage()->GetColorSpace(),
        }};

        if (W == 0 || H == 0) return Result;

        TArray<Float> Kernel = Math::Gaussian1D(I_Sigma, I_Radius);

        FImage TempImage{{
            .Width = W,
            .Height = H,
            .Depth = 1,
            .PixelFormat = I_SrcView.GetImage()->GetPixelFormat(),
            .ColorSpace = I_SrcView.GetImage()->GetColorSpace(),
        }};

        FImageView2D TempView{TempImage, 0, {0, 0}, {W > 0 ? W - 1 : 0, H > 0 ? H - 1 : 0}};
        FImageView2D DstView{Result, 0, {0, 0}, {W > 0 ? W - 1 : 0, H > 0 ? H - 1 : 0}};

        ConvolveHorizontal(I_SrcView, TempView, Kernel, I_Radius, I_AddressMode);
        ConvolveVertical(TempView, DstView, Kernel, I_Radius, I_AddressMode);

        return Result;
    }
}
