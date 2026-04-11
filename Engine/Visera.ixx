module;
#include <Visera-Core.hpp>
export module Visera;
#define VISERA_MODULE_NAME "Visera"
export import Visera.Core;
export import Visera.Runtime;
export import Visera.Platform;

export namespace Visera
{
    /** Create engine from FEngineCreateInfo. Each TOptional with a value enables that service. */
    [[nodiscard]] inline TUniquePtr<FViseraEngine>
    CreateEngine(const FEngineCreateInfo& I_CreateInfo)
    {
        return MakeUnique<FViseraEngine>(I_CreateInfo);
    }

    /** Create engine by mode. Standard = full stack (headless until script OnInit calls setMainWindow or you set FEngineCreateInfo.MainWindow). Forge = AssetHub only (no Run loop). */
    [[nodiscard]] inline TUniquePtr<FViseraEngine>
    CreateEngine(EEngineMode I_Mode)
    {
        FEngineCreateInfo Info;
        Info.Name = "Visera";
        Info.MaxFrameRate = (I_Mode == EEngineMode::Standard) ? 60u : 0u;

        switch (I_Mode)
        {
        case EEngineMode::Standard:
            Info.RHI         = FRHICreateInfo{};
            Info.AssetHub    = FAssetHubCreateInfo{};
            Info.AudioEngine = FAudioCreateInfo
            {
                .Engine       = "Wwise",
                .BankBasePath = VPath{"@assets://soundbanks"},
            };
            Info.Graphics    = FGraphicsCreateInfo{};
            Info.Input       = FInputCreateInfo{};
            Info.Scripting   = FScriptingCreateInfo{};
            break;
        case EEngineMode::Forge:
            Info.AssetHub = FAssetHubCreateInfo{};
            break;
        default:
            LOG_FATAL("Unknown EEngineMode: {}", static_cast<Int32>(I_Mode));
        }
        return CreateEngine(Info);
    }
}
