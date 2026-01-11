#pragma once

#if defined(VISERA_ON_WINDOWS_SYSTEM)
  #if defined(VISERA_CORE_BUILD_STATIC)
    #define VISERA_CORE_API
  #elif defined(VISERA_CORE_BUILD_SHARED) || defined(VISERA_MONOLITHIC_MODE)
    #define VISERA_CORE_API __declspec(dllexport)
  #else
    #define VISERA_CORE_API __declspec(dllimport)
  #endif
#else
  #define VISERA_CORE_API __attribute__((visibility("default")))
#endif

#if (defined(_M_IX86) || defined(__i386__) || defined(_M_X64) || defined(__amd64__) || defined(__x86_64__)) && !defined(_M_ARM64EC)
#define VISERA_ON_X86_CPU
#endif

#if (defined(__arm__) || defined(_M_ARM) || defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC))
#define VISERA_ON_ARM_CPU
#endif

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define VISERA_ON_LITTLE_ENDIAN_PLATFORM
#elif defined(VISERA_ON_WINDOWS_SYSTEM)
    // Windows is always little-endian (on supported CPUs)
    #define VISERA_ON_LITTLE_ENDIAN_PLATFORM
#endif

#if defined(_MSC_VER)
#define VISERA_ON_MSVC_COMPILER
#elif defined(__clang__)
#define VISERA_ON_CLANG_COMPILER
#elif defined(__GNUC__)
#define VISERA_ON_GCC_COMPILER
#endif

#if defined(VISERA_ON_MSVC_COMPILER)
#define VISERA_NO_OPERATION __noop
#else
#define VISERA_NO_OPERATION (void)(0)
#endif

#if defined(VISERA_ON_MSVC_COMPILER)
	#define VISERA_NOINLINE      __declspec(noinline)
	#define VISERA_FORCEINLINE   __forceinline
#elif defined(VISERA_ON_GCC_COMPILER) || defined(VISERA_ON_CLANG_COMPILER)
	#define VISERA_NOINLINE      __attribute__((noinline))
	#define VISERA_FORCEINLINE   inline __attribute__((always_inline))
#else
	#define VISERA_NOINLINE
	#define VISERA_FORCEINLINE   inline
#endif

#if defined(VISERA_ON_WINDOWS_SYSTEM)
	#define PLATFORM_ASSERT(expression) ((void)(                                                       \
		(!!(expression)) ||                                                               \
		(_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0)) \
		)
#else
	#if __DARWIN_UNIX03
	#define	PLATFORM_ASSERT(e) \
	(__builtin_expect(!(e), 0) ? __assert_rtn(__func__, __ASSERT_FILE_NAME, __LINE__, #e) : (void)0)
	#else /* !__DARWIN_UNIX03 */
	#define PLATFORM_ASSERT(e)  \
	(__builtin_expect(!(e), 0) ? __assert (#e, __ASSERT_FILE_NAME, __LINE__) : (void)0)
	#endif /* __DARWIN_UNIX03 */
#endif

#if defined(VISERA_RELEASE_MODE)
#define VISERA_ASSERT(Expression) VISERA_NO_OPERATION
#else
#define VISERA_ASSERT(Expression) PLATFORM_ASSERT(Expression)
#endif

#if !defined(VISERA_RELEASE_MODE)
#define DEBUG_ONLY_FIELD(I_Content) I_Content
#define RELEASE_ONLY_FIELD(I_Content)
#else
#define DEBUG_ONLY_FIELD(I_Content)
#define RELEASE_ONLY_FIELD(I_Content) I_Content
#endif

#define VISERA_UNIMPLEMENTED_API VISERA_ASSERT(False)

// << PCHs >>
#include <cassert>
#include <ranges>
#include <memory>
#include <string>
#include <chrono>
#include <algorithm>
#include <functional>
#include <string_view>
#include <type_traits>
#include <source_location>
#include <spdlog/fmt/fmt.h>

namespace Visera
{
	using Bool		    = bool;
    using Float  	    = float;
    using Double 	    = double;
    using Int8          = char;
    using UInt8         = unsigned char;
    using FByte         = UInt8;
    using Int16         = int16_t;
    using UInt16        = uint16_t;
    using Int32  	    = std::int32_t;
    using UInt32 	    = std::uint32_t;
    using Int64  	    = std::int64_t;
    using UInt64 	    = std::uint64_t;
	using UInt128		= std::pair<UInt64, UInt64>;

    using FString         = std::string;
    using FStringView     = std::string_view;
	using FWideString     = std::wstring;
	using FWideStringView = std::wstring_view;
	using FUTF8StringView = std::u8string_view;

    using FANSIChar	    = char;
    using FWideChar     = wchar_t;

#if defined(VISERA_ON_WINDOWS_SYSTEM)
	#define PLATFORM_STRING(I_String) L##I_String
	using FPlatformChar   = wchar_t;
	using FPlatformString = FWideString;
	using FPlatformStringView = FWideStringView;
#else
	#define PLATFORM_STRING(I_String) I_String
	using FPlatformChar   = char;
	using FPlatformString = FString;
	using FPlatformStringView = FStringView;
#endif

    constexpr Bool True  = true;
    constexpr Bool False = false;

	using FErrorCode = std::error_code;

	namespace Concepts
	{
		template<typename T> concept
		Integral = std::integral<T>;

		template<typename T> concept
		SignedIntegral = std::signed_integral<T>;

		template<typename T> concept
		UnsingedIntegral = std::unsigned_integral<T>;

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
	}

	template <Concepts::Mutable T>
	using TMutable  = T*;

    template<typename T1, typename T2>
    using TPair     = std::pair<T1, T2>;

    template <typename... Args>
	using TTuple    = std::tuple<Args...>;

	template<typename T>
	using TOptional = std::optional<T>;

	template<typename Signature>
	using TFunction = std::function<Signature>;

    template<typename T>
    using TSharedPtr   = std::shared_ptr<T>;
    template<typename T, typename... Args> TSharedPtr<T>
    MakeShared(Args &&...args) { return std::make_shared<T>(std::forward<Args>(args)...); }

	template<typename T>
	using TSharedRef   = const TSharedPtr<T>&;

    template<typename T>
	using TWeakPtr	   = std::weak_ptr<T>;
	template<typename T>
	using TWeakRef	   = const std::weak_ptr<T>&;

    template<typename T>
    using TUniquePtr   = std::unique_ptr<T>;
    template<typename T, typename... Args> TUniquePtr<T>
    MakeUnique(Args &&...args) { return std::make_unique<T>(std::forward<Args>(args)...); }

	template<typename T>
	using TUniqueRef   = const TUniquePtr<T>&;

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

	template<typename... Args> [[nodiscard]] static FString
	Format(fmt::format_string<Args...> I_Fmt, Args &&... I_Args)
	{ return fmt::format(I_Fmt, std::forward<Args>(I_Args)...); }
}

#define VISERA_MAKE_FLAGS(EnumClass)			\
	static_assert(std::is_enum_v<EnumClass>);	\
	template<> struct Visera::TEnableBitMaskOperators<EnumClass> : std::true_type {}

#define VISERA_MAKE_FORMATTER(Type, BODY, FormatString, ...)               \
	template <>                                                            \
	struct fmt::formatter<Type>                                            \
	{                                                                      \
		constexpr auto parse(fmt::format_parse_context& I_Context)         \
		-> decltype(I_Context.begin())									   \
		{                                                                  \
			return I_Context.begin();                                      \
		}                                                                  \
		\
		template <typename FormatContext>                                  \
		auto format(const Type& I_Formatee, FormatContext& I_Context) const\
		-> decltype(I_Context.out())									   \
		{                                                                  \
			BODY														   \
			return fmt::format_to(I_Context.out(),                         \
                   FormatString, __VA_ARGS__);                             \
		}                                                                  \
	};

#define VISERA_MAKE_HASH(Type, BODY)                       \
	namespace std { template<> struct hash<Type> {         \
	size_t operator()(const Type& I_Object) const noexcept \
	{ BODY } }; }