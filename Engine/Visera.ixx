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

    /** Create engine by mode. Standard = full features with main window; Forge = AssetHub only (no Run loop). */
    [[nodiscard]] inline TUniquePtr<FViseraEngine>
    CreateEngine(EEngineMode I_Mode, FString I_WindowTitle = "Visera", UInt32 I_Width = 1024, UInt32 I_Height = 768)
    {
        FEngineCreateInfo Info;
        Info.Name = "Visera";
        Info.MaxFrameRate = (I_Mode == EEngineMode::Standard) ? 60u : 0u;

        switch (I_Mode)
        {
        case EEngineMode::Standard:
            Info.MainWindow  = FWindowCreateInfo{ .Title = std::move(I_WindowTitle), .Width = I_Width, .Height = I_Height };
            Info.RHI         = FRHICreateInfo{};
            Info.AssetHub    = FAssetHubCreateInfo{};
            Info.AudioEngine = FAudioCreateInfo{ .Engine = "Wwise" };
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
