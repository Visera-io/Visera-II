module;
#include <Visera-Engine.hpp>
#include <entt/entt.hpp>
export module Visera.Engine.World.System.Movement;
#define VISERA_MODULE_NAME "Engine.World"
import Visera.Engine.World.Component.Transform;
import Visera.Engine.World.Component.Velocity;

namespace Visera
{
    export class VISERA_ENGINE_API SMovement
    {
    public:
        void
        Tick(entt::registry& I_Registry, Float I_DeltaTime)
        {
            auto View = I_Registry.view<CTransform2D, CVelocity2D>();

            View.each([I_DeltaTime](CTransform2D& T, const CVelocity2D& V)
            {
                T.Translation += V * I_DeltaTime;
            });
        }
    };
}