module;
#include <Visera-Assets.hpp>
export module Visera.Assets;
#define VISERA_MODULE_NAME "Assets"
import Visera.Core.Types.Path;
import Visera.Global.Service;
import Visera.Global.Log;
import Visera.Assets.Image;

namespace Visera
{
    export class VISERA_ASSETS_API FAssets : public IGlobalService<FAssets>
    {
    public:
        [[nodiscard]] inline TSharedPtr<FImage>
        CreateImage(const FPath& I_Path) const;

    private:

    public:
        FAssets() : IGlobalService(FName{"Assets"}) {}
    };

    export inline VISERA_ASSETS_API TUniquePtr<FAssets>
    GAssets = MakeUnique<FAssets>();

    TSharedPtr<FImage> FAssets::
    CreateImage(const FPath& I_Path) const
    {
        LOG_DEBUG("Creating an Image from \"{}\".", I_Path);
        return MakeShared<FImage>(I_Path);
    }
}