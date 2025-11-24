module;
#include <Visera-Core.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
export module Visera.Core.Log;
#define VISERA_MODULE_NAME "Core.Log"
import Visera.Core.Global;
import Visera.Core.OS.Time;

namespace Visera
{
	export enum class ELogLevel : UInt8
	{
		Trace   = VISERA_LOG_LEVEL_TRACE,
		Debug   = VISERA_LOG_LEVEL_DEBUG,
		Info    = VISERA_LOG_LEVEL_INFO,
		Warn    = VISERA_LOG_LEVEL_WARN,
		Error	= VISERA_LOG_LEVEL_ERROR,
		Fatal	= VISERA_LOG_LEVEL_FATAL,
	};

	export class VISERA_CORE_API FLog : public IGlobalSingleton<FLog>
	{
	public:
		template<typename... Args> inline void
		Trace(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > VISERA_LOG_LEVEL_TRACE) { return; }

			if (IsBootstrapped())
			{ Logger->trace(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('T', stdout, I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args> inline void
		Debug(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > VISERA_LOG_LEVEL_DEBUG) { return; }

			if (IsBootstrapped())
			{ Logger->debug(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('D', stdout, I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args> inline void
		Info(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > VISERA_LOG_LEVEL_INFO) { return; }

			if (IsBootstrapped())
			{ Logger->info(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('I', stdout, I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args> inline void
		Warn(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > VISERA_LOG_LEVEL_WARN) { return; }

			if (IsBootstrapped())
			{ Logger->warn(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('W', stdout, I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args> inline void
		Error(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > VISERA_LOG_LEVEL_ERROR) { return; }

			if (IsBootstrapped())
			{ Logger->error(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('E', stderr, I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args> inline void
		Fatal(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (IsBootstrapped())
			{
				Logger->critical(I_Fmt, std::forward<Args>(I_Args)...);
				Logger->flush();
			}
			else
			{
				NativeLog('C', stderr, I_Fmt, std::forward<Args>(I_Args)...);
				std::fflush(stderr);
			}
			std::abort();
		}

		void inline
		SetLevel(ELogLevel I_Level) { Level = static_cast<UInt8>(I_Level); }

	protected:
		TUniquePtr<spdlog::logger> Logger;
		UInt8 Level = VISERA_LOG_SYSTEM_VERBOSITY;

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
		FLog() : IGlobalSingleton{"Log"} {}
		~FLog() noexcept override { Logger.reset(); };
		void inline
		Bootstrap() override;
		void inline
		Terminate() override;
	};

	export inline VISERA_CORE_API TUniquePtr<FLog>
	GLog = MakeUnique<FLog>();

	void FLog::
	Bootstrap()
	{
		if (IsBootstrapped()) { return;}
		LOG_TRACE("Bootstrapping Log.");

		auto ConsoleSink = MakeShared<spdlog::sinks::stdout_color_sink_mt>();

		// auto RotatingSink = MakeShared<spdlog::sinks::rotating_file_sink_mt>(
		// 	"/Logs/Engine.log", //VISERA_APP_CACHE_DIR
		// 	5 * 1024, // N MB per file,
		// 	1         // keep N backups
		// );
		TArray<spdlog::sink_ptr> Sinks
		{
			ConsoleSink,
			//RotatingSink,
		};

		Logger = MakeUnique<spdlog::logger>("System Log", Sinks.begin(), Sinks.end());

		Logger->set_level(spdlog::level::level_enum(VISERA_LOG_SYSTEM_VERBOSITY));
		Logger->set_pattern("%^[%L] [%Y-%m-%d %H:%M:%S.%e] [T:%t] %v%$");
		//Logger->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%L] %v%$");

		Status = EStatus::Bootstrapped;
	}

	void FLog::
	Terminate()
	{
		if (!IsBootstrapped()) { return;}
		LOG_TRACE("Terminating Log.");

		Logger->flush();
		//Do not call drop_all() in your class! spdlog::drop_all();
		Logger.reset();

		Status = EStatus::Terminated;
	}
}