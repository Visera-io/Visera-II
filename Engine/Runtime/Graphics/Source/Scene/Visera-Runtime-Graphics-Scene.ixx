module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Scene;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Runtime.Graphics.Scene.Camera;
export import Visera.Runtime.Graphics.Scene.Light;
export import Visera.Runtime.Graphics.Scene.Renderable;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.Pointer;

export namespace Visera
{
    /** Per-frame scene payload: renderables and lights. Read-only on the graphics thread. */
    struct VISERA_RUNTIME_API FRenderData
    {
        TArray<TSharedPtr<IRenderable>> Renderables;
        TArray<FLight>                 Lights;

        [[nodiscard]] const TArray<TSharedPtr<IRenderable>>&
        GetRenderables() const { return Renderables; }
        [[nodiscard]] const TArray<FLight>&
        GetLights() const { return Lights; }
    };

    /** Per-frame view payload: view and projection matrices. Built from FCamera. */
    struct VISERA_RUNTIME_API FRenderView
    {
        FMatrix4x4F ViewMatrix       {FMatrix4x4F::Identity()};
        FMatrix4x4F ProjectionMatrix {FMatrix4x4F::Identity()};
    };

    class VISERA_RUNTIME_API FScene
    {
    public:
        void
        SetCamera(TSharedPtr<FCamera> I_Camera) { Camera = std::move(I_Camera); }
        void
        SetLight(TSharedPtr<FLight> I_Light) { Light = std::move(I_Light); }
        void
        SetRenderables(TArray<TSharedPtr<IRenderable>> I_Renderables) { Renderables = std::move(I_Renderables); }
        void
        ClearRenderables() { Renderables.Clear(); }
        void
        AddRenderable(TSharedPtr<IRenderable> I_Renderable) { Renderables.PushBack(std::move(I_Renderable)); }

        /** Fills Out with current renderables and lights. Safe to call from Main. */
        void
        BuildRenderData(FRenderData& Out) const
        {
            Out.Renderables = Renderables;
            Out.Lights.Clear();
            if (Light) { Out.Lights.PushBack(*Light); }
        }

        /** Returns view/projection from current camera, or identity if no camera. Safe to call from Main. */
        [[nodiscard]] FRenderView
        BuildRenderView() const
        {
            FRenderView Out;
            if (Camera)
            {
                Out.ViewMatrix       = Camera->GetViewMatrix();
                Out.ProjectionMatrix = Camera->GetProjectionMatrix();
            }
            return Out;
        }

    private:
        TSharedPtr<FCamera>             Camera;
        TSharedPtr<FLight>              Light;
        TArray<TSharedPtr<IRenderable>> Renderables;
    };
}
