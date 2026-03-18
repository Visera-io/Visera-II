module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.UI;
#define VISERA_MODULE_NAME "Runtime.UI"
export import Visera.Runtime.UI.Context;
       import Visera.Runtime.Window;
       import Visera.Runtime.Graphics;
       import Visera.Runtime.Input;
       import Visera.Core.Log;
       import Visera.Core.Containers.Map;

export namespace Visera
{
    /** CreateInfo for FUI. Presence in FEngineCreateInfo enables the service; dependencies are passed at construction. */
    struct VISERA_RUNTIME_API FUICreateInfo
    {
    };

    class VISERA_RUNTIME_API FUI
    {
    public:
        explicit FUI(const FUICreateInfo& I_CreateInfo = FUICreateInfo{});
        ~FUI();

    public:
        /** Bind a context to a layer for multi-layer UI (e.g. Debug = FImGUIContext, InGame = FGameUIContext). */
        void
        Begin(ELayer I_Layer, IUIContext* I_Context)
        {
            if (I_Context)
            { Contexts.Insert(I_Layer, I_Context); }
        }

        [[nodiscard]] IUIContext*
        GetContext(ELayer I_Layer) const
        {
            auto it = Contexts.Find(I_Layer);
            return it != Contexts.end() ? it->second : nullptr;
        }

    private:
        TMap<ELayer, IUIContext*> Contexts;
    };

    FUI::FUI(const FUICreateInfo& I_CreateInfo)
    {
        (void)I_CreateInfo;
    }

    FUI::~FUI()
    {
        Contexts.Clear();
    }
}
