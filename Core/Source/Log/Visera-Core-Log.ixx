module;
#include <Visera-Core.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
export module Visera.Core.Log;
#define VISERA_MODULE_NAME "Core.Log"

namespace Visera
{

	class VISERA_CORE_API FLog : public IGlobalSingleton
	{
	public:
		template<typename... Args>
		inline void
		Trace(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (IsBootstrapped())
			{ Logger->trace(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('T', I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args>
		inline void
		Debug(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (IsBootstrapped())
			{ Logger->debug(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('D', I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args>
		inline void
		Info(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (IsBootstrapped())
			{ Logger->info(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('I', I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args>
		inline void
		Warn(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (IsBootstrapped())
			{ Logger->warn(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('W', I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args>
		inline void
		Error(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (IsBootstrapped())
			{ Logger->error(I_Fmt, std::forward<Args>(I_Args)...); }
			else
			{ NativeLog('E', I_Fmt, std::forward<Args>(I_Args)...); }
		}

		template<typename... Args>
		inline void
		Fatal(spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			if (IsBootstrapped())
			{
				Logger->critical(I_Fmt, std::forward<Args>(I_Args)...);
				Logger->flush();
			}
			else
			{
				NativeLog('C', I_Fmt, std::forward<Args>(I_Args)...);
			}

			std::abort();
		}

	protected:
		TUniquePtr<spdlog::logger> Logger;

		template<typename... Args>
		inline void
		NativeLog(const char I_Level, spdlog::format_string_t<Args...> I_Fmt, Args &&...I_Args)
		{
			const auto Now = std::chrono::system_clock::now();
			const auto Time = std::chrono::system_clock::to_time_t(Now);
			const auto MilliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(Now.time_since_epoch()) % 1000;
			
			std::tm local_tm{};
#if defined(VISERA_ON_WINDOWS_SYSTEM)
			localtime_s(&local_tm, &Time);
#else
			localtime_r(&Time, &local_tm);
#endif

			auto thread_id = std::this_thread::get_id()._Get_underlying_id();
			auto formatted_msg = fmt::format(I_Fmt, std::forward<Args>(I_Args)...);
			auto Stream = (I_Level == 'E' || I_Level == 'C')? stderr : stdout;
			fmt::println(Stream, "[{}] [{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d}] [T:{}] {}",
						I_Level,
						local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
						local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, MilliSeconds.count(),
						thread_id, formatted_msg);
			if (I_Level == 'E' || I_Level == 'C')
			{ std::fflush(Stream); }
		}

	public:
		FLog() : IGlobalSingleton{"Log"} {}
		~FLog() noexcept override = default;
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
		LOG_DEBUG("Bootstrapping Log.");

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

		Statue = EStatues::Bootstrapped;
	}

	void FLog::
	Terminate()
	{
		LOG_DEBUG("Terminating Log.");

		Logger->flush();
		//Do not call drop_all() in your class! spdlog::drop_all();
		Logger.reset();

		Statue = EStatues::Terminated;
	}

}