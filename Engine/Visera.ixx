module;
#include <Visera-Core.hpp>
export module Visera;
#define VISERA_MODULE_NAME "Visera"
export import Visera.Core;
export import Visera.Runtime;
export import Visera.Platform;

export namespace Visera
{
    /** Create engine from Config. Services enabled via Engine.<Service>.Enable (default false when missing). */
    [[nodiscard]] inline TUniquePtr<FViseraEngine>
    CreateEngine(const FJSON& I_Config)
    {
        return MakeUnique<FViseraEngine>(I_Config);
    }

    /** Create engine by mode. No Config needed. Standard = full; Forge = Task, AssetHub, RHI only (Visera-Forge). */
    [[nodiscard]] inline TUniquePtr<FViseraEngine>
    CreateEngine(EEngineMode I_Mode)
    {
        FJSON Config{};
        switch (I_Mode)
        {
        case EEngineMode::Standard:
            Config.Set(TJSONRoute<"Engine.Tasks.Enable">(),    True);
            Config.Set(TJSONRoute<"Engine.RHI.Enable">(),      True);
            Config.Set(TJSONRoute<"Engine.Audio.Enable">(),    True);
            Config.Set(TJSONRoute<"Engine.Audio.Engine">(),    "Wwise");
            Config.Set(TJSONRoute<"Engine.AssetHub.Enable">(), True);
            Config.Set(TJSONRoute<"Engine.Graphics.Enable">(), True);
            break;
        case EEngineMode::Forge:
            Config.Set(TJSONRoute<"Engine.Tasks.Enable">(),    True);
            Config.Set(TJSONRoute<"Engine.RHI.Enable">(),      True);
            Config.Set(TJSONRoute<"Engine.AssetHub.Enable">(), True);
            break;
        default:
            LOG_FATAL("Unknown EEngineMode: {}", static_cast<Int32>(I_Mode));
        }
        return CreateEngine(Config);
    }
}
