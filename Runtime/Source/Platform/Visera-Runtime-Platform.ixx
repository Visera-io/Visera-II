module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Platform;
#define VISERA_MODULE_NAME "Runtime.Platform"
import Visera.Core.Log;

namespace Visera { class GRuntime;}

export namespace Visera
{

    class GPlatform
    {
        friend class GRuntime;
    private:
        static void
        Bootstrap();
        static void
        Terminate();
    };

    void GPlatform::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping Platform...");
    }

    void GPlatform::
    Terminate()
    {
        LOG_DEBUG("Terminating Platform...");
    }
}