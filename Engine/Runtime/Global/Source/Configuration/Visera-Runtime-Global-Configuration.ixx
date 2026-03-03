module;
#include <Visera-Global.hpp>
export module Visera.Runtime.Global.Configuration;
#define VISERA_MODULE_NAME "Runtime.Global"
export import Visera.Core.Types.JSON;
export import Visera.Core.Delegate;
       import Visera.Core.Types.String;

export namespace Visera
{
    inline constexpr FStringView kConfigKeyEngine = "Engine";
    inline constexpr FStringView kConfigKeyApps   = "Apps";

    /** Engine config: root JSON with Engine/Apps structure. */
    class VISERA_RUNTIME_API FEngineConfig
    {
    public:
        TMulticastDelegate<const FJSONRoute&>
        OnEngineConfigChange;

        [[nodiscard]] FJSON&       GetRoot()       { return Root; }
        [[nodiscard]] const FJSON& GetRoot() const { return Root; }

        /** Engine-scoped config view (prefix "Engine"). For global services. */
        [[nodiscard]] FJSONView GetEngineConfig()
        {
            return FJSONView(Root, kConfigKeyEngine);
        }

        /** App-scoped config view (prefix "Apps.<AppName>"). For local services. */
        [[nodiscard]] FJSONView GetAppConfig(FStringView I_AppName)
        {
            return FJSONView(Root, FString::Format("{}.{}", kConfigKeyApps, I_AppName));
        }

        explicit FEngineConfig(const FJSON& I_Initial)
            : Root(I_Initial)
        {
            if (!Root.Contains(kConfigKeyEngine)) { Root.Set(kConfigKeyEngine, FJSON{}); }
            if (!Root.Contains(kConfigKeyApps))   { Root.Set(kConfigKeyApps,   FJSON{}); }
        }

        FEngineConfig()
        {
            Root.Set(kConfigKeyEngine, FJSON{});
            Root.Set(kConfigKeyApps,   FJSON{});
        }

    private:
        FJSON Root;
    };
}
