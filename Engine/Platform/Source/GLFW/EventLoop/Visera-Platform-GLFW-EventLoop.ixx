module;
#include <Visera-Platform.hpp>
#include <GLFW/glfw3.h>
export module Visera.Platform.GLFW.EventLoop;
#define VISERA_MODULE_NAME "Platform.GLFW"
import Visera.Platform.Interface.EventLoop;
import Visera.Core.OS.Thread;
import Visera.Core.Log;

export namespace Visera
{
    class VISERA_PLATFORM_API FGLFWEventLoop : public IPlatformEventLoop
    {
    public:
        void
        PollEvents() const override { VISERA_ASSERT(FThread::IsMainThread()); glfwPollEvents(); }
        void
        WaitEvents() const override { VISERA_ASSERT(FThread::IsMainThread()); glfwWaitEvents(); }

    public:
        FGLFWEventLoop()
        {
            if (!glfwInit()) { LOG_FATAL("Failed to initialize GLFW!"); }
        }
    };
}