module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Scene.Camera;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Core.Math.Algebra;
export import Visera.Core.Math.Trigonometry;

/** Scene camera (left-handed). Provides view/projection and View/Projection matrices for viewport rendering. */
export namespace Visera
{
    /** Left-handed scene camera. Supports perspective and orthographic projection; View, Projection, and ViewProjection matrices are lazily updated via dirty flags when position/rotation or projection params change. */
    class VISERA_RUNTIME_API FCamera
    {
    public:
        /** Projection type. */
        enum class EProjectionType : UInt8
        {
            Perspective,   /**< Perspective (FOV, aspect, near/far). */
            Orthographic,  /**< Orthographic (width, height, near/far). */

            Default = Perspective,  /**< Default is perspective. */
        };

        /** Creates a perspective camera. I_FOVY: vertical FOV; I_AspectRatio: width/height; I_NearPlane/I_FarPlane: clip distances. Returns new instance. */
        [[nodiscard]] static FCamera
        MakePerspective(FDegree I_FOVY,
                        Float   I_AspectRatio,
                        Float   I_NearPlane,
                        Float   I_FarPlane) noexcept;
        /** Creates an orthographic camera. I_Width/I_Height: viewport size; I_NearPlane/I_FarPlane: clip distances. Returns new instance. */
        [[nodiscard]] static FCamera
        MakeOrthographic(Float I_Width,
                         Float I_Height,
                         Float I_NearPlane,
                         Float I_FarPlane) noexcept;

    public:
        /** World-space position. SetPosition marks view (and view-proj) dirty. */
        [[nodiscard]] constexpr const FVector3F&
        GetPosition() const noexcept { return Position; }
        constexpr void
        SetPosition(const FVector3F& I_Position) noexcept { Position = I_Position; MarkViewDirty(); }
        /** Orientation as quaternion. SetRotation marks view (and view-proj) dirty. */
        [[nodiscard]] constexpr const FQuaternion&
        GetRotation() const noexcept { return Rotation; }
        constexpr void
        SetRotation(const FQuaternion& I_Rotation) noexcept { Rotation = I_Rotation; MarkViewDirty(); }
        /** Set rotation from Euler angles (Yaw-Pitch-Roll). Marks view dirty. */
        void
        SetEulerAngles(FDegree I_Yaw, FDegree I_Pitch, FDegree I_Roll) noexcept;
        /** Returns Euler angles (X=Pitch, Y=Yaw, Z=Roll) extracted from current rotation. */
        [[nodiscard]] FVector3F
        GetEulerAngles() const noexcept;
        /** Current projection type (perspective or orthographic). */
        [[nodiscard]] constexpr EProjectionType
        GetProjectionType() const noexcept { return ProjectionType; }

        /** Vertical FOV (perspective only). Aspect ratio is width/height. */
        [[nodiscard]] constexpr FDegree
        GetFOVY() const noexcept { return FOV_Y; }
        [[nodiscard]] constexpr Float
        GetAspectRatio() const noexcept { return AspectRatio; }
        [[nodiscard]] constexpr Float
        GetNearPlane() const noexcept { return NearPlane; }
        [[nodiscard]] constexpr Float
        GetFarPlane() const noexcept { return FarPlane; }
        /** Orthographic viewport width/height (ortho only). */
        [[nodiscard]] constexpr Float
        GetOrthoWidth() const noexcept { return OrthoWidth; }
        [[nodiscard]] constexpr Float
        GetOrthoHeight() const noexcept { return OrthoHeight; }
        /** View matrix. Lazily updated when dirty; returns const reference. */
        [[nodiscard]] const FMatrix4x4F&
        GetViewMatrix() const noexcept;
        /** Projection matrix. Lazily updated when dirty; returns const reference. */
        [[nodiscard]] const FMatrix4x4F&
        GetProjectionMatrix() const noexcept;
        /** View * Projection. Lazily updated when view or projection dirty; returns const reference. */
        [[nodiscard]] const FMatrix4x4F&
        GetViewProjectionMatrix() const noexcept;

        /** Switch to perspective and set FOV, aspect, near/far. Marks projection (and view-proj) dirty. */
        constexpr void
        SetPerspective(FDegree I_FOVY,
                       Float   I_AspectRatio,
                       Float   I_NearPlane,
                       Float   I_FarPlane) noexcept
        {
            ProjectionType = EProjectionType::Perspective;
            FOV_Y          = I_FOVY;
            AspectRatio    = I_AspectRatio;
            NearPlane      = I_NearPlane;
            FarPlane       = I_FarPlane;
            OrthoWidth     = 0.0f;
            OrthoHeight    = 0.0f;
            MarkProjectionDirty();
        }

        /** Switch to orthographic and set width, height, near/far. Marks projection (and view-proj) dirty. */
        constexpr void
        SetOrthographic(Float I_Width,
                        Float I_Height,
                        Float I_NearPlane,
                        Float I_FarPlane) noexcept
        {
            ProjectionType = EProjectionType::Orthographic;
            OrthoWidth     = I_Width;
            OrthoHeight    = I_Height;
            NearPlane      = I_NearPlane;
            FarPlane       = I_FarPlane;
            MarkProjectionDirty();
        }

    private:
        /** Transform: world position and orientation. */
        FVector3F   Position {0.0f, 0.0f, 0.0f};
        FQuaternion Rotation {FQuaternion::Identity()};

        /** Projection type (perspective or orthographic). */
        EProjectionType ProjectionType {EProjectionType::Default};

        /** Perspective: vertical FOV and aspect ratio (width/height). */
        FDegree     FOV_Y {60.0f};
        Float       AspectRatio {16.0f / 9.0f};

        /** Orthographic: viewport width and height. */
        Float OrthoWidth  {0.0f};
        Float OrthoHeight {0.0f};

        /** Clip plane distances (shared by perspective and ortho). */
        Float NearPlane {0.1f};
        Float FarPlane  {1000.0f};

        /** Cached matrices; lazily updated when dirty. */
        mutable FMatrix4x4F ViewMatrix        {FMatrix4x4F::Identity()};
        mutable FMatrix4x4F ProjectionMatrix  {FMatrix4x4F::Identity()};
        mutable FMatrix4x4F ViewProjMatrix    {FMatrix4x4F::Identity()};

    public:
        FCamera() noexcept = default;

    private:
        /** Dirty-flag bits for lazy matrix update. */
        enum : UInt8
        {
            ViewDirtyMask       = 1U << 0,
            ProjectionDirtyMask = 1U << 1,
            ViewProjDirtyMask   = 1U << 2,

            AllDirtyMask = ViewDirtyMask | ProjectionDirtyMask | ViewProjDirtyMask,
        };
        mutable UInt8 MatrixDirtyFlags = AllDirtyMask;

        /** True if view / projection / view-proj matrix needs recompute. */
        inline Bool
        IsViewDirty()         const { return (MatrixDirtyFlags & ViewDirtyMask) != 0; }
        inline Bool
        IsProjectionDirty()   const { return (MatrixDirtyFlags & ProjectionDirtyMask) != 0; }
        inline Bool
        IsViewProjDirty()     const { return (MatrixDirtyFlags & ViewProjDirtyMask) != 0; }

        /** Marks view (and view-proj) dirty. Called when position or rotation changes. */
        constexpr void
        MarkViewDirty() const noexcept
        {
            MatrixDirtyFlags |= ViewDirtyMask       |
                                ViewProjDirtyMask;
        }

        /** Marks projection (and view-proj) dirty. Called when projection params change. */
        constexpr void
        MarkProjectionDirty() const noexcept
        {
            MatrixDirtyFlags |= ProjectionDirtyMask |
                                ViewProjDirtyMask;
        }

        /** Clears dirty bit after matrix has been recomputed. */
        constexpr void
        UnmarkViewDirty() const noexcept
        {
            MatrixDirtyFlags &= ~ViewDirtyMask;
        }

        constexpr void
        UnmarkProjectionDirty() const noexcept
        {
            MatrixDirtyFlags &= ~ProjectionDirtyMask;
        }

        constexpr void
        UnmarkViewProjDirty() const noexcept
        {
            MatrixDirtyFlags &= ~ViewProjDirtyMask;
        }

        /** Builds a left-handed view matrix from Position and Rotation.
         *  Convention: X-right, Y-up, Z-forward. Each basis vector becomes a row,
         *  and the translation column is -dot(axis, Position) to move the world
         *  into camera space. FMatrix4x4F ctor takes row-major arguments. */
        void
        UpdateViewMatrix() const noexcept
        {
            if(!IsViewDirty()) { return; }

            const FVector3F Right   = Rotation.RotateVector({1.0f, 0.0f, 0.0f});
            const FVector3F Up      = Rotation.RotateVector({0.0f, 1.0f, 0.0f});
            const FVector3F Forward = Rotation.RotateVector({0.0f, 0.0f, 1.0f});

            ViewMatrix = FMatrix4x4F{
                Right.X,    Right.Y,    Right.Z,    -(Right.Dot(Position)),
                Up.X,       Up.Y,       Up.Z,       -(Up.Dot(Position)),
                Forward.X,  Forward.Y,  Forward.Z,  -(Forward.Dot(Position)),
                0.0f,       0.0f,       0.0f,       1.0f
            };

            UnmarkViewDirty();
        }

        /** Builds the projection matrix.
         *  Vulkan conventions: depth Z mapped to [0, 1], Y-axis flipped (negative H)
         *  so clip-space Y points downward matching Vulkan NDC.
         *  Perspective: W[3][2]=1 gives left-handed (Z-forward) infinite-far support.
         *  Orthographic: same Y-flip and Z-remap. */
        void
        UpdateProjectionMatrix() const noexcept
        {
            if(!IsProjectionDirty()) { return; }

            if (ProjectionType == EProjectionType::Perspective)
            {
                const Float HalfTanFovY = Math::Tan(FOV_Y * 0.5f);
                const Float W = 1.0f / (AspectRatio * HalfTanFovY);
                const Float H = 1.0f / HalfTanFovY;
                const Float FRange = FarPlane / (FarPlane - NearPlane);

                ProjectionMatrix = FMatrix4x4F{
                    W,    0.0f, 0.0f,                      0.0f,
                    0.0f, -H,   0.0f,                      0.0f,
                    0.0f, 0.0f, FRange,                    -NearPlane * FRange,
                    0.0f, 0.0f, 1.0f,                      0.0f
                };
            }
            else
            {
                const Float W = (OrthoWidth  > 0.0f) ? OrthoWidth  : 1.0f;
                const Float H = (OrthoHeight > 0.0f) ? OrthoHeight : 1.0f;
                const Float FRange = 1.0f / (FarPlane - NearPlane);

                ProjectionMatrix = FMatrix4x4F{
                    2.0f / W, 0.0f,      0.0f,                   0.0f,
                    0.0f,     -2.0f / H, 0.0f,                   0.0f,
                    0.0f,     0.0f,      FRange,                 -NearPlane * FRange,
                    0.0f,     0.0f,      0.0f,                   1.0f
                };
            }

            UnmarkProjectionDirty();
        }

        /** Recomputes view and projection if dirty, then ViewProjMatrix = ProjectionMatrix * ViewMatrix. */
        void
        UpdateViewProjectionMatrix() const noexcept
        {
            if(!IsViewProjDirty()) { return; }

            UpdateViewMatrix();
            UpdateProjectionMatrix();

            ViewProjMatrix = ProjectionMatrix * ViewMatrix;

            UnmarkViewProjDirty();
        }
    };

    FCamera FCamera::
    MakePerspective(FDegree I_FOVY, Float I_AspectRatio,
                    Float I_NearPlane, Float I_FarPlane) noexcept
    {
        FCamera Camera;  /* SetPerspective then return. */
        Camera.SetPerspective(I_FOVY, I_AspectRatio, I_NearPlane, I_FarPlane);
        return Camera;
    }

    FCamera FCamera::
    MakeOrthographic(Float I_Width, Float I_Height,
                     Float I_NearPlane, Float I_FarPlane) noexcept
    {
        FCamera Camera;  /* SetOrthographic then return. */
        Camera.SetOrthographic(I_Width, I_Height, I_NearPlane, I_FarPlane);
        return Camera;
    }

    /** Rotation from Euler angles using YXZ intrinsic order (Yaw * Pitch * Roll).
     *  This matches the conventional FPS camera: yaw around world Y, pitch around
     *  local X, roll around local Z. */
    void FCamera::
    SetEulerAngles(FDegree I_Yaw, FDegree I_Pitch, FDegree I_Roll) noexcept
    {
        const FQuaternion Qy = FQuaternion::FromAxisAngle({0.0f, 1.0f, 0.0f}, I_Yaw);
        const FQuaternion Qx = FQuaternion::FromAxisAngle({1.0f, 0.0f, 0.0f}, I_Pitch);
        const FQuaternion Qz = FQuaternion::FromAxisAngle({0.0f, 0.0f, 1.0f}, I_Roll);
        Rotation = Qy * Qx * Qz;
        MarkViewDirty();
    }

    /** Extracts Euler angles (radians) from the rotation quaternion via its 3x3 matrix.
     *  Pitch = asin(-R(1,2)); gimbal lock handled when |sin(pitch)| approaches 1. 
     *  Returns: X=Yaw, Y=Pitch, Z=Roll. */
    FVector3F FCamera::
    GetEulerAngles() const noexcept
    {
        const FMatrix3x3F R = Rotation.ToMatrix3x3();
        const Float SP = -R(1, 2);
        const Float ClampedSP = Math::Clamp(SP, -1.0f, 1.0f);
        const Float Pitch = std::asin(ClampedSP);

        FVector3F Euler;
        if (Math::Abs(ClampedSP) < 0.9999f)
        {
            Euler.X = std::atan2(R(0, 2), R(2, 2));   // Yaw
            Euler.Z = std::atan2(R(1, 0), R(1, 1));   // Roll
        }
        else
        {   // Gimbal lock: pitch ~ +/-90deg, yaw and roll become coupled
            Euler.X = std::atan2(-R(2, 0), R(0, 0));
            Euler.Z = 0.0f;
        }
        Euler.Y = Pitch;
        return Euler;
    }

    const FMatrix4x4F&
    FCamera::GetViewMatrix() const noexcept
    {
        UpdateViewMatrix();
        return ViewMatrix;
    }

    const FMatrix4x4F&
    FCamera::GetProjectionMatrix() const noexcept
    {
        UpdateProjectionMatrix();
        return ProjectionMatrix;
    }

    const FMatrix4x4F&
    FCamera::GetViewProjectionMatrix() const noexcept
    {
        UpdateViewProjectionMatrix();
        return ViewProjMatrix;
    }
}
