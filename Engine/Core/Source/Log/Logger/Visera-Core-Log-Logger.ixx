module;
#include <Visera-Core.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
export module Visera.Core.Log.Logger;
#define VISERA_MODULE_NAME "Core.Log"
import Visera.Core.OS.Time;
import Visera.Core.Types.String;

export namespace Visera
{
	class VISERA_CORE_API FLogger
	{
	public:
		enum class ELevel : UInt8
		{
			Trace   = spdlog::level::level_enum::trace,
			Debug   = spdlog::level::level_enum::debug,
			Info    = spdlog::level::level_enum::info,
			Warn    = spdlog::level::level_enum::warn,
			Error	= spdlog::level::level_enum::err,
			Fatal	= spdlog::level::level_enum::critical,
		};

		template<typename... Args> inline void
		Trace(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > ELevel::Trace) { return; }
			Backend->trace(I_Fmt, std::forward<Args>(I_Args)...);
		}

		template<typename... Args> inline void
		Debug(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > ELevel::Debug) { return; }
			Backend->debug(I_Fmt, std::forward<Args>(I_Args)...);
		}

		template<typename... Args> inline void
		Info(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > ELevel::Info) { return; }
			Backend->info(I_Fmt, std::forward<Args>(I_Args)...);
		}

		template<typename... Args> inline void
		Warn(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > ELevel::Warn) { return; }
			Backend->warn(I_Fmt, std::forward<Args>(I_Args)...);
		}

		template<typename... Args> inline void
		Error(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (Level > ELevel::Error) { return; }
			Backend->error(I_Fmt, std::forward<Args>(I_Args)...);
		}

		template<typename... Args> inline void
		Fatal(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			Backend->critical(I_Fmt, std::forward<Args>(I_Args)...);
			Backend->flush();
			std::abort();
		}

		void inline
		SetLevel(ELevel I_Level)
		{
			Level = I_Level;
			Backend->set_level(static_cast<spdlog::level::level_enum>(Level));
		}

	protected:
		spdlog::logger* Backend {nullptr};
		ELevel       Level = ELevel::Trace;

	public:
		FLogger(ELevel I_Level = ELevel::Trace);
		~FLogger();
	};
	
	FLogger::
	FLogger(ELevel I_Level /* = ELevel::Trace */)
	: Level(I_Level)
	{
		auto ConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

		// auto RotatingSink = MakeShared<spdlog::sinks::rotating_file_sink_mt>(
		// 	"/Logs/Engine.log", //VISERA_APP_CACHE_DIR
		// 	5 * 1024, // N MB per file,
		// 	1         // keep N backups
		// );
		Backend = new spdlog::logger("Visera Log", spdlog::sinks_init_list{ ConsoleSink /*, RotatingSink*/ });

		Backend->set_pattern("%^[%L] [%Y-%m-%d %H:%M:%S.%e] [T:%t] %v%$");
		//Backend->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%L] %v%$");
		SetLevel(Level);
	}

	FLogger::
	~FLogger()
	{
		Backend->flush();
		//Do not call drop_all() in your class! spdlog::drop_all();
		delete Backend;
	}
}