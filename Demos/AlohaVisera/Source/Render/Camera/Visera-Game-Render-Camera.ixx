module;
#include <Visera-Game.hpp>
export module Visera.Game.Render.Renderer;
#define VISERA_MODULE_NAME "Game.Render"
import Visera.Core.Math.Algebra;
import Visera.Core.Math.Trigonometry;

export namespace Visera
{
    // Left-handed camera
    class VISERA_ENGINE_API FCamera
    {
    public:
        enum class EProjectionType : UInt8
        {
            Perspective,
            Orthographic,
            
            Default = Perspective,
        };

        [[nodiscard]] static FCamera
        MakePerspective(FDegree I_FOVY,
                        Float   I_AspectRatio,
                        Float   I_NearPlane,
                        Float   I_FarPlane) noexcept;
        [[nodiscard]] static FCamera
        MakeOrthographic(Float I_Width,
                         Float I_Height,
                         Float I_NearPlane,
                         Float I_FarPlane) noexcept;

    public:
        [[nodiscard]] constexpr const FVector3F&
        GetPosition() const noexcept { return Position; }
        constexpr void
        SetPosition(const FVector3F& I_Position) noexcept { Position = I_Position; MarkViewDirty(); }
        [[nodiscard]] constexpr const FQuaternion&
        GetRotation() const noexcept { return Rotation; }
        constexpr void
        SetRotation(const FQuaternion& I_Rotation) noexcept { Rotation = I_Rotation; MarkViewDirty(); }
        void
        SetEulerAngles(FDegree I_Yaw, FDegree I_Pitch, FDegree I_Roll) noexcept;
        [[nodiscard]] FVector3F
        GetEulerAngles() const noexcept;
        [[nodiscard]] constexpr EProjectionType
        GetProjectionType() const noexcept { return ProjectionType; }


        [[nodiscard]] constexpr FDegree
        GetFOVY() const noexcept { return FOV_Y; }
        [[nodiscard]] constexpr Float
        GetAspectRatio() const noexcept { return AspectRatio; }
        [[nodiscard]] constexpr Float
        GetNearPlane() const noexcept { return NearPlane; }
        [[nodiscard]] constexpr Float
        GetFarPlane() const noexcept { return FarPlane; }
        [[nodiscard]] constexpr Float
        GetOrthoWidth() const noexcept { return OrthoWidth; }
        [[nodiscard]] constexpr Float
        GetOrthoHeight() const noexcept { return OrthoHeight; }
        [[nodiscard]] const FMatrix4x4F&
        GetViewMatrix() const noexcept;
        [[nodiscard]] const FMatrix4x4F&
        GetProjectionMatrix() const noexcept;
        [[nodiscard]] const FMatrix4x4F&
        GetViewProjectionMatrix() const noexcept;

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
        FVector3F   Position {0.0f, 0.0f, 0.0f};
        FQuaternion Rotation {}; // identity by default

        EProjectionType ProjectionType {EProjectionType::Default};

        FDegree     FOV_Y {60.0f};
        Float       AspectRatio {16.0f / 9.0f};

        Float OrthoWidth  {0.0f};
        Float OrthoHeight {0.0f};

        Float NearPlane {0.1f};
        Float FarPlane  {1000.0f};

        mutable FMatrix4x4F ViewMatrix        {FMatrix4x4F::Identity()};
        mutable Bool        bViewDirty        {True};
        mutable FMatrix4x4F ProjectionMatrix  {FMatrix4x4F::Identity()};
        mutable Bool        bProjectionDirty  {True};
        mutable FMatrix4x4F ViewProjMatrix    {FMatrix4x4F::Identity()};
        mutable Bool        bViewProjDirty    {True};

    public:
        FCamera() noexcept = default;

    private:
        constexpr void
        MarkViewDirty() const noexcept
        {
            bViewDirty     = True;
            bViewProjDirty = True;
        }

        constexpr void
        MarkProjectionDirty() const noexcept
        {
            bProjectionDirty = True;
            bViewProjDirty   = True;
        }

        void
        UpdateViewMatrix() const noexcept;

        void
        UpdateProjectionMatrix() const noexcept;

        void
        UpdateViewProjectionMatrix() const noexcept;
    };
}
