module;
#include <Visera-Game.hpp>
export module Visera.Game.World.Component.Transform;
#define VISERA_MODULE_NAME "Game.World"
import Visera.Core.Math.Algebra.Vector;
import Visera.Core.Math.Geometry.Rotation;

namespace Visera
{
    export class VISERA_ENGINE_API CTransform2D
    {
    public:
        FVector2F   Translation   {0.0, 0.0};
        FRotation2D Rotation      { 0.0 };
        FVector2F   Scale         {1.0, 1.0};
    };
}
