module;
#include <charted/core/charted-core.hpp>

export module charted.core;

export namespace charted
{
    namespace concepts
    {
        using ::charted::concepts::Route;
    }

    using ::charted::StringLiteral;
    using ::charted::route;
    using ::charted::view;
    using ::charted::cursor;
    using ::charted::Waypoint;
    using ::charted::WaypointType;
    using ::charted::RouteView;
    using ::charted::Cursor;
    using ::charted::DynamicRoute;
    using ::charted::StaticRoute;
}
