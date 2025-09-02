module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Asset;
#define VISERA_MODULE_NAME "Runtime.Asset"
import Visera.Core.Log;

namespace Visera
{
    class VISERA_RUNTIME_API FAssetHub : IGlobalSingleton
    {
    public:
        void Bootstrap() override;
        void Terminate() override;
        FAssetHub() : IGlobalSingleton("AssetHub") {};
        ~FAssetHub() override= default;
    };

    export inline VISERA_RUNTIME_API auto
    GAssetHub = MakeUnique<FAssetHub>();

    void FAssetHub::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping AssetHub.");
    }

    void FAssetHub::
    Terminate()
    {
        LOG_DEBUG("Terminating AssetHub.");
    }
}
