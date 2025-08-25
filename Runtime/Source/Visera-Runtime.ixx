module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime;
#define VISERA_MODULE_NAME "Runtime"
export import Visera.Runtime.Graphics;
export import Visera.Runtime.RHI;
export import Visera.Runtime.Window;
       import Visera.Core.OS.Time;

namespace Visera
{
    class VISERA_RUNTIME_API FRuntime
    {
    public:
        FRuntime()
        {

        }

        ~FRuntime()
        {
            std::cout << fmt::format("[Runtime] Finalizing Runtime (running time: {}s)\n", Timer.Elapsed().Seconds());
        }

    private:
        FHiResClock Timer{};
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FRuntime>
    GRuntime = MakeUnique<FRuntime>();
}