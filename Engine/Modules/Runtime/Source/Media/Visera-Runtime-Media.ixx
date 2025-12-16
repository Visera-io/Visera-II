module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Media;
#define VISERA_MODULE_NAME "Runtime.Media"
import Visera.Core.Types.Path;
import Visera.Core.Global;
import Visera.Runtime.Log;
import Visera.Runtime.Media.Image;

namespace Visera
{
    export class VISERA_RUNTIME_API FMedia : public IGlobalSingleton<FMedia>
    {
    public:
        [[nodiscard]] inline TSharedPtr<FImage>
        CreateImage(const FPath& I_Path) const;

    private:

    public:
        FMedia() : IGlobalSingleton("Media") {}

        void
        Bootstrap() override;
        void
        Terminate() override;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FMedia>
    GMedia = MakeUnique<FMedia>();

    void FMedia::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Media.");

        Status = EStatus::Bootstrapped;
    }

    void FMedia::
    Terminate()
    {
        LOG_TRACE("Terminating Media.");

        Status = EStatus::Terminated;
    }

    TSharedPtr<FImage> FMedia::
    CreateImage(const FPath& I_Path) const
    {
        LOG_DEBUG("Creating an Image from \"{}\".", I_Path);
        return MakeShared<FImage>(I_Path);
    }
}