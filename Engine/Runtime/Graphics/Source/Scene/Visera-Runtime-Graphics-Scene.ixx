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
    class VISERA_RUNTIME_API FScene
    {
    public:


    private:
        TUniquePtr<FCamera>             Camera;
        TUniquePtr<FLight>              Light;
        TArray<TUniquePtr<IRenderable>> Renderables;

    public:
        struct FSnapshot
        {

        };

        /** Returns a read-only snapshot for this frame. Safe to call from Main; result can be enqueued to Graphics. */
        [[nodiscard]] FSnapshot
        Snapshot() const
        {
            return FSnapshot{};
        }
    };
}
