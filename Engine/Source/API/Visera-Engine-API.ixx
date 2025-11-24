module;
#include <Visera-Engine.hpp>
export module Visera.Engine.API;
#define VISERA_MODULE_NAME "Engine.API"
import Visera.Core.Log;
import Visera.Runtime.Input;
import Visera.Runtime.Window;
import Visera.Engine.AssetHub;

#if defined(VISERA_ON_WINDOWS_SYSTEM)
#define VISERA_APP_CALLABLE VISERA_ENGINE_API auto __cdecl
#else
#define VISERA_APP_CALLABLE VISERA_ENGINE_API auto
#endif

extern "C"
{
export namespace Visera::API
{
    VISERA_APP_CALLABLE
    Print(ELogLevel I_Level, const char* I_Module, const char* I_Message) -> void
    {
        switch (I_Level)
        {
        case ELogLevel::Trace: GLog->Trace ("[M:App.{}] {}", I_Module, I_Message); break;
        case ELogLevel::Debug: GLog->Debug ("[M:App.{}] {}", I_Module, I_Message); break;
        case ELogLevel::Info:  GLog->Info  ("[M:App.{}] {}", I_Module, I_Message); break;
        case ELogLevel::Warn:  GLog->Warn  ("[M:App.{}] {}", I_Module, I_Message); break;
        case ELogLevel::Error: GLog->Error ("[M:App.{}] {}", I_Module, I_Message); break;
        case ELogLevel::Fatal: GLog->Fatal ("[M:App.{}] {}", I_Module, I_Message); break;
        }
    }

    VISERA_APP_CALLABLE
    SetWindowTitle(const char* I_Title) -> void
    {
        GWindow->SetTitle(I_Title);
    }

    VISERA_APP_CALLABLE
    ResizeWindow(Int32 I_Width, Int32 I_Height) -> void
    {
        GWindow->SetSize(I_Width, I_Height);
    }

    VISERA_APP_CALLABLE
    LoadShader(const char* I_Path, const char* I_EntryPoint) -> Bool
    {
        VISERA_ASSERT(I_Path && I_EntryPoint);
        auto Shader = GAssetHub->LoadShader(FPath{I_Path}, I_EntryPoint);
        return Shader != nullptr;
    }

    VISERA_APP_CALLABLE
    LoadTexture(const char* I_Path) -> Bool
    {
        VISERA_ASSERT(I_Path);
        auto Texture = GAssetHub->LoadTexture(FPath{I_Path});
        return Texture != nullptr;
    }

    VISERA_APP_CALLABLE
    IsPressed(UInt32 I_Key) -> Bool
    {
        if(I_Key > FKeyboard::MaxKey) { return False; }
        return GInput->GetKeyboard()->IsPressed(static_cast<FKeyboard::EKey>(I_Key));
    }
}
}