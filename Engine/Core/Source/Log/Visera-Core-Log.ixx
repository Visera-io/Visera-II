module;
#include <Visera-Core.hpp>
export module Visera.Core.Log;
#define VISERA_MODULE_NAME "Core.Log"
import Visera.Core.Log.Logger;
import Visera.Core.Types.Path;

export namespace Visera
{
    using ELogLevel = FLogger::ELevel;

    class VISERA_CORE_API FLog
    {
    public:
        static inline auto&
        Get() { static FLogger Logger{}; return Logger; }

        /** Sets the log file sink directory (e.g. platform Logs path). No file is created until this is called. */
        static inline void
        SetSinkPath(const FPath& I_LogDirectory) { Get().SetSinkPath(I_LogDirectory); }
    };

    static_assert(ELogLevel::Trace == static_cast<ELogLevel>(VISERA_LOG_LEVEL_TRACE));
    static_assert(ELogLevel::Debug == static_cast<ELogLevel>(VISERA_LOG_LEVEL_DEBUG));
    static_assert(ELogLevel::Info  == static_cast<ELogLevel>(VISERA_LOG_LEVEL_INFO));
    static_assert(ELogLevel::Warn  == static_cast<ELogLevel>(VISERA_LOG_LEVEL_WARN));
    static_assert(ELogLevel::Error == static_cast<ELogLevel>(VISERA_LOG_LEVEL_ERROR));
    static_assert(ELogLevel::Fatal == static_cast<ELogLevel>(VISERA_LOG_LEVEL_FATAL));
}