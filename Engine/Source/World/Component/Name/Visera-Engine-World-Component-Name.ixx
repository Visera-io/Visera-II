module;
#include <Visera-Engine.hpp>
export module Visera.Engine.World.Component.Name;
#define VISERA_MODULE_NAME "Engine.World"
import Visera.Core.Types.Name;

namespace Visera
{
    export class VISERA_ENGINE_API CName
    {
    public:
        FName Value { EName::None };
    };
}
