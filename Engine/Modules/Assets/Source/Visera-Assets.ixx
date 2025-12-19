module;
#include <Visera-Assets.hpp>
export module Visera.Assets;
#define VISERA_MODULE_NAME "Assets"
import Visera.Core.Types.Path;
import Visera.Core.Global;
import Visera.Runtime.Log;
import Visera.Assets.Image;

namespace Visera
{
    export class VISERA_ASSETS_API FAssets : public IGlobalSingleton<FAssets>
    {
    public:
        [[nodiscard]] inline TSharedPtr<FImage>
        CreateImage(const FPath& I_Path) const;

    private:

    public:
        FAssets() : IGlobalSingleton("Assets") {}

        void
        Bootstrap() override;
        void
        Terminate() override;
    };

    export inline VISERA_ASSETS_API TUniquePtr<FAssets>
    GAssets = MakeUnique<FAssets>();

    void FAssets::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Assets.");

        Status = EStatus::Bootstrapped;
    }

    void FAssets::
    Terminate()
    {
        LOG_TRACE("Terminating Assets.");

        Status = EStatus::Terminated;
    }

    TSharedPtr<FImage> FAssets::
    CreateImage(const FPath& I_Path) const
    {
        LOG_DEBUG("Creating an Image from \"{}\".", I_Path);
        return MakeShared<FImage>(I_Path);
    }
}