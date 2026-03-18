module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Input.Action;
#define VISERA_MODULE_NAME "Runtime.Input"
import Visera.Core.Delegate.Multicast;
import Visera.Core.Types.Name;

export namespace Visera
{
    /**
     * Abstract input action (UE5: UInputAction).
     * Subscribe to OnTriggered to respond when a mapping fires.
     * Callback receives the action pointer for convenience.
     */
    class VISERA_RUNTIME_API FInputAction
    {
    public:
        using FTriggeredDelegate = TMulticastDelegate<FInputAction*>;

        explicit FInputAction(FName I_ActionName) : ActionName(I_ActionName) {}

        [[nodiscard]] const FName& GetName() const { return ActionName; }
        FTriggeredDelegate OnTriggered;

    private:
        FName ActionName;
    };
}
