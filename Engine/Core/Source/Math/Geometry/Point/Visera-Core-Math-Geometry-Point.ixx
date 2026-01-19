module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Geometry.Point;
#define VISERA_MODULE_NAME "Core.Math.Geometry"
import Visera.Core.Math.Arithmetic.Operation;
import Visera.Core.Math.Algebra.Vector;

//#define VISERA_SAFE_MODE;
#if defined(VISERA_SAFE_MODE)
#define CHECK(I_Statement) VISERA_ASSERT(I_Statement)
#else
#define CHECK(I_Statement) VISERA_NO_OPERATION
#endif

export namespace Visera
{
    class VISERA_CORE_API FPoint2I
    {
    public:
        union
        {
            struct { Int32 X; Int32 Y; };
            Int32 Data[2]{ 0, 0 };
        };

        [[nodiscard]] constexpr Int32&
        operator[](UInt32 I_Index) noexcept { CHECK(I_Index < 2); return Data[I_Index]; }
        [[nodiscard]] constexpr const Int32&
        operator[](UInt32 I_Index) const noexcept { CHECK(I_Index < 2); return Data[I_Index]; }

        [[nodiscard]] static constexpr FPoint2I
        Zero() noexcept { return FPoint2I{ 0, 0 }; }

        [[nodiscard]] constexpr Bool
        IsZero() const noexcept { return X == 0 && Y == 0; }

        [[nodiscard]] constexpr Bool
        operator==(const FPoint2I& I_Rhs) const noexcept { return X == I_Rhs.X && Y == I_Rhs.Y; }
        [[nodiscard]] constexpr Bool
        operator!=(const FPoint2I& I_Rhs) const noexcept { return !(*this == I_Rhs); }

        [[nodiscard]] constexpr FPoint2I
        operator+(const FPoint2I& I_Rhs) const noexcept { return FPoint2I{ X + I_Rhs.X, Y + I_Rhs.Y }; }
        [[nodiscard]] constexpr FPoint2I
        operator-(const FPoint2I& I_Rhs) const noexcept { return FPoint2I{ X - I_Rhs.X, Y - I_Rhs.Y }; }
        constexpr FPoint2I&
        operator+=(const FPoint2I& I_Rhs) noexcept { X += I_Rhs.X; Y += I_Rhs.Y; return *this; }
        constexpr FPoint2I&
        operator-=(const FPoint2I& I_Rhs) noexcept { X -= I_Rhs.X; Y -= I_Rhs.Y; return *this; }

        [[nodiscard]] constexpr FPoint2I
        operator*(Int32 I_Factor) const noexcept { return FPoint2I{ X * I_Factor, Y * I_Factor }; }
        [[nodiscard]] constexpr FPoint2I
        operator/(Int32 I_Divisor) const noexcept { CHECK(I_Divisor != 0); return FPoint2I{ X / I_Divisor, Y / I_Divisor }; }
        constexpr FPoint2I&
        operator*=(Int32 I_Factor) noexcept { X *= I_Factor; Y *= I_Factor; return *this; }
        constexpr FPoint2I&
        operator/=(Int32 I_Divisor) noexcept { CHECK(I_Divisor != 0); X /= I_Divisor; Y /= I_Divisor; return *this; }

        [[nodiscard]] constexpr Int64
        Dot(const FPoint2I& I_Rhs) const noexcept { return Int64(X) * Int64(I_Rhs.X) + Int64(Y) * Int64(I_Rhs.Y); }

        [[nodiscard]] constexpr Int64
        SquaredNorm() const noexcept { return Dot(*this); }

        constexpr FPoint2I() noexcept = default;
        constexpr FPoint2I(Int32 I_X, Int32 I_Y) noexcept : X(I_X), Y(I_Y) {}
    };
    static_assert(sizeof(FPoint2I) == 8);
    static_assert(std::is_standard_layout_v<FPoint2I>);

    class VISERA_CORE_API FPoint2U
    {
    public:
        union
        {
            struct { UInt32 X; UInt32 Y; };
            UInt32 Data[2]{ 0U, 0U };
        };

        [[nodiscard]] constexpr UInt32&
        operator[](UInt32 I_Index) noexcept { CHECK(I_Index < 2); return Data[I_Index]; }
        [[nodiscard]] constexpr const UInt32&
        operator[](UInt32 I_Index) const noexcept { CHECK(I_Index < 2); return Data[I_Index]; }

        [[nodiscard]] static constexpr FPoint2U
        Zero() noexcept { return FPoint2U{ 0U, 0U }; }

        [[nodiscard]] constexpr Bool
        IsZero() const noexcept { return X == 0U && Y == 0U; }

        [[nodiscard]] constexpr Bool
        operator==(const FPoint2U& I_Rhs) const noexcept { return X == I_Rhs.X && Y == I_Rhs.Y; }
        [[nodiscard]] constexpr Bool
        operator!=(const FPoint2U& I_Rhs) const noexcept { return !(*this == I_Rhs); }

        [[nodiscard]] constexpr FPoint2U
        operator+(const FPoint2U& I_Rhs) const noexcept { return FPoint2U{ X + I_Rhs.X, Y + I_Rhs.Y }; }
        [[nodiscard]] constexpr FPoint2U
        operator-(const FPoint2U& I_Rhs) const noexcept { return FPoint2U{ X - I_Rhs.X, Y - I_Rhs.Y }; }
        constexpr FPoint2U&
        operator+=(const FPoint2U& I_Rhs) noexcept { X += I_Rhs.X; Y += I_Rhs.Y; return *this; }
        constexpr FPoint2U&
        operator-=(const FPoint2U& I_Rhs) noexcept { X -= I_Rhs.X; Y -= I_Rhs.Y; return *this; }

        [[nodiscard]] constexpr UInt64
        Dot(const FPoint2U& I_Rhs) const noexcept { return UInt64(X) * UInt64(I_Rhs.X) + UInt64(Y) * UInt64(I_Rhs.Y); }

        [[nodiscard]] constexpr UInt64
        SquaredNorm() const noexcept { return Dot(*this); }

        [[nodiscard]] constexpr Bool
        IsPositive() const noexcept { return X > 0U && Y > 0U; }

        constexpr FPoint2U() noexcept = default;
        constexpr FPoint2U(UInt32 I_X, UInt32 I_Y) noexcept : X(I_X), Y(I_Y) {}
    };
    static_assert(sizeof(FPoint2U) == 8);
    static_assert(std::is_standard_layout_v<FPoint2U>);

    class VISERA_CORE_API FPoint3I
    {
    public:
        union
        {
            struct { Int32 X; Int32 Y; Int32 Z; };
            Int32 Data[3]{ 0, 0, 0 };
        };

        [[nodiscard]] constexpr Int32&
        operator[](UInt32 I_Index) noexcept { CHECK(I_Index < 3); return Data[I_Index]; }
        [[nodiscard]] constexpr const Int32&
        operator[](UInt32 I_Index) const noexcept { CHECK(I_Index < 3); return Data[I_Index]; }

        [[nodiscard]] static constexpr FPoint3I
        Zero() noexcept { return FPoint3I{ 0, 0, 0 }; }

        [[nodiscard]] constexpr Bool
        IsZero() const noexcept { return X == 0 && Y == 0 && Z == 0; }

        [[nodiscard]] constexpr Bool
        operator==(const FPoint3I& I_Rhs) const noexcept { return X == I_Rhs.X && Y == I_Rhs.Y && Z == I_Rhs.Z; }
        [[nodiscard]] constexpr Bool
        operator!=(const FPoint3I& I_Rhs) const noexcept { return !(*this == I_Rhs); }

        // Point - Point = Vector
        [[nodiscard]] constexpr FVector3I
        operator-(const FPoint3I& I_Rhs) const noexcept { return FVector3I{ X - I_Rhs.X, Y - I_Rhs.Y, Z - I_Rhs.Z }; }
        // Point + Vector = Point
        [[nodiscard]] constexpr FPoint3I
        operator+(const FVector3I& I_Vector) const noexcept { return FPoint3I{ X + I_Vector.X, Y + I_Vector.Y, Z + I_Vector.Z }; }
        // Point - Vector = Point
        [[nodiscard]] constexpr FPoint3I
        operator-(const FVector3I& I_Vector) const noexcept { return FPoint3I{ X - I_Vector.X, Y - I_Vector.Y, Z - I_Vector.Z }; }
        constexpr FPoint3I&
        operator+=(const FVector3I& I_Vector) noexcept { X += I_Vector.X; Y += I_Vector.Y; Z += I_Vector.Z; return *this; }
        constexpr FPoint3I&
        operator-=(const FVector3I& I_Vector) noexcept { X -= I_Vector.X; Y -= I_Vector.Y; Z -= I_Vector.Z; return *this; }

        [[nodiscard]] constexpr FPoint3I
        operator*(Int32 I_Factor) const noexcept { return FPoint3I{ X * I_Factor, Y * I_Factor, Z * I_Factor }; }
        [[nodiscard]] constexpr FPoint3I
        operator/(Int32 I_Divisor) const noexcept { CHECK(I_Divisor != 0); return FPoint3I{ X / I_Divisor, Y / I_Divisor, Z / I_Divisor }; }
        constexpr FPoint3I&
        operator*=(Int32 I_Factor) noexcept { X *= I_Factor; Y *= I_Factor; Z *= I_Factor; return *this; }
        constexpr FPoint3I&
        operator/=(Int32 I_Divisor) noexcept { CHECK(I_Divisor != 0); X /= I_Divisor; Y /= I_Divisor; Z /= I_Divisor; return *this; }

        [[nodiscard]] constexpr Int64
        Dot(const FPoint3I& I_Rhs) const noexcept { return Int64(X) * Int64(I_Rhs.X) + Int64(Y) * Int64(I_Rhs.Y) + Int64(Z) * Int64(I_Rhs.Z); }

        [[nodiscard]] constexpr Int64
        SquaredNorm() const noexcept { return Dot(*this); }

        constexpr FPoint3I() noexcept = default;
        constexpr FPoint3I(Int32 I_X, Int32 I_Y, Int32 I_Z) noexcept : X(I_X), Y(I_Y), Z(I_Z) {}
    };
    static_assert(sizeof(FPoint3I) == 12);
    static_assert(std::is_standard_layout_v<FPoint3I>);

    class VISERA_CORE_API FPoint3U
    {
    public:
        union
        {
            struct { UInt32 X; UInt32 Y; UInt32 Z; };
            UInt32 Data[3]{ 0U, 0U, 0U };
        };

        [[nodiscard]] constexpr UInt32&
        operator[](UInt32 I_Index) noexcept { CHECK(I_Index < 3); return Data[I_Index]; }
        [[nodiscard]] constexpr const UInt32&
        operator[](UInt32 I_Index) const noexcept { CHECK(I_Index < 3); return Data[I_Index]; }

        [[nodiscard]] static constexpr FPoint3U
        Zero() noexcept { return FPoint3U{ 0U, 0U, 0U }; }

        [[nodiscard]] constexpr Bool
        IsZero() const noexcept { return X == 0U && Y == 0U && Z == 0U; }

        [[nodiscard]] constexpr Bool
        operator==(const FPoint3U& I_Rhs) const noexcept { return X == I_Rhs.X && Y == I_Rhs.Y && Z == I_Rhs.Z; }
        [[nodiscard]] constexpr Bool
        operator!=(const FPoint3U& I_Rhs) const noexcept { return !(*this == I_Rhs); }

        // Point - Point = Vector
        [[nodiscard]] constexpr FVector3U
        operator-(const FPoint3U& I_Rhs) const noexcept { return FVector3U{ X - I_Rhs.X, Y - I_Rhs.Y, Z - I_Rhs.Z }; }
        // Point + Vector = Point
        [[nodiscard]] constexpr FPoint3U
        operator+(const FVector3U& I_Vector) const noexcept { return FPoint3U{ X + I_Vector.X, Y + I_Vector.Y, Z + I_Vector.Z }; }
        // Point - Vector = Point
        [[nodiscard]] constexpr FPoint3U
        operator-(const FVector3U& I_Vector) const noexcept { return FPoint3U{ X - I_Vector.X, Y - I_Vector.Y, Z - I_Vector.Z }; }
        constexpr FPoint3U&
        operator+=(const FVector3U& I_Vector) noexcept { X += I_Vector.X; Y += I_Vector.Y; Z += I_Vector.Z; return *this; }
        constexpr FPoint3U&
        operator-=(const FVector3U& I_Vector) noexcept { X -= I_Vector.X; Y -= I_Vector.Y; Z -= I_Vector.Z; return *this; }

        [[nodiscard]] constexpr UInt64
        Dot(const FPoint3U& I_Rhs) const noexcept { return UInt64(X) * UInt64(I_Rhs.X) + UInt64(Y) * UInt64(I_Rhs.Y) + UInt64(Z) * UInt64(I_Rhs.Z); }

        [[nodiscard]] constexpr UInt64
        SquaredNorm() const noexcept { return Dot(*this); }

        [[nodiscard]] constexpr Bool
        IsPositive() const noexcept { return X > 0U && Y > 0U && Z > 0U; }

        constexpr FPoint3U() noexcept = default;
        constexpr FPoint3U(UInt32 I_X, UInt32 I_Y, UInt32 I_Z) noexcept : X(I_X), Y(I_Y), Z(I_Z) {}
    };
    static_assert(sizeof(FPoint3U) == 12);
    static_assert(std::is_standard_layout_v<FPoint3U>);

    namespace Math
    {
        constexpr void
        Clamp(TMutable<FPoint2I> IO_Point, const FPoint2I& I_Min, const FPoint2I& I_Max) noexcept
        {
            Math::Clamp(&IO_Point->X, I_Min.X, I_Max.X);
            Math::Clamp(&IO_Point->Y, I_Min.Y, I_Max.Y);
        }
    }
}
VISERA_MAKE_FORMATTER(Visera::FPoint2I, {}, "({}, {})_Point2I", I_Formatee.X, I_Formatee.Y)
VISERA_MAKE_FORMATTER(Visera::FPoint2U, {}, "({}, {})_Point2U", I_Formatee.X, I_Formatee.Y)
VISERA_MAKE_FORMATTER(Visera::FPoint3I, {}, "({}, {}, {})_Point3I", I_Formatee.X, I_Formatee.Y, I_Formatee.Z)
VISERA_MAKE_FORMATTER(Visera::FPoint3U, {}, "({}, {}, {})_Point3U", I_Formatee.X, I_Formatee.Y, I_Formatee.Z)