#pragma once

#define VISERA_ASSERT(Expression) assert(Expression);
#define VISERA_WIP VISERA_ASSERT("Work in Progress!")

#if defined(_WIN32) || defined(_WIN64)
#define VISERA_ON_WINDOWS_SYSTEM
#endif
#if defined(__APPLE__)
#define VISERA_ON_APPLE_SYSTEM
#endif

#if (defined(_M_IX86) || defined(__i386__) || defined(_M_X64) || defined(__amd64__) || defined(__x86_64__)) && !defined(_M_ARM64EC)
#define VISERA_ON_X86_CPU
#endif

#if (defined(__arm__) || defined(_M_ARM) || defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC))
#define VISERA_ON_ARM_CPU
#endif

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define VISERA_ON_LITTLE_ENDIAN_PLATFORM
#elif defined(_WIN32)
    // Windows is always little-endian (on supported CPUs)
    #define VISERA_ON_LITTLE_ENDIAN_PLATFORM
#endif

#if defined(_MSC_VER)
#define VISERA_ON_MSVC_COMPILER
#endif

#if defined(VISERA_ON_MSVC_COMPILER)
#define VISERA_NO_OPERATION __noop
#else
#define VISERA_NO_OPERATION (void)(0)
#endif

//      [       Level      ]   [Print in Console] [Sink in Files] [Text Color] [Background Color] [Additional Information]
#define VISERA_LOG_LEVEL_TRACE 0 //|   Conditional  | | Conditional | |   Grey   | |                | |                      |
#define VISERA_LOG_LEVEL_DEBUG 1 //|   Conditional  | |     Yes     | |   Blue   | |                | |                      |
#define VISERA_LOG_LEVEL_INFO  2 //|   Conditional  | |     Yes     | |   Green  | |                | |                      |
#define VISERA_LOG_LEVEL_WARN  3 //|      Yes       | |     Yes     | |   Yellow | |                | |                      |
#define VISERA_LOG_LEVEL_ERROR 4 //|      Yes       | |     Yes     | |   Red    | |                | |                      |
#define VISERA_LOG_LEVEL_FATAL 5 //|      Yes       | |     Yes     | |   Red    | |       Red      | |     SRuntimeError    |

#define VISERA_LOG_SYSTEM_VERBOSITY VISERA_LOG_LEVEL_DEBUG

#if defined(VISERA_ON_MSVC_COMPILER)
	#if VISERA_LOG_LEVEL_TRACE >= VISERA_LOG_SYSTEM_VERBOSITY
	#define VISERA_LOG_TRACE(I_Fmt, ...) GLog::GetInstance().Trace("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define VISERA_LOG_TRACE(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_DEBUG >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_DEBUG(I_Fmt, ...) GLog::GetInstance().Debug("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_DEBUG(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_INFO >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_INFO(I_Fmt, ...) GLog::GetInstance().Info("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_INFO(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_WARN >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_WARN(I_Fmt, ...) GLog::GetInstance().Warn("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_WARN(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_ERROR >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_ERROR(I_Fmt, ...) GLog::GetInstance().Error("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_ERROR(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_FATAL >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_FATAL(I_Fmt, ...) GLog::GetInstance().Fatal("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_FATAL(I_Fmt, ...) VISERA_NO_OPERATION
	#endif
#else
	#if VISERA_LOG_LEVEL_TRACE >= VISERA_LOG_SYSTEM_VERBOSITY
	#define VISERA_LOG_TRACE(I_Fmt, ...) GLog::GetInstance().Trace("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define VISERA_LOG_TRACE(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_DEBUG >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_DEBUG(I_Fmt, ...) GLog::GetInstance().Debug("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define LOG_DEBUG(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_INFO >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_INFO(I_Fmt, ...) GLog::GetInstance().Info("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define LOG_INFO(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_WARN >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_WARN(I_Fmt, ...) GLog::GetInstance().Warn("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define LOG_WARN(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_ERROR >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_ERROR(I_Fmt, ...) GLog::GetInstance().Error("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define LOG_ERROR(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_FATAL >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_FATAL(I_Fmt, ...) GLog::GetInstance().Fatal("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define LOG_FATAL(I_Fmt, ...) VISERA_NO_OPERATION
	#endif
#endif

#define TEXT(I_Text) u8##I_Text

// << STD Modules >>
#include <cassert>
#include <cmath>
#include <random>
#include <sstream>
#include <ostream>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <format>
#include <vector>
#include <list>
#include <bit>
#include <algorithm>
#include <array>
#include <bitset>
#include <shared_mutex>
#include <ranges>
#include <memory>
#include <typeinfo>
#include <filesystem>
#include <functional>
#include <source_location>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <type_traits>

// << Formatter >>
#include <spdlog/fmt/fmt.h>

namespace Visera
{
    using Bool		    = bool;
    using Float  	    = float;
    using Double 	    = double;
    using Int8          = char;
    using UInt8         = unsigned char;
    using Int16         = int16_t;
    using UInt16        = uint16_t;
    using Int32  	    = std::int32_t;
    using UInt32 	    = std::uint32_t;
    using Int64  	    = std::int64_t;
    using UInt64 	    = std::uint64_t;

    using FString       = std::string;
    using FStringView   = std::string_view;

    using FByte         = UInt8;
    using FANSIChar	    = char;
    using FWideChar     = wchar_t;

    constexpr Bool True  = true;
    constexpr Bool False = false;

    template<typename T>
    using TArray   = std::vector<T>;

    template<typename T>
    using TList     = std::list<T>;

    template<typename T>
    using TSet      = std::unordered_set<T>;

    template<typename Key, typename Value>
    using THashMap  = std::unordered_map<Key, Value>;

    template<typename T1, typename T2>
    using TPair     = std::pair<T1, T2>;

    template <typename... Args>
    using TTuple = std::tuple<Args...>;

    namespace Concepts
    {
        template<typename T> concept
        Integeral = std::integral<T> || std::unsigned_integral<T>;

        template<typename T> concept
        FloatingPoint = std::floating_point<T>;
    }

    template<Concepts::Integeral T>
    Bool IsPowerOfTwo(T I_Number) { return (I_Number > 0) && ((I_Number & (I_Number - 1)) == 0); }


    template<typename T>
    using TSharedPtr   = std::shared_ptr<T>;
    template<typename T, typename... Args>
    TSharedPtr<T>
    MakeShared(Args &&...args) { return std::make_shared<T>(std::forward<Args>(args)...); }

    template<typename T>
    using WeakPtr	  = std::weak_ptr<T>;

    template<typename T>
    using TUniquePtr   = std::unique_ptr<T>;
    template<typename T, typename... Args>
    TUniquePtr<T>
    MakeUnique(Args &&...args) { return std::make_unique<T>(std::forward<Args>(args)...); }

}