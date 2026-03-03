module;
#include <Visera-Platform.hpp>
#include <GLFW/glfw3.h>
export module Visera.Platform.Cross.GLFW.EventLoop;
#define VISERA_MODULE_NAME "Platform.Cross"
import Visera.Platform.Interface.EventLoop;
import Visera.Core.Log;

export namespace Visera
{
    class VISERA_PLATFORM_API FGLFWEventLoop : public IPlatformEventLoop
    {
    public:
        void
        PollEvents() const override { glfwPollEvents(); }
        void
        WaitEvents() const override { glfwWaitEvents(); }
    
    public:
        FGLFWEventLoop() { if (!glfwInit()) { LOG_ERROR("Failed to initialize GLFW!"); } }
    };
}