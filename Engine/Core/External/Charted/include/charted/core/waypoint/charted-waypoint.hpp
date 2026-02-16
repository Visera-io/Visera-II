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

namespace charted
{
    enum class WaypointType : std::uint8_t
    {
        Key,
        Index
    };

    struct Waypoint
    {
        const char*      Ptr   { nullptr };
        std::uint16_t    Length{ 0 };
        WaypointType   Type  { WaypointType::Key };
        std::uint32_t    Index { 0 };

        [[nodiscard]] std::string_view GetString() const noexcept
        {
            return (Ptr != nullptr && Length > 0)
                ? std::string_view(Ptr, static_cast<std::size_t>(Length))
                : std::string_view{};
        }
    };

    static_assert(sizeof(Waypoint) == 16, "Waypoint expected to be 16 bytes.");
}