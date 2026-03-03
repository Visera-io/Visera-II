module;
#include <Visera-UI.hpp>
export module Visera.Runtime.UI;
#define VISERA_MODULE_NAME "Runtime.UI"
export import Visera.Runtime.UI.Context;
       import Visera.Runtime.Global;
       import Visera.Runtime.Window;
       import Visera.Runtime.Graphics;
       import Visera.Runtime.Input;
       import Visera.Core.Log;
       import Visera.Core.Containers.Map;

export namespace Visera
{
    class VISERA_RUNTIME_API FUI : public IRuntimeService
    {
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

    public:
        FUI(FString I_Name, FServiceRegistry* I_Registry, FJSONView I_ConfigView,
            TMulticastDelegate<const FJSONRoute&>* I_OnConfigChange, FStringView I_RuntimeName)
            : IRuntimeService(I_Name, I_Registry, std::move(I_ConfigView), I_OnConfigChange, I_RuntimeName)
        {
            Dependencies =
            {
                EService::Window,
                EService::Graphics,
                EService::Input,
            };

            if (!OnBootstrap.TryBind([this]
            {
                if (!GetService<FWindow>(EService::Window).Lock())
                { LOG_FATAL("FUI: failed to get Window!"); return False; }
                if (!GetService<FGraphics>(EService::Graphics).Lock())
                { LOG_FATAL("FUI: failed to get Graphics!"); return False; }
                if (!GetService<FInput>(EService::Input).Lock())
                { LOG_FATAL("FUI: failed to get Input!"); return False; }
                return True;
            }))
            { LOG_FATAL("FUI: failed to bind OnBootstrap!"); }

            if (!OnTerminate.TryBind([this]
            {
                Contexts.Clear();
                return True;
            }))
            { LOG_FATAL("FUI: failed to bind OnTerminate!"); }
        }
    };
}
