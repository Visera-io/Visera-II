module;
#include <Visera-Game.hpp>
export module Visera.Game.World.Component.Name;
#define VISERA_MODULE_NAME "Game.World"
import Visera.Global.Name;

namespace Visera
{
    export class VISERA_ENGINE_API CName
    {
    public:
        FName Value;
    };
}
