#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <concepts>
#include <memory_resource>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "../waypoint/charted-waypoint.hpp"

namespace charted
{
    class RouteView;
    class Cursor;

    template <std::size_t N>
    struct StringLiteral
    {
        char Data[N]{};

        constexpr StringLiteral(const char (&value)[N]) noexcept
        {
            for (std::size_t i = 0; i < N; ++i)
            {
                Data[i] = value[i];
            }
        }

        [[nodiscard]] static consteval std::size_t Size() noexcept { return N; }
    };

    template <std::size_t N>
    StringLiteral(const char (&)[N]) -> StringLiteral<N>;

    namespace detail
    {
        template <std::size_t MaxTokens>
        struct ParsedStaticRoute
        {
            std::array<Waypoint, MaxTokens> Waypoints{};
            std::size_t                       Count{ 0 };
            bool                              Valid{ true };
        };

        template <StringLiteral Route, std::size_t MaxTokens>
        consteval ParsedStaticRoute<MaxTokens> ParseStaticRoute() noexcept
        {
            ParsedStaticRoute<MaxTokens> parsed{};
            constexpr std::size_t routeSize = Route.Size();
            constexpr std::size_t length   = (routeSize > 0) ? (routeSize - 1) : 0;
            constexpr std::size_t maxTokenLength = 65535;

            std::size_t cursor = 0;
            while (cursor < length)
            {
                if (Route.Data[cursor] == '.')
                {
                    if ((cursor + 1) < length && Route.Data[cursor + 1] == '.')
                    {
                        parsed.Valid = false;
                        return parsed;
                    }
                    ++cursor;
                    continue;
                }

                const std::size_t keyStart = cursor;
                while (cursor < length && Route.Data[cursor] != '.' && Route.Data[cursor] != '[')
                {
                    ++cursor;
                }

                if (keyStart < cursor)
                {
                    if (parsed.Count >= MaxTokens)
                    {
                        parsed.Valid = false;
                        return parsed;
                    }
                    if ((cursor - keyStart) > maxTokenLength)
                    {
                        parsed.Valid = false;
                        return parsed;
                    }
                    parsed.Waypoints[parsed.Count++] = Waypoint{
                        .Ptr    = Route.Data + keyStart,
                        .Length = static_cast<std::uint16_t>(cursor - keyStart),
                        .Type   = WaypointType::Key,
                        .Index  = 0
                    };
                }

                if (cursor < length && Route.Data[cursor] == '[')
                {
                    ++cursor;
                    const std::size_t indexStart = cursor;
                    while (cursor < length && Route.Data[cursor] != ']')
                    {
                        const char c = Route.Data[cursor];
                        if (c < '0' || c > '9')
                        {
                            parsed.Valid = false;
                            return parsed;
                        }
                        ++cursor;
                    }

                    if (cursor >= length || indexStart == cursor)
                    {
                        parsed.Valid = false;
                        return parsed;
                    }

                    std::uint32_t index = 0;
                    for (std::size_t i = indexStart; i < cursor; ++i)
                    {
                        index = (index * 10u) + static_cast<std::uint32_t>(Route.Data[i] - '0');
                    }

                    if (parsed.Count >= MaxTokens)
                    {
                        parsed.Valid = false;
                        return parsed;
                    }
                    if ((cursor - indexStart) > maxTokenLength)
                    {
                        parsed.Valid = false;
                        return parsed;
                    }

                    parsed.Waypoints[parsed.Count++] = Waypoint{
                        .Ptr    = Route.Data + indexStart,
                        .Length = static_cast<std::uint16_t>(cursor - indexStart),
                        .Type   = WaypointType::Index,
                        .Index  = index
                    };

                    ++cursor;
                }

                if (cursor < length && Route.Data[cursor] == '.')
                {
                    if ((cursor + 1) < length && Route.Data[cursor + 1] == '.')
                    {
                        parsed.Valid = false;
                        return parsed;
                    }
                    ++cursor;
                }
            }

            return parsed;
        }

        inline std::size_t EstimateWaypointCapacity(std::string_view route) noexcept
        {
            if (route.empty())
            {
                return 0;
            }

            std::size_t capacity = 1;
            for (const char c : route)
            {
                if (c == '.' || c == '[')
                {
                    ++capacity;
                }
            }
            return capacity;
        }

        inline bool ParseDynamicRoute(std::string_view route, std::pmr::vector<Waypoint>& outWaypoints) noexcept
        {
            outWaypoints.clear();
            constexpr std::size_t maxTokenLength = 65535;
            std::size_t cursor = 0;
            while (cursor < route.size())
            {
                if (route[cursor] == '.')
                {
                    if ((cursor + 1) < route.size() && route[cursor + 1] == '.')
                    {
                        outWaypoints.clear();
                        return false;
                    }
                    ++cursor;
                    continue;
                }

                const std::size_t keyStart = cursor;
                while (cursor < route.size() && route[cursor] != '.' && route[cursor] != '[')
                {
                    ++cursor;
                }

                if (keyStart < cursor)
                {
                    if ((cursor - keyStart) > maxTokenLength)
                    {
                        outWaypoints.clear();
                        return false;
                    }
                    outWaypoints.push_back(Waypoint{
                        .Ptr    = route.data() + keyStart,
                        .Length = static_cast<std::uint16_t>(cursor - keyStart),
                        .Type   = WaypointType::Key,
                        .Index  = 0
                    });
                }

                if (cursor < route.size() && route[cursor] == '[')
                {
                    ++cursor;
                    const std::size_t indexStart = cursor;
                    while (cursor < route.size() && route[cursor] != ']')
                    {
                        const char c = route[cursor];
                        if (c < '0' || c > '9')
                        {
                            outWaypoints.clear();
                            return false;
                        }
                        ++cursor;
                    }

                    if (cursor >= route.size() || indexStart == cursor)
                    {
                        outWaypoints.clear();
                        return false;
                    }

                    std::uint32_t index = 0;
                    for (std::size_t i = indexStart; i < cursor; ++i)
                    {
                        index = (index * 10u) + static_cast<std::uint32_t>(route[i] - '0');
                    }
                    if ((cursor - indexStart) > maxTokenLength)
                    {
                        outWaypoints.clear();
                        return false;
                    }

                    outWaypoints.push_back(Waypoint{
                        .Ptr    = route.data() + indexStart,
                        .Length = static_cast<std::uint16_t>(cursor - indexStart),
                        .Type   = WaypointType::Index,
                        .Index  = index
                    });

                    ++cursor;
                }

                if (cursor < route.size() && route[cursor] == '.')
                {
                    if ((cursor + 1) < route.size() && route[cursor + 1] == '.')
                    {
                        outWaypoints.clear();
                        return false;
                    }
                    ++cursor;
                }
            }

            return true;
        }
    } // namespace detail

    class RouteView
    {
    public:
        constexpr RouteView() noexcept = default;

        constexpr RouteView(std::string_view route, std::span<const Waypoint> waypoints) noexcept
            : Route(route)
            , Waypoints(waypoints)
        {
        }

        template <typename TJSONRoute>
        constexpr explicit RouteView(const TJSONRoute& route)
            requires requires(const TJSONRoute& value)
            {
                { value.GetRouteString() } -> std::convertible_to<std::string_view>;
                { value.GetWaypoints() } -> std::convertible_to<std::span<const Waypoint>>;
            }
            : Route(route.GetRouteString())
            , Waypoints(route.GetWaypoints())
        {
        }

        [[nodiscard]] constexpr std::string_view GetRouteString() const noexcept { return Route; }
        [[nodiscard]] constexpr std::span<const Waypoint> GetWaypoints() const noexcept { return Waypoints; }
        [[nodiscard]] constexpr std::size_t Size() const noexcept { return Waypoints.size(); }
        [[nodiscard]] constexpr bool Empty() const noexcept { return Waypoints.empty(); }

    private:
        std::string_view            Route{};
        std::span<const Waypoint>   Waypoints{};
    };

    class DynamicRoute
    {
    public:
        DynamicRoute()
            : DynamicRoute(std::string_view{})
        {
        }

        template <std::size_t N>
        explicit DynamicRoute(
            const char (&route)[N],
            std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
            : DynamicRoute(std::string_view(route, N > 0 ? (N - 1) : 0), upstream)
        {
        }

        explicit DynamicRoute(
            const char* route,
            std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
            : DynamicRoute(std::string_view(route != nullptr ? route : ""), upstream)
        {
        }

        explicit DynamicRoute(
            std::string_view route,
            std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
            : Upstream(upstream)
            , Route(route, upstream)
            , Waypoints(upstream)
        {
            Waypoints.reserve(detail::EstimateWaypointCapacity(Route));
            Parse();
        }

        explicit DynamicRoute(
            std::string route,
            std::pmr::memory_resource* upstream = std::pmr::get_default_resource())
            : Upstream(upstream)
            , Route(std::move(route), upstream)
            , Waypoints(upstream)
        {
            Waypoints.reserve(detail::EstimateWaypointCapacity(Route));
            Parse();
        }

        DynamicRoute(const DynamicRoute& other)
            : Upstream(other.Upstream)
            , Route(other.Route, other.Upstream)
            , Waypoints(other.Upstream)
        {
            CopyWaypointsFrom(other);
            Valid = other.Valid;
        }

        DynamicRoute(DynamicRoute&& other) noexcept
            : Upstream(other.Upstream)
            , Route(std::move(other.Route), other.Upstream)
            , Waypoints(other.Upstream)
        {
            Waypoints.reserve(other.Waypoints.size());
            Parse();
        }

        DynamicRoute& operator=(const DynamicRoute& other)
        {
            if (this == &other)
            {
                return *this;
            }

            Route = other.Route;
            CopyWaypointsFrom(other);
            Valid = other.Valid;
            return *this;
        }

        DynamicRoute& operator=(DynamicRoute&& other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            Route = std::move(other.Route);
            Waypoints.reserve(other.Waypoints.size());
            Parse();
            return *this;
        }

        [[nodiscard]] std::string_view GetRouteString() const noexcept { return Route; }
        [[nodiscard]] std::span<const Waypoint> GetWaypoints() const noexcept { return Waypoints; }
        [[nodiscard]] std::size_t GetWaypointCount() const noexcept { return Waypoints.size(); }
        [[nodiscard]] bool IsValid() const noexcept { return Valid; }
        [[nodiscard]] RouteView AsView() const noexcept { return RouteView(GetRouteString(), GetWaypoints()); }
        [[nodiscard]] Cursor AsCursor() const noexcept;
        [[nodiscard]] std::pmr::memory_resource* GetMemoryResource() const noexcept
        {
            return Upstream;
        }

    private:
        void CopyWaypointsFrom(const DynamicRoute& other)
        {
            Waypoints.clear();
            Waypoints.reserve(other.Waypoints.size());

            const char* otherBase = other.Route.data();
            const char* thisBase = Route.data();

            for (const Waypoint& waypoint : other.Waypoints)
            {
                Waypoint copied = waypoint;
                if (copied.Ptr != nullptr && otherBase != nullptr && thisBase != nullptr)
                {
                    const std::size_t offset = static_cast<std::size_t>(copied.Ptr - otherBase);
                    copied.Ptr = thisBase + offset;
                }
                Waypoints.push_back(copied);
            }
        }

        void Parse() { Valid = detail::ParseDynamicRoute(Route, Waypoints); }

        std::pmr::memory_resource* Upstream{ std::pmr::get_default_resource() };
        std::pmr::string           Route;
        std::pmr::vector<Waypoint> Waypoints;
        bool                       Valid{ true };
    };

    template <StringLiteral Route, std::size_t MaxTokens = (Route.Size() > 1 ? (Route.Size() - 1) : 1)>
    class StaticRoute
    {
    public:
        static_assert(MaxTokens > 0, "MaxTokens must be greater than 0.");
        static constexpr auto Parsed = detail::ParseStaticRoute<Route, MaxTokens>();
        static_assert(Parsed.Valid, "Invalid route literal.");
        static constexpr std::size_t WaypointCount = Parsed.Count;
        static constexpr bool        Valid      = Parsed.Valid;

        [[nodiscard]] static constexpr std::string_view GetRouteString() noexcept
        {
            return std::string_view(Route.Data, Route.Size() - 1);
        }

        [[nodiscard]] static constexpr std::span<const Waypoint> GetWaypoints() noexcept
        {
            return std::span<const Waypoint>(Parsed.Waypoints.data(), Parsed.Count);
        }

        [[nodiscard]] constexpr std::size_t GetWaypointCount() const noexcept { return WaypointCount; }
        [[nodiscard]] constexpr bool IsValid() const noexcept { return Valid; }
        [[nodiscard]] constexpr RouteView AsView() const noexcept { return RouteView(GetRouteString(), GetWaypoints()); }
        [[nodiscard]] constexpr Cursor AsCursor() const noexcept;
    };

    namespace concepts
    {
        template <typename T>
        struct IsStaticRoute : std::false_type
        {
        };

        template <StringLiteral Route, std::size_t MaxTokens>
        struct IsStaticRoute<StaticRoute<Route, MaxTokens>> : std::true_type
        {
        };

        template <typename T>
        concept Route =
            (std::same_as<std::remove_cvref_t<T>, DynamicRoute> ||
             IsStaticRoute<std::remove_cvref_t<T>>::value) &&
            requires(const std::remove_cvref_t<T>& route)
            {
                { route.GetRouteString() } -> std::convertible_to<std::string_view>;
                { route.GetWaypoints() } -> std::convertible_to<std::span<const Waypoint>>;
            };
    } // namespace concepts
}