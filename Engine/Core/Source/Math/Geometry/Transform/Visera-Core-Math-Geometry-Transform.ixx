module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Geometry.Transform;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic.Operation;
import Visera.Core.Math.Algebra.Vector;
import Visera.Core.Math.Algebra.Matrix;
import Visera.Core.Math.Algebra.Quaternion;
import Visera.Core.Math.Trigonometry;

export namespace Visera
{
    /** Affine transform stored as 3 rows x 4 columns (row-major, 48 bytes).
     *  The implicit 4th row is (0, 0, 0, 1).
     *  Memory layout matches Slang `float3x4` under its default row-major mode,
     *  making it safe for direct GPU upload via StructuredBuffer without padding. */
    class VISERA_CORE_API FTransform3x4F
    {
    public:
        FVector4F Rows[3]
        {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
        };

        [[nodiscard]] static constexpr FTransform3x4F
        Identity() noexcept { return FTransform3x4F{}; }

        /** Extract the top 3 rows from a column-major FMatrix4x4F. */
        [[nodiscard]] static constexpr FTransform3x4F
        FromMatrix4x4(const FMatrix4x4F& I_Matrix) noexcept
        {
            FTransform3x4F T;
            for (UInt32 Row = 0; Row < 3; ++Row)
            {
                T.Rows[Row] = FVector4F{
                    I_Matrix(Row, 0),
                    I_Matrix(Row, 1),
                    I_Matrix(Row, 2),
                    I_Matrix(Row, 3)
                };
            }
            return T;
        }

        /** Reconstruct a full 4x4 column-major matrix by appending implicit row (0,0,0,1). */
        [[nodiscard]] constexpr FMatrix4x4F
        ToMatrix4x4() const noexcept
        {
            return FMatrix4x4F{
                Rows[0].X, Rows[0].Y, Rows[0].Z, Rows[0].W,
                Rows[1].X, Rows[1].Y, Rows[1].Z, Rows[1].W,
                Rows[2].X, Rows[2].Y, Rows[2].Z, Rows[2].W,
                0.0f,      0.0f,      0.0f,      1.0f
            };
        }

        /** Build a 2D affine transform (scale * rotation + translation).
         *  Produces a unit-quad-friendly matrix: shader generates [0,1]^2 vertices,
         *  this matrix scales, rotates, and translates them into world space.
         *  I_Depth is written into Rows[2].W for Z-ordering in the depth buffer. */
        [[nodiscard]] static inline FTransform3x4F
        MakeTransform2D(FVector2F I_Position, FVector2F I_Scale,
                        FDegree I_Rotation, Float I_Depth) noexcept
        {
            const Float C = Math::Cos(I_Rotation);
            const Float S = Math::Sin(I_Rotation);
            FTransform3x4F T;
            T.Rows[0] = { I_Scale.X * C, -I_Scale.Y * S, 0.0f, I_Position.X };
            T.Rows[1] = { I_Scale.X * S,  I_Scale.Y * C, 0.0f, I_Position.Y };
            T.Rows[2] = { 0.0f,           0.0f,          1.0f, I_Depth      };
            return T;
        }

        /** Build a 3D affine transform from TRS decomposition (Translation * Rotation * Scale). */
        [[nodiscard]] static inline FTransform3x4F
        MakeTransform3D(const FVector3F& I_Position,
                        const FQuaternion& I_Rotation,
                        const FVector3F& I_Scale) noexcept
        {
            const FMatrix3x3F R = I_Rotation.ToMatrix3x3();
            FTransform3x4F T;
            T.Rows[0] = { R(0,0) * I_Scale.X, R(0,1) * I_Scale.Y, R(0,2) * I_Scale.Z, I_Position.X };
            T.Rows[1] = { R(1,0) * I_Scale.X, R(1,1) * I_Scale.Y, R(1,2) * I_Scale.Z, I_Position.Y };
            T.Rows[2] = { R(2,0) * I_Scale.X, R(2,1) * I_Scale.Y, R(2,2) * I_Scale.Z, I_Position.Z };
            return T;
        }

        constexpr FTransform3x4F() noexcept = default;
    };
    static_assert(sizeof(FTransform3x4F) == 48);
    static_assert(std::is_standard_layout_v<FTransform3x4F>);
}
VISERA_MAKE_FORMATTER(Visera::FTransform3x4F, {},
            "\n"
            "| {:>10.6f}, {:>10.6f}, {:>10.6f}, {:>10.6f} |\n"
            "| {:>10.6f}, {:>10.6f}, {:>10.6f}, {:>10.6f} |\n"
            "| {:>10.6f}, {:>10.6f}, {:>10.6f}, {:>10.6f} |_Transform3x4F",
            I_Formatee.Rows[0].X, I_Formatee.Rows[0].Y, I_Formatee.Rows[0].Z, I_Formatee.Rows[0].W,
            I_Formatee.Rows[1].X, I_Formatee.Rows[1].Y, I_Formatee.Rows[1].Z, I_Formatee.Rows[1].W,
            I_Formatee.Rows[2].X, I_Formatee.Rows[2].Y, I_Formatee.Rows[2].Z, I_Formatee.Rows[2].W);
