module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Assets;
import Visera.Core.Log;
import Visera.Core.OS.FileSystem;

namespace Visera
{
    export using EAssetType = IAsset::EType;

    export enum class EAssetSource
    {
        Engine,
        Studio,
        App,

        Any,
    };

    class VISERA_RUNTIME_API FAssetHub : public IGlobalSingleton
    {
    public:

    public:
        void Bootstrap() override;
        void Terminate() override;
        FAssetHub() : IGlobalSingleton("AssetHub") {};
        ~FAssetHub() override= default;

    private:
        TMap<EAssetSource, FFileSystem> Roots;
    };

    export inline VISERA_RUNTIME_API auto
    GAssetHub = MakeUnique<FAssetHub>();

    void FAssetHub::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping AssetHub.");
        Roots[EAssetSource::Engine] = FFileSystem{FPath(VISERA_ENGINE_DIR)};
        Roots[EAssetSource::Studio] = FFileSystem{FPath(VISERA_STUDIO_DIR)};
        Roots[EAssetSource::App]    = FFileSystem{FPath(VISERA_APP_DIR)};
    }

    void FAssetHub::
    Terminate()
    {
        LOG_DEBUG("Terminating AssetHub.");
    }
}
