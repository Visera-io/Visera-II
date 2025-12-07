module;
#include <Visera-Studio.hpp>
#if !defined(VISERA_OFFSCREEN_MODE)
#include <imgui.h>
#endif
export module Visera.Studio.UI;
#define VISERA_MODULE_NAME "Studio.UI"
import Visera.Core.Log;
import Visera.Studio.UI.ImGui;

namespace Visera
{
    export class VISERA_STUDIO_API FStudioUI
    {
    public:
        struct VISERA_STUDIO_API FWindow
        {
            Bool Status = False;
            FWindow() = delete;
            VISERA_NOINLINE
            FWindow(FStringView I_Title) { Status = ImGui::Begin(I_Title.data()); }
            VISERA_NOINLINE
            ~FWindow() { ImGui::End(); }
            [[nodiscard]] explicit
            operator Bool() const { return Status; }
        };
        VISERA_NOINLINE [[nodiscard]] FWindow
        Window(FStringView I_Title) { return FWindow{I_Title};  }
        VISERA_NOINLINE void
        Text(FStringView I_Text) { ImGui::TextUnformatted(I_Text.data()); }
        VISERA_NOINLINE Bool
        Button(FStringView I_Label) { return ImGui::Button(I_Label.data()); }
        VISERA_NOINLINE Bool
        Slider(FStringView I_Label, TMutable<Float> I_Value, Float I_Min, Float I_Max) { return ImGui::SliderFloat(I_Label.data(), I_Value, I_Min, I_Max); }

        VISERA_FORCEINLINE void
        BeginFrame() { ImGuiContext.BeginFrame(); if (auto W = Window("Hi Kiki")) { Text("??"); }}
        VISERA_FORCEINLINE void
        EndFrame() { ImGuiContext.EndFrame(); }

    private:
        FImGui ImGuiContext;
    };
}