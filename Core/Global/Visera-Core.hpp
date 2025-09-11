#pragma once

#define VISERA_ASSERT(Expression) assert(Expression);
#define VISERA_WIP VISERA_ASSERT("Work in Progress!")

#if (!NDEBUG)
#define VISERA_DEBUG_MODE
#endif

#if defined(_WIN32) || defined(_WIN64)
#define VISERA_ON_WINDOWS_SYSTEM
#endif
#if defined(__APPLE__)
#define VISERA_ON_APPLE_SYSTEM
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
	#if defined(VISERA_CORE_BUILD_SHARED)
		#define VISERA_CORE_API __declspec(dllexport)
	#else
		#define VISERA_CORE_API __declspec(dllimport)
	#endif
#else
	#define VISERA_CORE_API
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
	#define LOG_TRACE(I_Fmt, ...) GLog->Trace("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_TRACE(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_DEBUG >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_DEBUG(I_Fmt, ...) GLog->Debug("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_DEBUG(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_INFO >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_INFO(I_Fmt, ...) GLog->Info("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_INFO(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_WARN >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_WARN(I_Fmt, ...) GLog->Warn("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_WARN(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_ERROR >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_ERROR(I_Fmt, ...) GLog->Error("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_ERROR(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_FATAL >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_FATAL(I_Fmt, ...) GLog->Fatal("[M:{}] " I_Fmt, VISERA_MODULE_NAME, __VA_ARGS__);
	#else
	#define LOG_FATAL(I_Fmt, ...) VISERA_NO_OPERATION
	#endif
#else
	#if VISERA_LOG_LEVEL_TRACE >= VISERA_LOG_SYSTEM_VERBOSITY
	#define VISERA_LOG_TRACE(I_Fmt, ...) GLog->Trace("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define VISERA_LOG_TRACE(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_DEBUG >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_DEBUG(I_Fmt, ...) GLog->Debug("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define LOG_DEBUG(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_INFO >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_INFO(I_Fmt, ...) GLog->Info("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define LOG_INFO(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_WARN >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_WARN(I_Fmt, ...) GLog->Warn("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define LOG_WARN(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_ERROR >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_ERROR(I_Fmt, ...) GLog->Error("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define LOG_ERROR(I_Fmt, ...) VISERA_NO_OPERATION
	#endif

	#if VISERA_LOG_LEVEL_FATAL >= VISERA_LOG_SYSTEM_VERBOSITY
	#define LOG_FATAL(I_Fmt, ...) GLog->Fatal("[M:{}] " I_Fmt, VISERA_MODULE_NAME, ##__VA_ARGS__);
	#else
	#define LOG_FATAL(I_Fmt, ...) VISERA_NO_OPERATION
	#endif
#endif

#define TEXT(I_Text) FText{u8##I_Text}
#define PATH(I_Path) FPath{FText(u8##I_Path)}

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
#include <variant>
#include <type_traits>
#include <ankerl/unordered_dense.h>

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
	using UInt128       = std::pair<UInt64, UInt64>;

    using FString       = std::string;
    using FStringView   = std::string_view;

    using FByte         = UInt8;
    using FANSIChar	    = char;
    using FWideChar     = wchar_t;

    constexpr Bool True  = true;
    constexpr Bool False = false;

	namespace Concepts
	{
		template<typename T> concept
		Integeral     = std::integral<T> || std::unsigned_integral<T>;

		template<typename T> concept
		FloatingPoint = std::floating_point<T>;

		template<typename T> concept
		Constant      = std::is_const_v<T>;

		template<typename T> concept
		Mutable = requires
		{
			requires !std::is_const_v<std::remove_reference_t<T>>;
			requires !std::is_const_v<std::remove_pointer_t<T>>;
		};

		template<typename T> concept
	    Clock = requires
		{
			requires std::is_class_v<std::chrono::system_clock>;
			requires std::is_class_v<std::chrono::high_resolution_clock>;
		};
	}

    template<typename T>
    using TArray    = std::vector<T>;

	template <typename T>
	using TArrayView= std::span<const T>;

	template <typename T, size_t Extent>
	using TSpan     = std::span<T, Extent>;

	template <Concepts::Mutable T>
	using TMutable  = T*;

    template<typename T>
    using TList     = std::list<T>;

    template<typename T>
    using TSet      = ankerl::unordered_dense::set<T>;

    template<typename Key, typename Value>
    using TMap      = ankerl::unordered_dense::map<Key, Value>;

    template<typename T1, typename T2>
    using TPair     = std::pair<T1, T2>;

    template <typename... Args>
    using TTuple    = std::tuple<Args...>;

	VISERA_CORE_API void inline
	Sleep(Float I_Seconds) { std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<UInt64>(1000 * I_Seconds))); }

    template<typename T>
    using TSharedPtr   = std::shared_ptr<T>;
    template<typename T, typename... Args>
    TSharedPtr<T>
    MakeShared(Args &&...args) { return std::make_shared<T>(std::forward<Args>(args)...); }

    template<typename T>
    using TWeakPtr	   = std::weak_ptr<T>;

    template<typename T>
    using TUniquePtr   = std::unique_ptr<T>;
    template<typename T, typename... Args>
    TUniquePtr<T>
    MakeUnique(Args &&...args) { return std::make_unique<T>(std::forward<Args>(args)...); }

	class VISERA_CORE_API IGlobalSingleton
    {
    public:
    	enum EStatues { Disabled, Bootstrapped, Terminated  };

    	[[nodiscard]] FStringView
		GetDebugName() const { return Name; }

    	virtual void inline
		Bootstrap() = 0;
    	virtual void inline
		Terminate() = 0;

    	[[nodiscard]] inline Bool
		IsBootstrapped() const { return Statue == EStatues::Bootstrapped; }
    	[[nodiscard]] inline Bool
		IsTerminated() const   { return Statue == EStatues::Terminated; }

    	IGlobalSingleton() = delete;
    	explicit IGlobalSingleton(const char* I_Name) : Name(I_Name) {}
    	virtual ~IGlobalSingleton()
    	{
    		if (IsBootstrapped())
    		{
    			std::cerr << "[WARNING] " << Name << " was NOT terminated properly!\n";
    		}
    	}

    	IGlobalSingleton(const IGlobalSingleton&)			 = delete;
    	IGlobalSingleton& operator=(const IGlobalSingleton&) = delete;
    	IGlobalSingleton(IGlobalSingleton&&)				 = delete;
    	IGlobalSingleton& operator=(IGlobalSingleton&&)      = delete;

    protected:
    	const char* Name;
    	mutable EStatues Statue = EStatues::Disabled;
    };

	constexpr bool operator==(const UInt128& I_A, const UInt128& I_B)
	{ return I_A.first == I_B.first && I_A.second == I_B.second; }

	constexpr bool operator!=(const UInt128& I_A, const UInt128& I_B)
	{ return !(I_A == I_B); }

	constexpr bool operator<(const UInt128& I_A, const UInt128& I_B)
	{
		if (I_A.first < I_B.first) { return true;  }
		if (I_A.first > I_B.first) { return false; }
		return I_A.second < I_B.second;
	}

	constexpr bool operator>(const UInt128& I_A, const UInt128& I_B)
	{ return I_B < I_A; }

	constexpr bool operator<=(const UInt128& I_A, const UInt128& I_B)
	{ return !(I_B < I_A); }

	constexpr bool operator>=(const UInt128& I_A, const UInt128& I_B)
	{ return !(I_A < I_B); }
}