module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Event.Signal;
#define VISERA_MODULE_NAME "Runtime.Event"

namespace Visera
{
    export class VISERA_RUNTIME_API FSignal
    {
    public:
        [[nodiscard]] Bool
        Fire() const { VISERA_WIP; return False; }
        [[nodiscard]] Bool
        Connect() const { VISERA_WIP; return False; }
        [[nodiscard]] Bool
        Disconnect() const { VISERA_WIP; return False; }

    private:
        TList<std::function<void()>> Slots;
    };
}
