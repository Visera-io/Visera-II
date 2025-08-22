module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime;
#define VISERA_MODULE_NAME "Runtime"
export import Visera.Runtime.Graphics;
export import Visera.Runtime.RHI;
export import Visera.Runtime.Window;

namespace Visera
{
    class VISERA_RUNTIME_API FRuntime
    {

    };

    export inline VISERA_RUNTIME_API TUniquePtr<FRuntime> GRuntime = MakeUnique<FRuntime>();
}