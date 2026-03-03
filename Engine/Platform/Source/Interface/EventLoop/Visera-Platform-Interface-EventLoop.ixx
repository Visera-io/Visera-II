module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface.EventLoop;
#define VISERA_MODULE_NAME "Platform.Interface"

export namespace Visera
{
    class VISERA_PLATFORM_API IPlatformEventLoop
    {
    public:
        virtual void
        PollEvents() const = 0;
        virtual void
        WaitEvents() const = 0;

        virtual ~IPlatformEventLoop() = default;
    };
}
