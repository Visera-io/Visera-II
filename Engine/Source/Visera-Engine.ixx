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
            LOG_TRACE("Bootstrapping Engine.");

            GAssetHub->Bootstrap();
            GWindow->Bootstrap();
            GRHI->Bootstrap();
            GAudio->Bootstrap();
            GScene->Bootstrap();

            Status = EStatus::Bootstrapped;
        }

        void Run()
        {
            VISERA_ASSERT(IsBootstrapped());

            if (!GWindow->IsBootstrapped())
            {
                LOG_INFO("Off-Screen Mode.");
                return;
            }
            else
            {

            }

            while (!GWindow->ShouldClose())
            {
                GWindow->PollEvents();
                GAudio->Tick();
            }
        }

        void Terminate() override
        {
            LOG_TRACE("Terminating Engine.");

            GScene->Terminate();
            GAudio->Terminate();
            GRHI->Terminate();
            GWindow->Terminate();
            GAssetHub->Terminate();

            Status = EStatus::Terminated;
        }

        FEngine() : IGlobalSingleton("Engine") {};
    };

    export inline VISERA_ENGINE_API TUniquePtr<FEngine>
    GEngine = MakeUnique<FEngine>();


}
