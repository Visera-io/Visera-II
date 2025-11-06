module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Scripting.API;
#define VISERA_MODULE_NAME "Engine.Scripting"
import Visera.Core.Log;
import Visera.Runtime.Window;

#define CAPI(I_RetType) VISERA_ENGINE_API I_RetType __cdecl

extern "C"
{
export namespace Visera
{
    CAPI(void)
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

    CAPI(void)
    Fatal(const char* I_Message)\
    { LOG_FATAL("{}", I_Message); }

    CAPI(void)
    SetWindowTitle(const char* I_Title)
    {
        GWindow->SetTitle(I_Title);
    }
}
}