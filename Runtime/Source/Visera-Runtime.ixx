module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime;
#define VISERA_MODULE_NAME "Runtime"
export import Visera.Runtime.Platform;
       import Visera.Core.Log;

export namespace Visera
{
    class GRuntime
    {

    public://[TODO]: Engine Only
        static void
        Bootstrap();
        static void
        Terminate();
    };

    void GRuntime::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping Runtime...");
    }

    void GRuntime::
    Terminate()
    {
        LOG_DEBUG("Terminating Runtime...");
    }
}
