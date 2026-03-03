module;
#include <Visera-UI.hpp>
export module Visera.Runtime.UI.Context;
#define VISERA_MODULE_NAME "Runtime.UI.Context"

export namespace Visera
{
    /** UI layer for multi-layer UI; FUI::Begin(ELayer, IUIContext) binds a context to a layer. */
    enum class VISERA_RUNTIME_API ELayer : UInt8
    {
        Debug,   // DebugUI (e.g. FImGUIContext)
        InGame,  // Game UI (e.g. future FGameUIContext)
        Overlay,
    };

    /** Abstract UI context; FImGUIContext and future FGameUIContext implement this. */
    class VISERA_RUNTIME_API IUIContext
    {
    public:
        virtual void BeginFrame() = 0;
        virtual void EndFrame()   = 0;
        virtual ~IUIContext()     = default;
    };
}
