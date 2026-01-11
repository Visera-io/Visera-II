module;
#include <Visera-Global.hpp>
export module Visera.Global.Log;
#define VISERA_MODULE_NAME "Global.Log"
import Visera.Core.Logger;

namespace Visera
{
    export class VISERA_GLOBAL_API FLog
    {

    };

    export inline VISERA_GLOBAL_API TUniquePtr<FLogger>
    GLog = MakeUnique<FLogger>(static_cast<ELogLevel>(VISERA_LOG_SYSTEM_VERBOSITY)); //[TODO]: class FLog

    static_assert(ELogLevel::Trace == static_cast<ELogLevel>(VISERA_LOG_LEVEL_TRACE));
    static_assert(ELogLevel::Debug == static_cast<ELogLevel>(VISERA_LOG_LEVEL_DEBUG));
    static_assert(ELogLevel::Info  == static_cast<ELogLevel>(VISERA_LOG_LEVEL_INFO));
    static_assert(ELogLevel::Warn  == static_cast<ELogLevel>(VISERA_LOG_LEVEL_WARN));
    static_assert(ELogLevel::Error == static_cast<ELogLevel>(VISERA_LOG_LEVEL_ERROR));
    static_assert(ELogLevel::Fatal == static_cast<ELogLevel>(VISERA_LOG_LEVEL_FATAL));
}