module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface;
#define VISERA_MODULE_NAME "Platform"
export import Visera.Platform.Interface.Library;
export import Visera.Platform.Interface.Path;
export import Visera.Platform.Interface.Window;
export import Visera.Platform.Interface.EventLoop;
export import Visera.Platform.Interface.FileSystem;
export import Visera.Core.Types.Pointer.Unique;
export import Visera.Core.Types.Text;

export namespace Visera
{
    enum class EPlatform
    {
        Unknown,

        Windows,
        MacOS,
    };

    class VISERA_PLATFORM_API IPlatform
    {
    public:
        [[nodiscard]] virtual TUniquePtr<IPlatformWindow>
        CreateWindow(const FText& I_Title, UInt32 I_Width, UInt32 I_Height) const = 0;
        [[nodiscard]] virtual TSharedPtr<IPlatformLibrary>
        LoadLibrary(const IPlatformPath& I_Path) const = 0;
        [[nodiscard]] virtual TUniquePtr<IPlatformPath>
        GetExecutableDirectory() const = 0;
        [[nodiscard]] virtual TUniquePtr<IPlatformPath>
        GetResourceDirectory() const = 0;
        [[nodiscard]] virtual TUniquePtr<IPlatformPath>
        GetFrameworkDirectory() const = 0;
        [[nodiscard]] virtual Bool
        SetEnvironmentVariable(const FText& I_Variable, const FText& I_Value) const = 0;
        [[nodiscard]] virtual FUUID
        GenerateUUID() const = 0;
        virtual void
        SetCurrentThreadName(const FText& I_Name) const = 0;
        [[nodiscard]] virtual IPlatformFileSystem&
        GetFileSystem() const = 0;
        virtual void
        PollEvents() const = 0;
        virtual void
        WaitEvents() const = 0;
        /** Currently focused platform window (e.g. for input). May be nullptr. */
        [[nodiscard]] virtual IPlatformWindow*
        GetFocusedWindow() const = 0;
        [[nodiscard]] inline EPlatform
        GetType() const { return Type; }

    public:
        explicit IPlatform() = delete;
        explicit IPlatform(EPlatform I_Type) : Type(I_Type) {};
        virtual ~IPlatform() {}

    private:
        const EPlatform Type;
    };   
}