module;
#include <Visera-Engine.hpp>
export module Visera.Engine.API;
#define VISERA_MODULE_NAME "Engine.API"
import Visera.Core.Log;
import Visera.Runtime.Window;

#define VISERA_APP_CALLABLE export VISERA_ENGINE_API auto __cdecl

extern "C"
{
namespace Visera
{
    VISERA_APP_CALLABLE
    Print(ELogLevel I_Level, const char* I_Module, const char* I_Message)
    {
        switch (I_Level)
        {
        case ELogLevel::Trace: GLog->Trace ("[M:{}] {}", I_Module, I_Message); break;
        case ELogLevel::Debug: GLog->Debug ("[M:{}] {}", I_Module, I_Message); break;
        case ELogLevel::Info:  GLog->Info  ("[M:{}] {}", I_Module, I_Message); break;
        case ELogLevel::Warn:  GLog->Warn  ("[M:{}] {}", I_Module, I_Message); break;
        case ELogLevel::Error: GLog->Error ("[M:{}] {}", I_Module, I_Message); break;
        case ELogLevel::Fatal: GLog->Fatal ("[M:{}] {}", I_Module, I_Message); break;
        }
    }

    VISERA_APP_CALLABLE
    SetWindowTitle(const char* I_Title)
    {
        GWindow->SetTitle(I_Title);
    }

    VISERA_APP_CALLABLE
    ResizeWindow(Int32 I_Width, Int32 I_Height)
    {
        GWindow->SetSize(I_Width, I_Height);
    }
}
}