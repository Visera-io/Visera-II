module;
#include <Visera-Platform.hpp>
#include <GLFW/glfw3.h>
export module Visera.Platform.GLFW;
#define VISERA_MODULE_NAME "Platform.GLFW"
export import Visera.Platform.GLFW.EventLoop;
export import Visera.Platform.GLFW.Window;
export import Visera.Platform.GLFW.Device;
       import Visera.Platform.Interface;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.UUID;
       import Visera.Core.Types.String;
       import Visera.Core.OS.Thread;
       import Visera.Core.Log;

export namespace Visera
{
    /** GLFW-backed platform (window/events only); unimplemented APIs LOG_ERROR and return safe defaults. UUID uses Core FUUID::Generate. */
    class VISERA_PLATFORM_API FGLFWPlatform : public IPlatform
    {
    public:
        /** Returns true if GLFW was built with at least one platform backend (uses glfwPlatformSupported). */
        [[nodiscard]] static Bool IsSupported()
        {
            return glfwPlatformSupported(GLFW_PLATFORM_WIN32)   != 0
                || glfwPlatformSupported(GLFW_PLATFORM_COCOA)   != 0
                || glfwPlatformSupported(GLFW_PLATFORM_X11)     != 0
                || glfwPlatformSupported(GLFW_PLATFORM_WAYLAND) != 0
                || glfwPlatformSupported(GLFW_PLATFORM_NULL)    != 0;
        }

        FGLFWPlatform()
        {
            glfwSetErrorCallback([](Int32 I_Error, const char* I_Message)
            { LOG_ERROR("{} (error:{})", I_Message, I_Error); });
            if (!glfwInit())
            { LOG_FATAL("Failed to initialize GLFW!"); }
        }

        [[nodiscard]] TUniquePtr<IPlatformWindow>
        CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) const override
        { return MakeUnique<FGLFWWindow>(I_Title, I_Width, I_Height); }

        [[nodiscard]] TSharedPtr<IPlatformLibrary>
        LoadLibrary(const IPlatformPath&) const override
        { LOG_ERROR("FGLFWPlatform: LoadLibrary not implemented."); return nullptr; }

        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetExecutableDirectory() const override
        { LOG_ERROR("FGLFWPlatform: GetExecutableDirectory not implemented."); return nullptr; }

        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetResourceDirectory() const override
        { LOG_ERROR("FGLFWPlatform: GetResourceDirectory not implemented."); return nullptr; }

        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetFrameworkDirectory() const override
        { LOG_ERROR("FGLFWPlatform: GetFrameworkDirectory not implemented."); return nullptr; }

        [[nodiscard]] FPath
        GetLogsDirectory() const override
        { return FPath(); }

        [[nodiscard]] Bool
        SetEnvironmentVariable(FStringView, FStringView) const override
        { LOG_ERROR("FGLFWPlatform: SetEnvironmentVariable not implemented."); return False; }

        [[nodiscard]] TOptional<FString>
        GetEnvironmentVariable(FStringView) const override
        { LOG_ERROR("FGLFWPlatform: GetEnvironmentVariable not implemented."); return std::nullopt; }

        [[nodiscard]] FUUID
        GenerateUUID() const override
        { return FUUID::Generate(); }

        void
        SetCurrentThreadName(FStringView) const override
        { LOG_ERROR("FGLFWPlatform: SetCurrentThreadName not implemented."); }

        [[nodiscard]] IPlatformFileSystem*
        GetFileSystem() const override
        { LOG_ERROR("FGLFWPlatform: GetFileSystem not implemented."); return nullptr; }

        void
        PollEvents() const override
        { EventLoop.PollEvents(); }

        void
        WaitEvents() const override
        { EventLoop.WaitEvents(); }

        [[nodiscard]] IPlatformWindow*
        GetFocusedWindow() const override
        { return FGLFWWindow::GetFocusedPlatformWindow(); }
        [[nodiscard]] FStringView
        GetPlatformName() const override { return "GLFW"; }

    private:
        mutable FGLFWEventLoop   EventLoop;
    };
}
