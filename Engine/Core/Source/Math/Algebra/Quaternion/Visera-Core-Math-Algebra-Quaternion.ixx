module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Algebra.Quaternion;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic.Operation;
import Visera.Core.Math.Algebra.Vector;
import Visera.Core.Math.Algebra.Matrix;
import Visera.Core.Math.Trigonometry;

//#define VISERA_SAFE_MODE;
#if defined(VISERA_SAFE_MODE)
#define CHECK(I_Statement) VISERA_ASSERT(I_Statement)
#else
#define CHECK(I_Statement) VISERA_NO_OPERATION
#endif

export namespace Visera
{
    class VISERA_CORE_API FQuaternion
    {
    public:
        union
        {
            struct { Float X, Y, Z, W; };
            Float Data[4] {0.0f, 0.0f, 0.0f, 0.0f};
        };

        [[nodiscard]] static constexpr FQuaternion
        Identity() noexcept { return FQuaternion{0.0f, 0.0f, 0.0f, 1.0f}; }
        [[nodiscard]] static constexpr FQuaternion
        Zero() noexcept { return FQuaternion{0.0f, 0.0f, 0.0f, 0.0f}; }

        [[nodiscard]] constexpr Bool
        IsZero() const noexcept { return X==0.0f && Y==0.0f && Z==0.0f && W==0.0f; }
        [[nodiscard]] constexpr Bool
        IsNearlyZero() const noexcept { return Math::IsNearlyEqual(X,0.0f) && Math::IsNearlyEqual(Y,0.0f) && Math::IsNearlyEqual(Z,0.0f) && Math::IsNearlyEqual(W,0.0f); }
        [[nodiscard]] constexpr Bool
        IsIdentity() const noexcept { return X==0.0f && Y==0.0f && Z==0.0f && W==1.0f; }
        [[nodiscard]] constexpr Bool
        IsNearlyIdentity() const noexcept { return Math::IsNearlyEqual(X,0.0f) && Math::IsNearlyEqual(Y,0.0f) && Math::IsNearlyEqual(Z,0.0f) && Math::IsNearlyEqual(W,1.0f); }

        [[nodiscard]] constexpr Float&
        operator[](UInt32 I_Index) noexcept { CHECK(I_Index < 4); return Data[I_Index]; }
        [[nodiscard]] constexpr const Float&
        operator[](UInt32 I_Index) const noexcept { CHECK(I_Index < 4); return Data[I_Index]; }

        [[nodiscard]] constexpr Float
        Dot(const FQuaternion& I_Rhs) const noexcept { return Math::MulAdd(X, I_Rhs.X, Math::MulAdd(Y, I_Rhs.Y, Math::MulAdd(Z, I_Rhs.Z, W * I_Rhs.W))); }
        [[nodiscard]] constexpr Float
        SquaredNorm() const noexcept { return Dot(*this); }
        [[nodiscard]] inline Float
        L2Norm() const noexcept { return Math::Sqrt(SquaredNorm()); }
        [[nodiscard]] inline Bool
        Normalize() noexcept
        {
            Float SqrNorm = SquaredNorm();
            if (Math::IsNearlyEqual(SqrNorm, 0.0f)) { return False; }
            Float InvNorm = 1.0f / Math::Sqrt(SqrNorm);
            X *= InvNorm; Y *= InvNorm; Z *= InvNorm; W *= InvNorm;
            return True;
        }
        [[nodiscard]] inline Bool
        IsNearlyUnit() const noexcept { return Math::IsNearlyEqual(SquaredNorm(), 1.0f); }

        [[nodiscard]] constexpr FQuaternion
        Conjugate() const noexcept { return FQuaternion{-X, -Y, -Z, W}; }
        [[nodiscard]] constexpr FQuaternion
        operator-() const noexcept { return FQuaternion{-X, -Y, -Z, -W}; }

        [[nodiscard]] constexpr FQuaternion
        operator+(const FQuaternion& I_Rhs) const noexcept { return { X + I_Rhs.X, Y + I_Rhs.Y, Z + I_Rhs.Z, W + I_Rhs.W }; }
        [[nodiscard]] constexpr FQuaternion
        operator-(const FQuaternion& I_Rhs) const noexcept { return { X - I_Rhs.X, Y - I_Rhs.Y, Z - I_Rhs.Z, W - I_Rhs.W }; }
        [[nodiscard]] constexpr FQuaternion
        operator*(Float I_Factor) const noexcept { return { X * I_Factor, Y * I_Factor, Z * I_Factor, W * I_Factor }; }
        [[nodiscard]] constexpr FQuaternion
        operator/(Float I_Factor) const noexcept { CHECK(!Math::IsNearlyEqual(I_Factor, 0.0f)); return (*this) * (1.0f / I_Factor); }

        constexpr FQuaternion&
        operator+=(const FQuaternion& I_Rhs) noexcept { X += I_Rhs.X; Y += I_Rhs.Y; Z += I_Rhs.Z; W += I_Rhs.W; return *this; }
        constexpr FQuaternion&
        operator-=(const FQuaternion& I_Rhs) noexcept { X -= I_Rhs.X; Y -= I_Rhs.Y; Z -= I_Rhs.Z; W -= I_Rhs.W; return *this; }
        constexpr FQuaternion&
        operator*=(Float I_Factor) noexcept { X *= I_Factor; Y *= I_Factor; Z *= I_Factor; W *= I_Factor; return *this; }
        constexpr FQuaternion&
        operator/=(Float I_Factor) noexcept { CHECK(!Math::IsNearlyEqual(I_Factor, 0.0f)); return (*this) *= (1.0f / I_Factor); }

        [[nodiscard]] constexpr FQuaternion
        operator*(const FQuaternion& I_Rhs) const noexcept
        {
            auto& Q = *this;
            const Float X2 = I_Rhs.X, Y2 = I_Rhs.Y, Z2 = I_Rhs.Z, W2 = I_Rhs.W;
            return FQuaternion{
                Q.W*X2 + Q.X*W2 + Q.Y*Z2 - Q.Z*Y2,
                Q.W*Y2 - Q.X*Z2 + Q.Y*W2 + Q.Z*X2,
                Q.W*Z2 + Q.X*Y2 - Q.Y*X2 + Q.Z*W2,
                Q.W*W2 - Q.X*X2 - Q.Y*Y2 - Q.Z*Z2
            };
        }
        constexpr FQuaternion&
        operator*=(const FQuaternion& I_Rhs) noexcept { *this = (*this) * I_Rhs; return *this; }

        [[nodiscard]] constexpr FVector3F
        RotateVector(const FVector3F& I_Vector) const noexcept
        {
            const FVector3F Qv{ X, Y, Z };

            const FVector3F T{
                2.0f * (Math::MulAdd(Qv.Y, I_Vector.Z, - Qv.Z * I_Vector.Y)),
                2.0f * (Math::MulAdd(Qv.Z, I_Vector.X, - Qv.X * I_Vector.Z)),
                2.0f * (Math::MulAdd(Qv.X, I_Vector.Y, - Qv.Y * I_Vector.X))
            };

            const FVector3F QvXT{
                Math::MulAdd(Qv.Y, T.Z, - Qv.Z * T.Y),
                Math::MulAdd(Qv.Z, T.X, - Qv.X * T.Z),
                Math::MulAdd(Qv.X, T.Y, - Qv.Y * T.X)
            };

            return {
                I_Vector.X + Math::MulAdd(W, T.X, QvXT.X),
                I_Vector.Y + Math::MulAdd(W, T.Y, QvXT.Y),
                I_Vector.Z + Math::MulAdd(W, T.Z, QvXT.Z)
            };
        }

        [[nodiscard]] static inline FQuaternion
        FromAxisAngle(const FVector3F& I_Axis, FRadian I_Radian) noexcept
        {
            FVector3F Axis = I_Axis;
            if (!Axis.Normalize()) { return Identity(); }

            const FRadian Half = I_Radian * 0.5f;
            const Float S = Math::Sin(Half);
            const Float C = Math::Cos(Half);

            return FQuaternion{ Axis.X * S, Axis.Y * S, Axis.Z * S, C };
        }

        [[nodiscard]] static inline FQuaternion
        FromAxisAngle(const FVector3F& I_Axis, FDegree I_Degree) noexcept { return FromAxisAngle(I_Axis, Math::Radian(I_Degree)); }

        [[nodiscard]] constexpr FMatrix3x3F
        ToMatrix3x3() const noexcept
        {
            auto& Q = *this;

            const Float XX = Q.X * Q.X; const Float YY = Q.Y * Q.Y; const Float ZZ = Q.Z * Q.Z;
            const Float XY = Q.X * Q.Y; const Float XZ = Q.X * Q.Z; const Float YZ = Q.Y * Q.Z;
            const Float WX = Q.W * Q.X; const Float WY = Q.W * Q.Y; const Float WZ = Q.W * Q.Z;

            return FMatrix3x3F{
                1.0f - 2.0f*(YY + ZZ),  2.0f*(XY - WZ),        2.0f*(XZ + WY),
                2.0f*(XY + WZ),         1.0f - 2.0f*(XX + ZZ), 2.0f*(YZ - WX),
                2.0f*(XZ - WY),         2.0f*(YZ + WX),        1.0f - 2.0f*(XX + YY)
            };
        }

        [[nodiscard]] constexpr FMatrix4x4F
        ToMatrix4x4() const noexcept
        {
            FMatrix3x3F R = ToMatrix3x3();
            return FMatrix4x4F{
                R(0,0), R(0,1), R(0,2), 0.0f,
                R(1,0), R(1,1), R(1,2), 0.0f,
                R(2,0), R(2,1), R(2,2), 0.0f,
                0.0f,   0.0f,   0.0f,   1.0f
            };
        }

        constexpr FQuaternion() noexcept = default;
        constexpr FQuaternion(Float I_X, Float I_Y, Float I_Z, Float I_W) noexcept : X(I_X), Y(I_Y), Z(I_Z), W(I_W) {}
    };
    static_assert(sizeof(FQuaternion) == 16);
    static_assert(std::is_standard_layout_v<FQuaternion>);

    namespace Math
    {
        [[nodiscard]] inline Bool
        Inverse(TMutable<FQuaternion> IO_Quaternion) noexcept
        {
            VISERA_ASSERT(IO_Quaternion != nullptr);
            Float SqrNorm = IO_Quaternion->SquaredNorm();
            if (IsNearlyEqual(SqrNorm, 0.0f)) { return False; }
            Float InvSqrNorm = 1.0f / SqrNorm;
            *IO_Quaternion = IO_Quaternion->Conjugate() * InvSqrNorm;
            return True;
        }
    }
}

VISERA_MAKE_FORMATTER(Visera::FQuaternion, {},
            "\n"
            "|   {:<10.6f} |\n"
            "|   {:<10.6f} |\n"
            "|   {:<10.6f} |\n"
            "|   {:<10.6f} |_Quaternion",
            I_Formatee.X, I_Formatee.Y, I_Formatee.Z, I_Formatee.W)
