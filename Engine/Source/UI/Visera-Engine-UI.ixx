module;
#include <Visera-Engine.hpp>
export module Visera.Engine.UI;
#define VISERA_MODULE_NAME "Engine.UI"
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FUI
    {

    };

    export inline VISERA_ENGINE_API TUniquePtr<FUI>
    GUI = MakeUnique<FUI>();
}
