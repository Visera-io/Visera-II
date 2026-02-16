#pragma once
#include <string_view>

#include "route/charted-route.hpp"
#include "cursor/charted-cursor.hpp"

namespace charted
{
    [[nodiscard]] inline DynamicRoute route(std::string_view path)
    {
        return DynamicRoute(path);
    }

    template <StringLiteral Path>
    [[nodiscard]] constexpr auto route()
    {
        return StaticRoute<Path>{};
    }

    template <concepts::Route TJSONRoute>
    [[nodiscard]] constexpr RouteView view(const TJSONRoute& routeValue)
    {
        return RouteView(routeValue);
    }

    template <concepts::Route TJSONRoute>
    [[nodiscard]] constexpr Cursor cursor(const TJSONRoute& routeValue)
    {
        return Cursor(routeValue);
    }
}
