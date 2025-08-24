module;
#include <Visera-Core.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
export module Visera.Core.Log;
#define VISERA_MODULE_NAME "Core.Log"

export namespace Visera
{

	class VISERA_CORE_API GLog
	{
	public:
		static inline GLog&
		GetInstance()
		{ static GLog Instance; return Instance; }

		template<typename... Args>
		inline void
		Trace(spdlog::format_string_t<Args...> Formatter, Args &&...Arguments)
		{ Logger->trace(Formatter, std::forward<Args>(Arguments)...); }

		template<typename... Args>
		inline void
		Debug(spdlog::format_string_t<Args...> Formatter, Args &&...Arguments)

		{ Logger->debug(Formatter, std::forward<Args>(Arguments)...); }
		template<typename... Args>
		inline void
		Info(spdlog::format_string_t<Args...> Formatter, Args &&...Arguments)
		{ Logger->info(Formatter, std::forward<Args>(Arguments)...); }

		template<typename... Args>
		inline void
		Warn(spdlog::format_string_t<Args...> Formatter, Args &&...Arguments)
		{ Logger->warn(Formatter, std::forward<Args>(Arguments)...); }

		template<typename... Args>
		inline void
		Error(spdlog::format_string_t<Args...> Formatter, Args &&...Arguments)
		{ Logger->error(Formatter, std::forward<Args>(Arguments)...); }

		template<typename... Args>
		inline void
		Fatal(spdlog::format_string_t<Args...> Formatter, Args &&...Arguments)
		{
			Logger->critical(Formatter, std::forward<Args>(Arguments)...);
			//throw SRuntimeError("A Fatal Error was triggered.");
		}

		GLog() noexcept
		{
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
		}
		virtual ~GLog() noexcept
		{
			Logger->flush();
			//Do not call drop_all() in your class! spdlog::drop_all();
			Logger.reset();
		}

	protected:
		TUniquePtr<spdlog::logger> Logger;
	};

}