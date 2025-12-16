#pragma once
#include <Visera-Core.hpp>
#if defined(VISERA_CORE_API)
#undef VISERA_CORE_API
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
  #if defined(VISERA_RUNTIME_BUILD_STATIC)
    #define VISERA_RUNTIME_API
  #elif defined(VISERA_RUNTIME_BUILD_SHARED) || defined(VISERA_MONOLITHIC_MODE)
    #define VISERA_RUNTIME_API __declspec(dllexport)
  #else
    #define VISERA_RUNTIME_API __declspec(dllimport)
  #endif
#else
  #define VISERA_RUNTIME_API __attribute__((visibility("default")))
#endif

//      [       Level      ]   [Print in Console] [Sink in Files] [Text Color] [Background Color] [Additional Information]
#define VISERA_LOG_LEVEL_TRACE 0 //|   Conditional  | | Conditional | |   Grey   | |                | |                      |
#define VISERA_LOG_LEVEL_DEBUG 1 //|   Conditional  | |     Yes     | |   Blue   | |                | |                      |
#define VISERA_LOG_LEVEL_INFO  2 //|   Conditional  | |     Yes     | |   Green  | |                | |                      |
#define VISERA_LOG_LEVEL_WARN  3 //|      Yes       | |     Yes     | |   Yellow | |                | |                      |
#define VISERA_LOG_LEVEL_ERROR 4 //|      Yes       | |     Yes     | |   Red    | |                | |                      |
#define VISERA_LOG_LEVEL_FATAL 5 //|      Yes       | |     Yes     | |   Red    | |       Red      | |        Abort()       |

#if defined(VISERA_DEBUG_MODE)
#define VISERA_LOG_SYSTEM_VERBOSITY VISERA_LOG_LEVEL_TRACE
#elif defined(VISERA_DEVELOP_MODE)
#define VISERA_LOG_SYSTEM_VERBOSITY VISERA_LOG_LEVEL_DEBUG
#elif defined(VISERA_RELEASE_MODE)
#define VISERA_LOG_SYSTEM_VERBOSITY VISERA_LOG_LEVEL_INFO
#endif

#if defined(VISERA_ON_MSVC_COMPILER)
	#if VISERA_LOG_LEVEL_TRACE >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_TRACE(I_Fmt, ...) Visera::GLog->Trace("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__)
	#else
	#define LOG_TRACE(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_DEBUG >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_DEBUG(I_Fmt, ...) Visera::GLog->Debug("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__)
	#else
	#define LOG_DEBUG(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_INFO >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_INFO(I_Fmt, ...) Visera::GLog->Info("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__)
	#else
	#define LOG_INFO(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_WARN >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_WARN(I_Fmt, ...) Visera::GLog->Warn("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__)
	#else
	#define LOG_WARN(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_ERROR >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_ERROR(I_Fmt, ...) Visera::GLog->Error("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__)
	#else
	#define LOG_ERROR(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_FATAL >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_FATAL(I_Fmt, ...) Visera::GLog->Fatal("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__)
	#else
	#define LOG_FATAL(I_Fmt, ...) VISERA_NO_OPERATION
	#endif
#else
	#if VISERA_LOG_LEVEL_TRACE >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_TRACE(I_Fmt, ...) Visera::GLog->Trace("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__)
	#else
	#define LOG_TRACE(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_DEBUG >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_DEBUG(I_Fmt, ...) Visera::GLog->Debug("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__)
	#else
	#define LOG_DEBUG(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_INFO >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_INFO(I_Fmt, ...) Visera::GLog->Info("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__)
	#else
	#define LOG_INFO(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_WARN >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_WARN(I_Fmt, ...) Visera::GLog->Warn("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__)
	#else
	#define LOG_WARN(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_ERROR >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_ERROR(I_Fmt, ...) Visera::GLog->Error("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__)
	#else
	#define LOG_ERROR(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_FATAL >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_FATAL(I_Fmt, ...) Visera::GLog->Fatal("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__)
	#else
	#define LOG_FATAL(I_Fmt, ...) VISERA_NO_OPERATION
	#endif
#endif