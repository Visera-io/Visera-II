module;
#include <Visera-Engine.hpp>
export module Visera.Engine.World;
#define VISERA_MODULE_NAME "Engine.World"
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FWorld
    {

    };

    export inline VISERA_ENGINE_API TUniquePtr<FWorld>
    GWorld = MakeUnique<FWorld>();
}
