module;
#include <Visera-Engine.hpp>
export module Visera.Engine;
#define VISERA_MODULE_NAME "Engine"
import Visera.Engine.Render;
import Visera.Engine.UI;
import Visera.Runtime;
import Visera.Core.Log;

namespace Visera
{
    class VISERA_ENGINE_API FEngine : public IGlobalSingleton
    {
    public:
        void Bootstrap() override
        {
            LOG_DEBUG("Bootstrapping Engine.");
            //GWindow->Initialize();

            GWindow->Bootstrap();
            GRHI->Bootstrap();

            Statue = EStatues::Bootstrapped;
        }

        void Run()
        {
            VISERA_ASSERT(IsBootstrapped());

            while (!GWindow->ShouldClose())
            {
                GWindow->PollEvents();
                //GWindow->PollEvents(); // You MUST call this function on MacOS.

            }
        }

        void Terminate() override
        {
            LOG_DEBUG("Terminating Engine.");

            GRHI->Terminate();
            GWindow->Terminate();

            Statue = EStatues::Terminated;
        }

        FEngine() : IGlobalSingleton("Engine") {};
    };

    export inline VISERA_ENGINE_API TUniquePtr<FEngine>
    GEngine = MakeUnique<FEngine>();


}
