#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>

#include "../route/charted-route.hpp"

namespace charted
{
    class Cursor
    {
    public:
        constexpr Cursor() noexcept = default;
        constexpr explicit Cursor(RouteView view) noexcept
            : View(view)
        {
        }

        template <typename TJSONRoute>
        constexpr explicit Cursor(const TJSONRoute& route)
            requires requires(const TJSONRoute& value)
            {
                { value.GetRouteString() } -> std::convertible_to<std::string_view>;
                { value.GetWaypoints() } -> std::convertible_to<std::span<const Waypoint>>;
            }
            : View(route)
        {
        }

        [[nodiscard]] constexpr bool HasNext() const noexcept
        {
            return Index < View.GetWaypoints().size();
        }

        [[nodiscard]] constexpr const Waypoint* Next() noexcept
        {
            if (!HasNext())
            {
                return nullptr;
            }
            return std::addressof(View.GetWaypoints()[Index++]);
        }

        [[nodiscard]] constexpr const Waypoint* Peek(std::size_t offset = 0) const noexcept
        {
            if (offset >= Remaining())
            {
                return nullptr;
            }
            return std::addressof(View.GetWaypoints()[Index + offset]);
        }

        constexpr void Reset() noexcept { Index = 0; }

        [[nodiscard]] constexpr std::size_t Remaining() const noexcept
        {
            return View.GetWaypoints().size() - Index;
        }

        [[nodiscard]] constexpr std::size_t Position() const noexcept
        {
            return Index;
        }

        [[nodiscard]] constexpr bool NextIsKey() const noexcept
        {
            const Waypoint* w = Peek();
            return w != nullptr && w->Type == WaypointType::Key;
        }

        [[nodiscard]] constexpr bool NextIsIndex() const noexcept
        {
            const Waypoint* w = Peek();
            return w != nullptr && w->Type == WaypointType::Index;
        }

        [[nodiscard]] constexpr std::optional<std::string_view> NextKey() noexcept
        {
            const Waypoint* w = Peek();
            if (w == nullptr || w->Type != WaypointType::Key)
            {
                return std::nullopt;
            }
            ++Index;
            return w->GetString();
        }

        [[nodiscard]] constexpr std::optional<std::uint32_t> NextIndex() noexcept
        {
            const Waypoint* w = Peek();
            if (w == nullptr || w->Type != WaypointType::Index)
            {
                return std::nullopt;
            }
            ++Index;
            return w->Index;
        }

        [[nodiscard]] constexpr bool ConsumeKey(std::string_view expected) noexcept
        {
            const Waypoint* w = Peek();
            if (w == nullptr || w->Type != WaypointType::Key || w->GetString() != expected)
            {
                return false;
            }
            ++Index;
            return true;
        }

        [[nodiscard]] constexpr bool ConsumeIndex(std::uint32_t expected) noexcept
        {
            const Waypoint* w = Peek();
            if (w == nullptr || w->Type != WaypointType::Index || w->Index != expected)
            {
                return false;
            }
            ++Index;
            return true;
        }

        [[nodiscard]] constexpr const RouteView& GetView() const noexcept { return View; }

    private:
        RouteView    View{};
        std::size_t  Index{ 0 };
    };

    inline Cursor DynamicRoute::AsCursor() const noexcept
    {
        return Cursor(AsView());
    }

    template <StringLiteral Route, std::size_t MaxTokens>
    constexpr Cursor StaticRoute<Route, MaxTokens>::AsCursor() const noexcept
    {
        return Cursor(AsView());
    }
}
