module;
#include <Visera-Global.hpp>
export module Visera.Global.Scheduler;
#define VISERA_MODULE_NAME "Global.Scheduler"
import Visera.Global.Service;
import Visera.Global.Log;

namespace Visera
{
    export class VISERA_GLOBAL_API FScheduler : public IGlobalService<FScheduler>
    {
    public:

    private:

    public:
        FScheduler() : IGlobalService(FName{"Scheduler"})
        {

        }
    };

    export inline VISERA_GLOBAL_API TUniquePtr<FScheduler>
    GScheduler = MakeUnique<FScheduler>();
}