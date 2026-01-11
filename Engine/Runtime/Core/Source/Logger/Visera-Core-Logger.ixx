module;
#include <Visera-Core.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
export module Visera.Core.Logger;
#define VISERA_MODULE_NAME "Core.Logger"
import Visera.Core.OS.Time;

namespace Visera
{
	export enum class ELogLevel : UInt8
	{
		Trace   = spdlog::level::level_enum::trace,
		Debug   = spdlog::level::level_enum::debug,
		Info    = spdlog::level::level_enum::info,
		Warn    = spdlog::level::level_enum::warn,
		Error	= spdlog::level::level_enum::err,
		Fatal	= spdlog::level::level_enum::critical,
	};

	export class VISERA_CORE_API FLogger
	{
	public:
		template<typename... Args> inline void
		Trace(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > ELogLevel::Trace) { return; }

			if (Backend)
			{ Backend->trace(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('T', stdout, I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args> inline void
		Debug(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > ELogLevel::Debug) { return; }

			if (Backend)
			{ Backend->debug(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('D', stdout, I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args> inline void
		Info(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > ELogLevel::Info) { return; }

			if (Backend)
			{ Backend->info(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('I', stdout, I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args> inline void
		Warn(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > ELogLevel::Warn) { return; }

			if (Backend)
			{ Backend->warn(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('W', stdout, I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args> inline void
		Error(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > ELogLevel::Error) { return; }

			if (Backend)
			{ Backend->error(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('E', stderr, I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args> inline void
		Fatal(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Backend)
			{
				Backend->critical(I_Fmt, std::forward<Args>(I_Args)...);
				Backend->flush();
			}
			else
			{
				NativeLog('C', stderr, I_Fmt, std::forward<Args>(I_Args)...);
				std::fflush(stderr);
			}
			std::abort();
		}

		void inline
		SetLevel(ELogLevel I_Level)
		{
			Level = I_Level;
			if (Backend)
			{ Backend->set_level(static_cast<spdlog::level::level_enum>(Level)); }
		}

	protected:
		TUniquePtr<spdlog::logger> Backend;
		ELogLevel Level = ELogLevel::Trace;

		template<typename... Args>
		inline void
		NativeLog(const char I_Level, decltype(stdout) I_Stream, spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			const auto Now = FSystemClock::Now();
			const auto Time = Now.ToSystemTimeType();

 			std::tm LocalTime{};
 #if defined(VISERA_ON_WINDOWS_SYSTEM)
 			localtime_s(&LocalTime, &Time);
 #else
 			localtime_r(&Time, &LocalTime);
 #endif
 			auto ThreadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
 			auto Message = Format(I_Fmt, std::forward<Args>(I_Args)...);

 			fmt::println(I_Stream, "[{}] [{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}] [T:{}] {}",
 						 I_Level,
 						 LocalTime.tm_year + 1900, LocalTime.tm_mon + 1, LocalTime.tm_mday,
 						 LocalTime.tm_hour, LocalTime.tm_min, LocalTime.tm_sec, Now.GetTimeFromEpoch().Milliseconds(),
 						 ThreadID, Message);
		}

	public:
		FLogger(ELogLevel I_Level = ELogLevel::Trace);
		~FLogger();
	};
	
	FLogger::
	FLogger(ELogLevel I_Level /* = ELogLevel::Trace */)
	: Level(I_Level)
	{
		auto ConsoleSink = MakeShared<spdlog::sinks::stdout_color_sink_mt>();

		// auto RotatingSink = MakeShared<spdlog::sinks::rotating_file_sink_mt>(
		// 	"/Logs/Engine.log", //VISERA_APP_CACHE_DIR
		// 	5 * 1024, // N MB per file,
		// 	1         // keep N backups
		// );
		Backend = MakeUnique<spdlog::logger>("System Log",
			spdlog::sinks_init_list{ ConsoleSink /*, RotatingSink*/ });

		Backend->set_pattern("%^[%L] [%Y-%m-%d %H:%M:%S.%e] [T:%t] %v%$");
		//Backend->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%L] %v%$");
		SetLevel(Level);
	}

	FLogger::
	~FLogger()
	{
		Backend->flush();
		//Do not call drop_all() in your class! spdlog::drop_all();
		Backend.reset();
	}
}