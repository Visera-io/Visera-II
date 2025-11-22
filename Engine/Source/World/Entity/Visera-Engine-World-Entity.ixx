module;
#include <Visera-Engine.hpp>
#include <entt/entt.hpp>
export module Visera.Engine.World.Entity;
#define VISERA_MODULE_NAME "Engine.World"
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FEntity
    {
    public:

    private:
        entt::registry& _registry; //[TODO]: remove
    };
}
