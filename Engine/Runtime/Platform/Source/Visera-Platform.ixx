module;
#include <Visera-Platform.hpp>
export module Visera.Platform;
#define VISERA_MODULE_NAME "Platform"
       import Visera.Platform.OS;
       import Visera.Platform.OS.Interface; //[TODO]: Redesign
export import Visera.Platform.Window;
export import Visera.Platform.Input;
       import Visera.Global.Service;
       import Visera.Core.OS.FileSystem;
       import Visera.Global.Log;

namespace Visera
{
    export class VISERA_PLATFORM_API FPlatform : public IGlobalService<FPlatform>
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
        [[nodiscard]] inline FUUID
        GenerateUUID() const { return Platform->GenerateUUID(); }

        [[nodiscard]] inline EPlatform
        GetType() const { return Platform->GetType(); }

    private:
        TUniquePtr<IOS> Platform;

    public:
        FPlatform() : IGlobalService(FName{"Platform"}) {}
        /*void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Platform.");
#if defined(VISERA_ON_WINDOWS_SYSTEM)
            Platform = MakeUnique<FWindowsPlatform>();
#elif defined(VISERA_ON_APPLE_SYSTEM)
            Platform = MakeUnique<FMacOSPlatform>();
#endif

            if (auto Error = FFileSystem::CreateDirectory(Platform->GetCacheDirectory()); Error)
            { LOG_ERROR("Failed to create cache directory -- {}!", Error.message()); }

#if !defined(VISERA_OFFSCREEN_MODE)
            GWindow     ->Bootstrap();
            GInput      ->Bootstrap();
#endif

            Status = EStatus::Bootstrapped;
        }
        void Terminate() override
        {
            LOG_TRACE("Terminating Platform.");
            
#if !defined(VISERA_OFFSCREEN_MODE)
            GInput      ->Terminate();
            GWindow     ->Terminate();
#endif

            Platform.reset();

            Status = EStatus::Terminated;
        }*/
    };

    export inline VISERA_PLATFORM_API TUniquePtr<FPlatform>
    GPlatform = MakeUnique<FPlatform>();
}