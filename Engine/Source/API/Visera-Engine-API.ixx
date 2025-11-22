module;
#include <Visera-Engine.hpp>
export module Visera.Engine.API;
#define VISERA_MODULE_NAME "Engine.API"
import Visera.Core;
import Visera.Runtime.Input;
import Visera.Runtime.Window;
import Visera.Engine.AssetHub;

#if defined(VISERA_ON_WINDOWS_SYSTEM)
#define VISERA_APP_CALLABLE VISERA_ENGINE_API __cdecl
#else
#define VISERA_APP_CALLABLE VISERA_ENGINE_API
#endif

extern "C"
{
export namespace Visera::API
{
    VISERA_APP_CALLABLE void
    Print(ELogLevel I_Level, const char* I_Module, const char* I_Message)
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

    VISERA_APP_CALLABLE UInt64
    CityHash64(const char* I_Data)
    {
        return Visera::CityHash64(I_Data);
    }

    VISERA_APP_CALLABLE void
    SetWindowTitle(const char* I_Title)
    {
        GWindow->SetTitle(I_Title);
    }

    VISERA_APP_CALLABLE void
    ResizeWindow(Int32 I_Width, Int32 I_Height)
    {
        GWindow->SetSize(I_Width, I_Height);
    }

    VISERA_APP_CALLABLE Bool
    LoadShader(const char* I_Path, const char* I_EntryPoint)
    {
        VISERA_ASSERT(I_Path && I_EntryPoint);
        auto Shader = GAssetHub->LoadShader(FPath{I_Path}, I_EntryPoint);
        return Shader != nullptr;
    }

    VISERA_APP_CALLABLE Bool
    LoadTexture(const char* I_Path)
    {
        VISERA_ASSERT(I_Path);
        auto Texture = GAssetHub->LoadTexture(FPath{I_Path});
        return Texture != nullptr;
    }

    VISERA_APP_CALLABLE Bool
    IsPressed(UInt32 I_Key)
    {
        if(I_Key > FKeyboard::MaxKey) { return False; }
        return GInput->GetKeyboard()->IsPressed(static_cast<FKeyboard::EKey>(I_Key));
    }
}
}