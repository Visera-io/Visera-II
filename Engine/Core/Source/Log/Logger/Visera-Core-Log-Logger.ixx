module;
#include <Visera-Core.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
export module Visera.Core.Log.Logger;
#define VISERA_MODULE_NAME "Core.Log"
import Visera.Core.Containers.Array;
import Visera.Core.OS.Time;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;

export namespace Visera
{
	/** Max size per log file before rotation (10 MB). */
	inline constexpr UInt64 kLogMaxBytes = 10ULL * 1024ULL * 1024ULL;
	/** Number of rotated log files to keep (1 current + 4 rotated = 5 total). */
	inline constexpr UInt32 kLogMaxFiles = 5U;

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

		/** Sets the directory for the log file sink. Creates a rotating file sink under I_LogDirectory; no file is written until this is called. Only the first call takes effect. */
		void SetSinkPath(const FPath& I_LogDirectory);

	protected:
		spdlog::logger* Backend {nullptr};
		ELevel       Level = ELevel::Trace;
		Bool         FileSinkAttached {False};

	public:
		FLogger(ELevel I_Level = ELevel::Trace);
		~FLogger();
	};

	FLogger::
	FLogger(ELevel I_Level /* = ELevel::Trace */)
	: Level(I_Level)
	{
		TArray<spdlog::sink_ptr> InitialSinks;
#if defined(VISERA_RELEASE_MODE)
		/* Release: no console sink; file sink is added only when SetSinkPath is called by Platform. */
#else
		/* Debug/Develop: console sink only; file sink optional via SetSinkPath. */
		InitialSinks.PushBack(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif
		Backend = new spdlog::logger("Visera Log", InitialSinks.begin(), InitialSinks.end());
		Backend->set_pattern("%^[%L] [%Y-%m-%d %H:%M:%S.%e] [T:%t] %v%$");
		SetLevel(Level);
	}

	void FLogger::
	SetSinkPath(const FPath& I_LogDirectory)
	{
		if (FileSinkAttached) { return; }
		if (I_LogDirectory.IsEmpty()) { return; }

		const FSystemTimePoint Now = FSystemClock::Now();
		const FString Timestamp = Now.ToString("%Y-%m-%d_%H-%M-%S");
		const FString FileName = FString::Format("Visera_{}.log", Timestamp);
		const FPath FullPath = I_LogDirectory / FPath(FileName);
		const std::string PathNative = FullPath.GetString().GetNative();

		auto FileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(PathNative, static_cast<std::size_t>(kLogMaxBytes), kLogMaxFiles);
		Backend->sinks().push_back(FileSink);
		FileSinkAttached = True;
	}

	FLogger::
	~FLogger()
	{
		Backend->flush();
		delete Backend;
	}
}