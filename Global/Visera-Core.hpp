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

#if defined(_MSC_VER)
#define VISERA_ON_MSVC_COMPILER
#endif

#if VISERA_IS_MSVC_COMPILER
#define VISERA_NO_OPERATION __noop
#else
#define VISERA_NO_OPERATION (void)(0)
#endif

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
    using TVector   = std::vector<T>;

    template<typename T, size_t Length>
    using TArray    = std::array<T, Length>;

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
    }

    template<Concepts::Integeral T>
    Bool IsPowerOfTwo(T I_Number) { return (I_Number > 0) && ((I_Number & (I_Number - 1)) == 0); }

}