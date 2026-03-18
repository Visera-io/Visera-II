module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface;
#define VISERA_MODULE_NAME "Platform.Interface"
export import Visera.Platform.Interface.Library;
export import Visera.Platform.Interface.Path;
export import Visera.Platform.Interface.Window;
export import Visera.Platform.Interface.Device;
export import Visera.Platform.Interface.EventLoop;
export import Visera.Platform.Interface.FileSystem;
export import Visera.Core.Types.Pointer.Unique;
export import Visera.Core.Types.String;
       import Visera.Core.Types.Optional;

export namespace Visera
{
    class VISERA_PLATFORM_API IPlatform
    {
    public:
        /** Platform identifier for branching (e.g. FPlatform::IsPlatform). */
        [[nodiscard]] virtual FStringView GetPlatformName() const = 0;
        [[nodiscard]] virtual TUniquePtr<IPlatformWindow>
        CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) const = 0;
        [[nodiscard]] virtual TSharedPtr<IPlatformLibrary>
        LoadLibrary(const IPlatformPath& I_Path) const = 0;
        [[nodiscard]] virtual TUniquePtr<IPlatformPath>
        GetExecutableDirectory() const = 0;
        [[nodiscard]] virtual TUniquePtr<IPlatformPath>
        GetResourceDirectory() const = 0;
        [[nodiscard]] virtual TUniquePtr<IPlatformPath>
        GetFrameworkDirectory() const = 0;
        /** Logs directory per platform convention (e.g. Windows: LocalAppData/AppName/Logs, macOS: ~/Library/Logs/BundleId). */
        [[nodiscard]] virtual FPath
        GetLogsDirectory() const = 0;
        [[nodiscard]] virtual Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) const = 0;
        /** Returns the value if the variable is set; nullopt if unset. */
        [[nodiscard]] virtual TOptional<FString>
        GetEnvironmentVariable(FStringView I_Variable) const = 0;
        [[nodiscard]] virtual FUUID
        GenerateUUID() const = 0;
        virtual void
        SetCurrentThreadName(FStringView I_Name) const = 0;
        /** May be nullptr if platform has no file system (e.g. Null). */
        [[nodiscard]] virtual IPlatformFileSystem*
        GetFileSystem() const = 0;
        virtual void
        PollEvents() const = 0;
        virtual void
        WaitEvents() const = 0;
        /** Currently focused platform window (e.g. for input). May be nullptr. */
        [[nodiscard]] virtual IPlatformWindow*
        GetFocusedWindow() const = 0;

    public:
        explicit IPlatform() = default;
        virtual ~IPlatform() {}
    };   
}