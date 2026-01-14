module;
#include "../Include/Visera.hpp"
export module Visera;
#define VISERA_MODULE_NAME "Visera"
import Visera.Global.Log;
import Visera.Platform;
import Visera.Core.Logger;

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
        case ELogLevel::Trace: FLog::Get().Trace ("[M:App.{}] {}", I_Module, I_Message); break;
        case ELogLevel::Debug: FLog::Get().Debug ("[M:App.{}] {}", I_Module, I_Message); break;
        case ELogLevel::Info:  FLog::Get().Info  ("[M:App.{}] {}", I_Module, I_Message); break;
        case ELogLevel::Warn:  FLog::Get().Warn  ("[M:App.{}] {}", I_Module, I_Message); break;
        case ELogLevel::Error: FLog::Get().Error ("[M:App.{}] {}", I_Module, I_Message); break;
        case ELogLevel::Fatal: FLog::Get().Fatal ("[M:App.{}] {}", I_Module, I_Message); break;
        }
    }
}
} // extern "C"