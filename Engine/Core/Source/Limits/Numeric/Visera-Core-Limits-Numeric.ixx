module;
#include <Visera-Core.hpp>
#include <limits>
export module Visera.Core.Limits.Numeric;
#define VISERA_MODULE_NAME "Core.Limits"

export namespace Visera::Limits
{
	template<typename NumT> requires std::is_arithmetic_v<NumT>
	[[nodiscard]] constexpr NumT
    Epsilon() noexcept { return std::numeric_limits<NumT>::epsilon(); }

	template<typename NumT> requires std::is_arithmetic_v<NumT>
	[[nodiscard]] constexpr NumT
    UpperBound() noexcept { return std::numeric_limits<NumT>::max(); }

	template<typename NumT> requires std::is_arithmetic_v<NumT>
	[[nodiscard]] constexpr NumT
    LowerBound() noexcept { return std::numeric_limits<NumT>::lowest(); }
}
