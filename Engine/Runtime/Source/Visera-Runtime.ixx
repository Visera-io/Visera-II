module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime;
#define VISERA_MODULE_NAME "Runtime"
export import Visera.Runtime.AssetHub;
export import Visera.Runtime.Audio;
export import Visera.Runtime.Graphics;
export import Visera.Runtime.Input;
export import Visera.Runtime.RHI;
export import Visera.Runtime.UI;
export import Visera.Runtime.Window;
export import Visera.Runtime.Scripting;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.Optional;
       import Visera.Core.OS.Time;
       import Visera.Core.OS.Thread;
       import Visera.Core.Delegate;
       import Visera.Core.Log;

namespace Visera
{
    /** Engine creation parameters. Each TOptional that has a value enables that service; empty = disabled. */
    export struct VISERA_RUNTIME_API FEngineCreateInfo
    {
        FString Name = "Visera";
        UInt32  MaxFrameRate = 0;

        TOptional<FWindowCreateInfo>    MainWindow;
        TOptional<FRHICreateInfo>       RHI;
        TOptional<FAssetHubCreateInfo>  AssetHub;
        TOptional<FAudioCreateInfo>     AudioEngine;
        TOptional<FGraphicsCreateInfo>  Graphics;
        TOptional<FInputCreateInfo>     Input;
        TOptional<FUICreateInfo>        UI;
        TOptional<FScriptingCreateInfo> Scripting;
    };

    /** Engine mode: Standard = full features, Forge = AssetHub only (batch, no Run). */
    export enum class EEngineMode
    {
        Standard,
        Forge,
    };

    export class VISERA_RUNTIME_API FViseraEngine;

    class VISERA_RUNTIME_API FViseraEngine
    {
    public:
        explicit FViseraEngine(const FEngineCreateInfo& I_CreateInfo);
        ~FViseraEngine();

        void Run();
        [[nodiscard]] Bool IsRunning() const { return bRunning; }
        void RequestExit() { bRunning = False; }

        [[nodiscard]] FInput*     GetInput()     const { return Input.Get(); }
        [[nodiscard]] FRHI*       GetRHI()       const { return RHI.Get(); }
        [[nodiscard]] FAssetHub*  GetAssetHub()  const { return AssetHub.Get(); }
        [[nodiscard]] FAudio*     GetAudio()     const { return Audio.Get(); }
        [[nodiscard]] FGraphics*  GetGraphics()  const { return Graphics.Get(); }
        [[nodiscard]] FWindow*    GetWindow()    const { return Window.Get(); }
        [[nodiscard]] FUI*        GetUI()        const { return UI.Get(); }
        [[nodiscard]] FScripting* GetScripting()   const { return Scripting.Get(); }

        TMulticastDelegate<Double> OnTick;
        TMulticastDelegate<>       OnPreRender;
        TMulticastDelegate<>       OnPostRender;

    private:
        void CreateServices(const FEngineCreateInfo& I_CreateInfo);
        void DestroyServices();
        void ApplyFramePacing(FHighResTimePoint I_FrameStart);

        FString     Name;
        UInt32      MaxFrameRate = 0;
        FHiResClock FrameClock;
        Bool        bRunning = False;

        TUniquePtr<FInput>     Input;
        TUniquePtr<FRHI>       RHI;
        TUniquePtr<FAssetHub>  AssetHub;
        TUniquePtr<FAudio>     Audio;
        TUniquePtr<FGraphics>  Graphics;
        TUniquePtr<FWindow>    Window;
        TUniquePtr<FUI>        UI;
        TUniquePtr<FScripting> Scripting;

        FViseraEngine(const FViseraEngine&)            = delete;
        FViseraEngine& operator=(const FViseraEngine&) = delete;
        FViseraEngine(FViseraEngine&&)                 = delete;
        FViseraEngine& operator=(FViseraEngine&&)      = delete;
    };

    FViseraEngine::FViseraEngine(const FEngineCreateInfo& I_CreateInfo)
        : Name(I_CreateInfo.Name)
        , MaxFrameRate(I_CreateInfo.MaxFrameRate)
    {
        CreateServices(I_CreateInfo);
    }

    FViseraEngine::~FViseraEngine()
    {
        DestroyServices();
    }

    void FViseraEngine::CreateServices(const FEngineCreateInfo& I_CreateInfo)
    {
        if (I_CreateInfo.Input.HasValue())
        { Input = MakeUnique<FInput>(I_CreateInfo.Input.GetValue()); }

        if (I_CreateInfo.RHI.HasValue())
        { RHI = MakeUnique<FRHI>(I_CreateInfo.RHI.GetValue()); }

        if (I_CreateInfo.AssetHub.HasValue())
        {
            AssetHub = MakeUnique<FAssetHub>(I_CreateInfo.AssetHub.GetValue());
            GAssetHub = AssetHub.Get();
        }

        if (I_CreateInfo.AudioEngine.HasValue())
        { Audio = MakeUnique<FAudio>(I_CreateInfo.AudioEngine.GetValue()); }

        if (I_CreateInfo.Graphics.HasValue())
        {
            FGraphicsCreateInfo GfxInfo = I_CreateInfo.Graphics.GetValue();
            if (RHI && AssetHub)
            {
                GfxInfo.RHI = RHI.Get();
                GfxInfo.AssetHub = AssetHub.Get();
                Graphics = MakeUnique<FGraphics>(GfxInfo);
            }
        }

        if (I_CreateInfo.MainWindow.HasValue())
        {
            FInput* InputPtr = Input.Get();
            Window = MakeUnique<FWindow>(I_CreateInfo.MainWindow.GetValue(), InputPtr);
        }

        if (I_CreateInfo.UI.HasValue() && Window && Graphics && Input)
        { UI = MakeUnique<FUI>(I_CreateInfo.UI.GetValue()); }

        if (I_CreateInfo.Scripting.HasValue() && Graphics && AssetHub)
        { Scripting = MakeUnique<FScripting>(I_CreateInfo.Scripting.GetValue(), Graphics.Get(), AssetHub.Get()); }
    }

    void FViseraEngine::DestroyServices()
    {
        Scripting.Reset();
        UI.Reset();
        Window.Reset();
        Graphics.Reset();
        Audio.Reset();
        GAssetHub = nullptr;
        AssetHub.Reset();
        RHI.Reset();
        Input.Reset();
    }

    void FViseraEngine::ApplyFramePacing(FHighResTimePoint I_FrameStart)
    {
        if (MaxFrameRate == 0) { return; }
        UInt32 const TargetMs = 1000 / MaxFrameRate;
        auto Elapsed = FrameClock.Now() - I_FrameStart;
        UInt32 ElapsedMs = static_cast<UInt32>(Elapsed.Milliseconds());
        if (ElapsedMs < TargetMs)
        { FThread::Sleep(TargetMs - ElapsedMs); }
    }

    void FViseraEngine::Run()
    {
        if (!Window)
        { LOG_WARN("({}) Run() called but no window; exiting immediately.", Name); return; }

        bRunning = True;
        while (bRunning && !Window->ShouldClose())
        {
            auto FrameStart = FrameClock.Now();

            if (Input) { Input->PollAndSync(); }
            Double Dt = FrameClock.Tick().Seconds();

            OnTick.Broadcast(Dt);
            if (Scripting)
            { Scripting->Tick(Dt); }

            OnPreRender.Broadcast();
            
            if (Graphics && Window)
            { Graphics->Render(Window.Get()); }
            OnPostRender.Broadcast();

            ApplyFramePacing(FrameStart);
        }
        bRunning = False;
    }
}
