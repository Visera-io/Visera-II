module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Platform;
#define VISERA_MODULE_NAME "Runtime.Platform"
export import Visera.Runtime.Platform.Interface;
#if defined(VISERA_ON_WINDOWS_SYSTEM)
import Visera.Runtime.Platform.Windows;
#elif defined(VISERA_ON_APPLE_SYSTEM)
import Visera.Runtime.Platform.MacOS;
#endif
import Visera.Core.Global;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FPlatform : public IGlobalSingleton<FPlatform>
    {
    public:
        [[nodiscard]] inline TSharedPtr<ILibrary>
        LoadLibrary(const FPath& I_Path) const { return Platform->LoadLibrary(I_Path); }
        [[nodiscard]] inline const FPath&
        GetExecutableDirectory() const { return Platform->GetExecutableDirectory(); }
        [[nodiscard]] inline const FPath&
        GetResourceDirectory() const { return Platform->GetResourceDirectory(); }
        [[nodiscard]] inline const FPath&
        GetFrameworkDirectory() const { return Platform->GetFrameworkDirectory(); }
        [[nodiscard]] inline Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) const { return Platform->SetEnvironmentVariable(I_Variable, I_Value); }

        [[nodiscard]] inline EPlatform
        GetType() const { return Platform->GetType(); }

    private:
        TUniquePtr<IPlatform> Platform;

    public:
        FPlatform() : IGlobalSingleton("Platform") {}
        void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Platform.");
#if defined(VISERA_ON_WINDOWS_SYSTEM)
            Platform = MakeUnique<FWindowsPlatform>();
#elif defined(VISERA_ON_APPLE_SYSTEM)
            Platform = MakeUnique<FMacOSPlatform>();
#endif

            Status = EStatus::Bootstrapped;
        }
        void Terminate() override
        {
            LOG_TRACE("Terminating Platform.");
            Platform.reset();

            Status = EStatus::Terminated;
        }
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FPlatform>
    GPlatform = MakeUnique<FPlatform>();
}