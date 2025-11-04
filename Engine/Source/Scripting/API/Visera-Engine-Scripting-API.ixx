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
    Log(const char* I_Message)
    {
        LOG_WARN("{}", I_Message);
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