module;
#include <Visera.hpp>
export module Visera;
#define VISERA_MODULE_NAME "Visera"
import Visera.Global.Log;
import Visera.Platform;
import Visera.Core.Logger;
import Visera.Global.Log;

#if defined(VISERA_ON_WINDOWS_SYSTEM)
#define VISERA_API __declspec(dllexport) auto __cdecl
#else
#define VISERA_API __attribute__((visibility("default"))) auto
#endif

extern "C"
{
export namespace Visera::API
{
    VISERA_API
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

    VISERA_API
    SetWindowTitle(const char* I_Title) -> void
    {
        GWindow->SetTitle(I_Title);
    }

    VISERA_API
    ResizeWindow(Int32 I_Width, Int32 I_Height) -> void
    {
        GWindow->SetSize(I_Width, I_Height);
    }
}
} // extern "C"