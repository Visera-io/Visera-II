module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Event.Signal;
#define VISERA_MODULE_NAME "Runtime.Event"

namespace Visera
{
    export VISERA_RUNTIME_API class FSignal
    {
    public:
        [[nodiscard]] Bool
        Fire() const { VISERA_WIP; }
        [[nodiscard]] Bool
        Connect() const { VISERA_WIP; }
        [[nodiscard]] Bool
        Disconnect() const { VISERA_WIP; }

    private:
        TList<std::function<void()>> Slots;
    }
}
