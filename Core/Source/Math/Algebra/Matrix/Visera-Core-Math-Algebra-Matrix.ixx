module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Algebra.Matrix;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic;
import Visera.Core.Math.Algebra.Vector;

//#define VISERA_SAFE_MODE;
#if defined(VISERA_SAFE_MODE)
#define CHECK(I_Statement) VISERA_ASSERT(I_Statement)
#else
#define CHECK(I_Statement) VISERA_NO_OPERATION
#endif

export namespace Visera
{
	class VISERA_CORE_API FMatrix2x2F // (column-major)
    {
    public:
        Float Data[4]{ 0.0f, 0.0f,
                       0.0f, 0.0f };

        [[nodiscard]] constexpr Float&
        operator[](UInt32 I_Index) noexcept { CHECK(I_Index < 4); return Data[I_Index]; }
        [[nodiscard]] constexpr const Float&
        operator[](UInt32 I_Index) const noexcept { CHECK(I_Index < 4); return Data[I_Index]; }

        [[nodiscard]] constexpr Float&
        operator()(UInt32 I_Row, UInt32 I_Col) noexcept { CHECK(I_Row < 2 && I_Col < 2); return Data[I_Col * 2 + I_Row]; }
        [[nodiscard]] constexpr const Float&
	    operator()(UInt32 I_Row, UInt32 I_Col) const noexcept { CHECK(I_Row < 2 && I_Col < 2); return Data[I_Col * 2 + I_Row]; }

        [[nodiscard]] static constexpr FMatrix2x2F
	    Identity() noexcept { return FMatrix2x2F{1.0f, 0.0f, 0.0f, 1.0f}; }
        [[nodiscard]] static constexpr FMatrix2x2F
	    Zero() noexcept { return FMatrix2x2F{0.0f, 0.0f, 0.0f, 0.0f}; }

        [[nodiscard]] constexpr Bool
	    IsZero() const noexcept { return Data[0]==0.0f && Data[1]==0.0f && Data[2]==0.0f && Data[3]==0.0f; }
        [[nodiscard]] constexpr Bool
	    IsNearlyZero() const noexcept { return Math::IsNearlyEqual(Data[0],0.0f) && Math::IsNearlyEqual(Data[1],0.0f) && Math::IsNearlyEqual(Data[2],0.0f) && Math::IsNearlyEqual(Data[3],0.0f); }
        [[nodiscard]] constexpr Bool
	    IsIdentity() const noexcept { return (*this)(0,0)==1.0f && (*this)(0,1)==0.0f && (*this)(1,0)==0.0f && (*this)(1,1)==1.0f; }
        [[nodiscard]] constexpr Bool
	    IsNearlyIdentity() const noexcept { return Math::IsNearlyEqual((*this)(0,0),1.0f) && Math::IsNearlyEqual((*this)(0,1),0.0f) && Math::IsNearlyEqual((*this)(1,0),0.0f) && Math::IsNearlyEqual((*this)(1,1),1.0f); }

        [[nodiscard]] constexpr Float
	    Determinant() const noexcept
        {
            auto& M = *this;
            return M(0,0) * M(1,1) - M(0,1) * M(1,0);
        }
        [[nodiscard]] constexpr FMatrix2x2F
	    Transposed() const noexcept
        {
            return FMatrix2x2F{
                (*this)(0,0), (*this)(0,1),
                (*this)(1,0), (*this)(1,1),
                True
            }.TransposeInPlace();
        }

        [[nodiscard]] constexpr FMatrix2x2F
	    operator+(const FMatrix2x2F& I_Rhs) const noexcept { FMatrix2x2F R = Zero(); for (UInt32 Idx = 0; Idx < 4; ++Idx) R.Data[Idx] = Data[Idx] + I_Rhs.Data[Idx]; return R; }
        [[nodiscard]] constexpr FMatrix2x2F
	    operator-(const FMatrix2x2F& I_Rhs) const noexcept { FMatrix2x2F R = Zero(); for (UInt32 Idx = 0; Idx < 4; ++Idx) R.Data[Idx] = Data[Idx] - I_Rhs.Data[Idx]; return R; }
        [[nodiscard]] constexpr FMatrix2x2F
	    operator*(Float I_Factor) const noexcept { FMatrix2x2F R = Zero(); for (UInt32 Idx = 0; Idx < 4; ++Idx) R.Data[Idx] = Data[Idx] * I_Factor; return R; }
        [[nodiscard]] constexpr FMatrix2x2F
	    operator/(Float I_Factor) const noexcept { CHECK(!Math::IsNearlyEqual(I_Factor, 0.0f)); return (*this) * (1.0f / I_Factor); }
        constexpr FMatrix2x2F&
        operator+=(const FMatrix2x2F& I_Rhs) noexcept { for (UInt32 Idx = 0; Idx < 4; ++Idx) Data[Idx] += I_Rhs.Data[Idx]; return *this; }
        constexpr FMatrix2x2F&
        operator-=(const FMatrix2x2F& I_Rhs) noexcept { for (UInt32 Idx = 0; Idx < 4; ++Idx) Data[Idx] -= I_Rhs.Data[Idx]; return *this; }
        constexpr FMatrix2x2F&
        operator*=(Float I_Factor) noexcept { for (UInt32 Idx = 0; Idx < 4; ++Idx) Data[Idx] *= I_Factor; return *this; }
        constexpr FMatrix2x2F&
        operator/=(Float I_Factor) noexcept { CHECK(!Math::IsNearlyEqual(I_Factor, 0.0f)); return (*this) *= (1.0f / I_Factor); }
        [[nodiscard]] constexpr FMatrix2x2F
	    operator*(const FMatrix2x2F& I_Rhs) const noexcept
        {
            auto R = FMatrix2x2F::Zero();
            for (UInt32 Row = 0; Row < 2; ++Row)
            {
                for (UInt32 Col = 0; Col < 2; ++Col)
                {
                    Float Sum = 0.0f;
                    for (UInt32 K = 0; K < 2; ++K) { Sum += (*this)(Row, K) * I_Rhs(K, Col); }
                    R(Row, Col) = Sum;
                }
            }
            return R;
        }
        constexpr FMatrix2x2F&
        operator*=(const FMatrix2x2F& I_Rhs) noexcept { *this = (*this) * I_Rhs; return *this; }
        [[nodiscard]] constexpr FVector2F
	    operator*(const FVector2F& I_Vector) const noexcept
        {
        	auto& M = *this;
            return {
                M(0,0)*I_Vector.X + M(0,1)*I_Vector.Y,
                M(1,0)*I_Vector.X + M(1,1)*I_Vector.Y};
        }
        constexpr FMatrix2x2F() noexcept = default;
        constexpr FMatrix2x2F(Float I_M00, Float I_M01, Float I_M10, Float I_M11) noexcept
		: Data { I_M00, I_M01, I_M10, I_M11} {}

    private:
        // Internal helper: not exported as public API necessity, just to support Transposed() compactly.
        constexpr FMatrix2x2F(Float I_M00, Float I_M01, Float I_M10, Float I_M11, Bool I_ColumnMajorAlreadyHandled) noexcept
        : Data{ I_M00, I_M10, I_M01, I_M11 } {}

        constexpr FMatrix2x2F& TransposeInPlace() noexcept
        {
        	auto& M = *this;
            const Float T01 = M(0,1);
            M(0,1) = M(1,0);
            M(1,0) = T01;
            return M;
        }
    };
    static_assert(sizeof(FMatrix2x2F) == 16);
    static_assert(std::is_standard_layout_v<FMatrix2x2F>);

	class VISERA_CORE_API FMatrix3x3F // (column-major)
    {
    public:
        Float Data[9]{ 0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f };

        [[nodiscard]] constexpr Float&
        operator[](UInt32 I_Index) noexcept { CHECK(I_Index < 9); return Data[I_Index]; }
        [[nodiscard]] constexpr const Float&
        operator[](UInt32 I_Index) const noexcept { CHECK(I_Index < 9); return Data[I_Index]; }

        [[nodiscard]] constexpr Float&
        operator()(UInt32 I_Row, UInt32 I_Col) noexcept { CHECK(I_Row < 3 && I_Col < 3); return Data[I_Col * 3 + I_Row]; }
        [[nodiscard]] constexpr const Float&
        operator()(UInt32 I_Row, UInt32 I_Col) const noexcept { CHECK(I_Row < 3 && I_Col < 3); return Data[I_Col * 3 + I_Row]; }

        [[nodiscard]] static constexpr FMatrix3x3F
        Identity() noexcept { return FMatrix3x3F{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}; }
        [[nodiscard]] static constexpr FMatrix3x3F
        Zero() noexcept { return FMatrix3x3F{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; }

        [[nodiscard]] constexpr Bool
        IsZero() const noexcept { return Data[0]==0.0f && Data[1]==0.0f && Data[2]==0.0f && Data[3]==0.0f && Data[4]==0.0f && Data[5]==0.0f && Data[6]==0.0f && Data[7]==0.0f && Data[8]==0.0f; }
        [[nodiscard]] constexpr Bool
        IsNearlyZero() const noexcept { return Math::IsNearlyEqual(Data[0],0.0f) && Math::IsNearlyEqual(Data[1],0.0f) && Math::IsNearlyEqual(Data[2],0.0f) && Math::IsNearlyEqual(Data[3],0.0f) && Math::IsNearlyEqual(Data[4],0.0f) && Math::IsNearlyEqual(Data[5],0.0f) && Math::IsNearlyEqual(Data[6],0.0f) && Math::IsNearlyEqual(Data[7],0.0f) && Math::IsNearlyEqual(Data[8],0.0f); }
        [[nodiscard]] constexpr Bool
        IsIdentity() const noexcept { return (*this)(0,0)==1.0f && (*this)(0,1)==0.0f && (*this)(0,2)==0.0f && (*this)(1,0)==0.0f && (*this)(1,1)==1.0f && (*this)(1,2)==0.0f && (*this)(2,0)==0.0f && (*this)(2,1)==0.0f && (*this)(2,2)==1.0f; }
        [[nodiscard]] constexpr Bool
        IsNearlyIdentity() const noexcept { return Math::IsNearlyEqual((*this)(0,0),1.0f) && Math::IsNearlyEqual((*this)(0,1),0.0f) && Math::IsNearlyEqual((*this)(0,2),0.0f) && Math::IsNearlyEqual((*this)(1,0),0.0f) && Math::IsNearlyEqual((*this)(1,1),1.0f) && Math::IsNearlyEqual((*this)(1,2),0.0f) && Math::IsNearlyEqual((*this)(2,0),0.0f) && Math::IsNearlyEqual((*this)(2,1),0.0f) && Math::IsNearlyEqual((*this)(2,2),1.0f); }

        [[nodiscard]] constexpr Float
        Determinant() const noexcept
        {
            auto& M = *this;
            const Float A00 = M(0,0), A01 = M(0,1), A02 = M(0,2);
            const Float A10 = M(1,0), A11 = M(1,1), A12 = M(1,2);
            const Float A20 = M(2,0), A21 = M(2,1), A22 = M(2,2);
            return A00*(A11*A22 - A12*A21) - A01*(A10*A22 - A12*A20) + A02*(A10*A21 - A11*A20);
        }
        [[nodiscard]] constexpr FMatrix3x3F
        Transposed() const noexcept
        {
            auto R = FMatrix3x3F::Zero();
            for (UInt32 Row = 0; Row < 3; ++Row)
            {
                for (UInt32 Col = 0; Col < 3; ++Col) { R(Row, Col) = (*this)(Col, Row); }
            }
            return R;
        }

        [[nodiscard]] constexpr FMatrix3x3F
        operator+(const FMatrix3x3F& I_Rhs) const noexcept { FMatrix3x3F R = Zero(); for (UInt32 Idx = 0; Idx < 9; ++Idx) R.Data[Idx] = Data[Idx] + I_Rhs.Data[Idx]; return R; }
        [[nodiscard]] constexpr FMatrix3x3F
        operator-(const FMatrix3x3F& I_Rhs) const noexcept { FMatrix3x3F R = Zero(); for (UInt32 Idx = 0; Idx < 9; ++Idx) R.Data[Idx] = Data[Idx] - I_Rhs.Data[Idx]; return R; }
        [[nodiscard]] constexpr FMatrix3x3F
        operator*(Float I_Factor) const noexcept { FMatrix3x3F R = Zero(); for (UInt32 Idx = 0; Idx < 9; ++Idx) R.Data[Idx] = Data[Idx] * I_Factor; return R; }
        [[nodiscard]] constexpr FMatrix3x3F
        operator/(Float I_Factor) const noexcept { CHECK(!Math::IsNearlyEqual(I_Factor, 0.0f)); return (*this) * (1.0f / I_Factor); }
        constexpr FMatrix3x3F&
        operator+=(const FMatrix3x3F& I_Rhs) noexcept { for (UInt32 Idx = 0; Idx < 9; ++Idx) Data[Idx] += I_Rhs.Data[Idx]; return *this; }
        constexpr FMatrix3x3F&
        operator-=(const FMatrix3x3F& I_Rhs) noexcept { for (UInt32 Idx = 0; Idx < 9; ++Idx) Data[Idx] -= I_Rhs.Data[Idx]; return *this; }
        constexpr FMatrix3x3F&
        operator*=(Float I_Factor) noexcept { for (UInt32 Idx = 0; Idx < 9; ++Idx) Data[Idx] *= I_Factor; return *this; }
        constexpr FMatrix3x3F&
        operator/=(Float I_Factor) noexcept { CHECK(!Math::IsNearlyEqual(I_Factor, 0.0f)); return (*this) *= (1.0f / I_Factor); }
        [[nodiscard]] constexpr FMatrix3x3F
        operator*(const FMatrix3x3F& I_Rhs) const noexcept
        {
            auto R = FMatrix3x3F::Zero();
            for (UInt32 Row = 0; Row < 3; ++Row)
            {
                for (UInt32 Col = 0; Col < 3; ++Col)
                {
                    Float Sum = 0.0f;
                    for (UInt32 K = 0; K < 3; ++K) { Sum += (*this)(Row, K) * I_Rhs(K, Col); }
                    R(Row, Col) = Sum;
                }
            }
            return R;
        }
        constexpr FMatrix3x3F&
        operator*=(const FMatrix3x3F& I_Rhs) noexcept { *this = (*this) * I_Rhs; return *this; }
        [[nodiscard]] constexpr FVector3F
        operator*(const FVector3F& I_Vector) const noexcept
        {
            auto& M = *this;
            return {
                M(0,0)*I_Vector.X + M(0,1)*I_Vector.Y + M(0,2)*I_Vector.Z,
                M(1,0)*I_Vector.X + M(1,1)*I_Vector.Y + M(1,2)*I_Vector.Z,
                M(2,0)*I_Vector.X + M(2,1)*I_Vector.Y + M(2,2)*I_Vector.Z };
        }

        constexpr FMatrix3x3F() noexcept = default;
        constexpr FMatrix3x3F(Float I_M00, Float I_M01, Float I_M02,
                              Float I_M10, Float I_M11, Float I_M12,
                              Float I_M20, Float I_M21, Float I_M22) noexcept
        : Data { I_M00, I_M10, I_M20,
                 I_M01, I_M11, I_M21,
                 I_M02, I_M12, I_M22 } {}
    };
    static_assert(sizeof(FMatrix3x3F) == 36);
    static_assert(std::is_standard_layout_v<FMatrix3x3F>);

    class VISERA_CORE_API FMatrix4x4F // (column-major)
    {
    public:
        Float Data[16]{ 0.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f };

        [[nodiscard]] constexpr Float&
        operator[](UInt32 I_Index) noexcept { CHECK(I_Index < 16); return Data[I_Index]; }
        [[nodiscard]] constexpr const Float&
        operator[](UInt32 I_Index) const noexcept { CHECK(I_Index < 16); return Data[I_Index]; }

        [[nodiscard]] constexpr Float&
        operator()(UInt32 I_Row, UInt32 I_Col) noexcept { CHECK(I_Row < 4 && I_Col < 4); return Data[I_Col * 4 + I_Row]; }
        [[nodiscard]] constexpr const Float&
        operator()(UInt32 I_Row, UInt32 I_Col) const noexcept { CHECK(I_Row < 4 && I_Col < 4); return Data[I_Col * 4 + I_Row]; }

        [[nodiscard]] static constexpr FMatrix4x4F
        Identity() noexcept { return FMatrix4x4F{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}; }
        [[nodiscard]] static constexpr FMatrix4x4F
        Zero() noexcept { return FMatrix4x4F{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; }

        [[nodiscard]] constexpr Bool
        IsZero() const noexcept { return Data[0]==0.0f && Data[1]==0.0f && Data[2]==0.0f && Data[3]==0.0f && Data[4]==0.0f && Data[5]==0.0f && Data[6]==0.0f && Data[7]==0.0f && Data[8]==0.0f && Data[9]==0.0f && Data[10]==0.0f && Data[11]==0.0f && Data[12]==0.0f && Data[13]==0.0f && Data[14]==0.0f && Data[15]==0.0f; }
        [[nodiscard]] constexpr Bool
        IsNearlyZero() const noexcept { return Math::IsNearlyEqual(Data[0],0.0f) && Math::IsNearlyEqual(Data[1],0.0f) && Math::IsNearlyEqual(Data[2],0.0f) && Math::IsNearlyEqual(Data[3],0.0f) && Math::IsNearlyEqual(Data[4],0.0f) && Math::IsNearlyEqual(Data[5],0.0f) && Math::IsNearlyEqual(Data[6],0.0f) && Math::IsNearlyEqual(Data[7],0.0f) && Math::IsNearlyEqual(Data[8],0.0f) && Math::IsNearlyEqual(Data[9],0.0f) && Math::IsNearlyEqual(Data[10],0.0f) && Math::IsNearlyEqual(Data[11],0.0f) && Math::IsNearlyEqual(Data[12],0.0f) && Math::IsNearlyEqual(Data[13],0.0f) && Math::IsNearlyEqual(Data[14],0.0f) && Math::IsNearlyEqual(Data[15],0.0f); }
        [[nodiscard]] constexpr Bool
        IsIdentity() const noexcept { return (*this)(0,0)==1.0f && (*this)(0,1)==0.0f && (*this)(0,2)==0.0f && (*this)(0,3)==0.0f && (*this)(1,0)==0.0f && (*this)(1,1)==1.0f && (*this)(1,2)==0.0f && (*this)(1,3)==0.0f && (*this)(2,0)==0.0f && (*this)(2,1)==0.0f && (*this)(2,2)==1.0f && (*this)(2,3)==0.0f && (*this)(3,0)==0.0f && (*this)(3,1)==0.0f && (*this)(3,2)==0.0f && (*this)(3,3)==1.0f; }
        [[nodiscard]] constexpr Bool
        IsNearlyIdentity() const noexcept { return Math::IsNearlyEqual((*this)(0,0),1.0f) && Math::IsNearlyEqual((*this)(0,1),0.0f) && Math::IsNearlyEqual((*this)(0,2),0.0f) && Math::IsNearlyEqual((*this)(0,3),0.0f) && Math::IsNearlyEqual((*this)(1,0),0.0f) && Math::IsNearlyEqual((*this)(1,1),1.0f) && Math::IsNearlyEqual((*this)(1,2),0.0f) && Math::IsNearlyEqual((*this)(1,3),0.0f) && Math::IsNearlyEqual((*this)(2,0),0.0f) && Math::IsNearlyEqual((*this)(2,1),0.0f) && Math::IsNearlyEqual((*this)(2,2),1.0f) && Math::IsNearlyEqual((*this)(2,3),0.0f) && Math::IsNearlyEqual((*this)(3,0),0.0f) && Math::IsNearlyEqual((*this)(3,1),0.0f) && Math::IsNearlyEqual((*this)(3,2),0.0f) && Math::IsNearlyEqual((*this)(3,3),1.0f); }

        [[nodiscard]] constexpr Float
        Determinant() const noexcept
        {
            auto& M = *this;
            const Float A00 = M(0,0), A01 = M(0,1), A02 = M(0,2), A03 = M(0,3);
            const Float A10 = M(1,0), A11 = M(1,1), A12 = M(1,2), A13 = M(1,3);
            const Float A20 = M(2,0), A21 = M(2,1), A22 = M(2,2), A23 = M(2,3);
            const Float A30 = M(3,0), A31 = M(3,1), A32 = M(3,2), A33 = M(3,3);

            const Float Det0 = A11*(A22*A33 - A23*A32) - A12*(A21*A33 - A23*A31) + A13*(A21*A32 - A22*A31);
            const Float Det1 = A10*(A22*A33 - A23*A32) - A12*(A20*A33 - A23*A30) + A13*(A20*A32 - A22*A30);
            const Float Det2 = A10*(A21*A33 - A23*A31) - A11*(A20*A33 - A23*A30) + A13*(A20*A31 - A21*A30);
            const Float Det3 = A10*(A21*A32 - A22*A31) - A11*(A20*A32 - A22*A30) + A12*(A20*A31 - A21*A30);

            return A00 * Det0 - A01 * Det1 + A02 * Det2 - A03 * Det3;
        }
        [[nodiscard]] constexpr FMatrix4x4F
        Transposed() const noexcept
        {
            auto R = FMatrix4x4F::Zero();
            for (UInt32 Row = 0; Row < 4; ++Row)
            {
                for (UInt32 Col = 0; Col < 4; ++Col) { R(Row, Col) = (*this)(Col, Row); }
            }
            return R;
        }

        [[nodiscard]] constexpr FMatrix4x4F
        operator+(const FMatrix4x4F& I_Rhs) const noexcept { FMatrix4x4F R = Zero(); for (UInt32 Idx = 0; Idx < 16; ++Idx) R.Data[Idx] = Data[Idx] + I_Rhs.Data[Idx]; return R; }
        [[nodiscard]] constexpr FMatrix4x4F
        operator-(const FMatrix4x4F& I_Rhs) const noexcept { FMatrix4x4F R = Zero(); for (UInt32 Idx = 0; Idx < 16; ++Idx) R.Data[Idx] = Data[Idx] - I_Rhs.Data[Idx]; return R; }
        [[nodiscard]] constexpr FMatrix4x4F
        operator*(Float I_Factor) const noexcept { FMatrix4x4F R = Zero(); for (UInt32 Idx = 0; Idx < 16; ++Idx) R.Data[Idx] = Data[Idx] * I_Factor; return R; }
        [[nodiscard]] constexpr FMatrix4x4F
        operator/(Float I_Factor) const noexcept { CHECK(!Math::IsNearlyEqual(I_Factor, 0.0f)); return (*this) * (1.0f / I_Factor); }
        constexpr FMatrix4x4F&
        operator+=(const FMatrix4x4F& I_Rhs) noexcept { for (UInt32 Idx = 0; Idx < 16; ++Idx) Data[Idx] += I_Rhs.Data[Idx]; return *this; }
        constexpr FMatrix4x4F&
        operator-=(const FMatrix4x4F& I_Rhs) noexcept { for (UInt32 Idx = 0; Idx < 16; ++Idx) Data[Idx] -= I_Rhs.Data[Idx]; return *this; }
        constexpr FMatrix4x4F&
        operator*=(Float I_Factor) noexcept { for (UInt32 Idx = 0; Idx < 16; ++Idx) Data[Idx] *= I_Factor; return *this; }
        constexpr FMatrix4x4F&
        operator/=(Float I_Factor) noexcept { CHECK(!Math::IsNearlyEqual(I_Factor, 0.0f)); return (*this) *= (1.0f / I_Factor); }
        [[nodiscard]] constexpr FMatrix4x4F
        operator*(const FMatrix4x4F& I_Rhs) const noexcept
        {
            auto R = FMatrix4x4F::Zero();
            for (UInt32 Row = 0; Row < 4; ++Row)
            {
                for (UInt32 Col = 0; Col < 4; ++Col)
                {
                    Float Sum = 0.0f;
                    for (UInt32 K = 0; K < 4; ++K) { Sum += (*this)(Row, K) * I_Rhs(K, Col); }
                    R(Row, Col) = Sum;
                }
            }
            return R;
        }
        constexpr FMatrix4x4F&
        operator*=(const FMatrix4x4F& I_Rhs) noexcept { *this = (*this) * I_Rhs; return *this; }
        [[nodiscard]] constexpr FVector4F
        operator*(const FVector4F& I_Vector) const noexcept
        {
            auto& M = *this;
            return {
                M(0,0)*I_Vector.X + M(0,1)*I_Vector.Y + M(0,2)*I_Vector.Z + M(0,3)*I_Vector.W,
                M(1,0)*I_Vector.X + M(1,1)*I_Vector.Y + M(1,2)*I_Vector.Z + M(1,3)*I_Vector.W,
                M(2,0)*I_Vector.X + M(2,1)*I_Vector.Y + M(2,2)*I_Vector.Z + M(2,3)*I_Vector.W,
                M(3,0)*I_Vector.X + M(3,1)*I_Vector.Y + M(3,2)*I_Vector.Z + M(3,3)*I_Vector.W };
        }

        constexpr FMatrix4x4F() noexcept = default;
        constexpr FMatrix4x4F(Float I_M00, Float I_M01, Float I_M02, Float I_M03,
                              Float I_M10, Float I_M11, Float I_M12, Float I_M13,
                              Float I_M20, Float I_M21, Float I_M22, Float I_M23,
                              Float I_M30, Float I_M31, Float I_M32, Float I_M33) noexcept
        : Data { I_M00, I_M10, I_M20, I_M30,
                 I_M01, I_M11, I_M21, I_M31,
                 I_M02, I_M12, I_M22, I_M32,
                 I_M03, I_M13, I_M23, I_M33 } {}
    };
    static_assert(sizeof(FMatrix4x4F) == 64);
    static_assert(std::is_standard_layout_v<FMatrix4x4F>);

	namespace Concepts
	{
		template<typename T> concept
		Matrical = std::is_class_v<FMatrix2x2F> ||
				   std::is_class_v<FMatrix3x3F> ||
				   std::is_class_v<FMatrix4x4F>;
	}
}
VISERA_MAKE_FORMATTER(Visera::FMatrix2x2F, {},
            "\n"
			"| {:>10.6f}, {:>10.6f} |\n"
			"| {:>10.6f}, {:>10.6f} |_Matrix2x2F",
			I_Formatee(0, 0), I_Formatee(0, 1),
			I_Formatee(1, 0), I_Formatee(1, 1));
VISERA_MAKE_FORMATTER(Visera::FMatrix3x3F, {},
            "\n"
            "| {:>10.6f}, {:>10.6f}, {:>10.6f} |\n"
            "| {:>10.6f}, {:>10.6f}, {:>10.6f} |\n"
            "| {:>10.6f}, {:>10.6f}, {:>10.6f} |_Matrix3x3F",
            I_Formatee(0, 0), I_Formatee(0, 1), I_Formatee(0, 2),
            I_Formatee(1, 0), I_Formatee(1, 1), I_Formatee(1, 2),
            I_Formatee(2, 0), I_Formatee(2, 1), I_Formatee(2, 2));
VISERA_MAKE_FORMATTER(Visera::FMatrix4x4F, {},
            "\n"
            "| {:>10.6f}, {:>10.6f}, {:>10.6f}, {:>10.6f} |\n"
            "| {:>10.6f}, {:>10.6f}, {:>10.6f}, {:>10.6f} |\n"
            "| {:>10.6f}, {:>10.6f}, {:>10.6f}, {:>10.6f} |\n"
            "| {:>10.6f}, {:>10.6f}, {:>10.6f}, {:>10.6f} |_Matrix4x4F",
            I_Formatee(0, 0), I_Formatee(0, 1), I_Formatee(0, 2), I_Formatee(0, 3),
            I_Formatee(1, 0), I_Formatee(1, 1), I_Formatee(1, 2), I_Formatee(1, 3),
            I_Formatee(2, 0), I_Formatee(2, 1), I_Formatee(2, 2), I_Formatee(2, 3),
            I_Formatee(3, 0), I_Formatee(3, 1), I_Formatee(3, 2), I_Formatee(3, 3));