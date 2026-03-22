module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Null;
#define VISERA_MODULE_NAME "Platform.Null"
export import Visera.Platform.Null.Device;
       import Visera.Platform.Interface;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.String;
       import Visera.Core.Log;

export namespace Visera
{
    /** Null platform; no functionality. All APIs VISERA_ASSERT(False) and return safe defaults. */
    class VISERA_PLATFORM_API FNullPlatform : public IPlatform
    {
    public:
        FNullPlatform()
        { LOG_WARN("Using Null platform; window/input and many APIs are no-op. Consider building with a supported platform (Windows/MacOS) or GLFW."); }

        [[nodiscard]] TUniquePtr<IPlatformWindow>
        CreateWindow(FStringView, UInt32, UInt32, Bool, Bool, Bool) const override
        { VISERA_ASSERT(False); return nullptr; }

        [[nodiscard]] TSharedPtr<IPlatformLibrary>
        LoadLibrary(const IPlatformPath&) const override
        { VISERA_ASSERT(False); return nullptr; }

        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetExecutableDirectory() const override
        { VISERA_ASSERT(False); return nullptr; }

        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetResourceDirectory() const override
        { VISERA_ASSERT(False); return nullptr; }

        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetFrameworkDirectory() const override
        { VISERA_ASSERT(False); return nullptr; }

        [[nodiscard]] FPath
        GetLogsDirectory() const override
        { return FPath(); }

        [[nodiscard]] Bool
        SetEnvironmentVariable(FStringView, FStringView) const override
        { VISERA_ASSERT(False); return False; }

        [[nodiscard]] TOptional<FString>
        GetEnvironmentVariable(FStringView) const override
        { VISERA_ASSERT(False); return std::nullopt; }

        [[nodiscard]] FUUID
        GenerateUUID() const override
        { VISERA_ASSERT(False); return {}; }

        void
        SetCurrentThreadName(FStringView) const override
        { VISERA_ASSERT(False); }

        [[nodiscard]] IPlatformFileSystem*
        GetFileSystem() const override
        { return nullptr; }

        void PollEvents() const override { }
        void WaitEvents() const override { }

        [[nodiscard]] IPlatformWindow*
        GetFocusedWindow() const override
        { return nullptr; }

        [[nodiscard]] FStringView
        GetPlatformName() const override { return "Null"; }
    };
}
