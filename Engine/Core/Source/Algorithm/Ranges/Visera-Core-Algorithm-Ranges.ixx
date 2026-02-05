module;
#include <Visera-Core.hpp>
#include <algorithm>
#include <ranges>
export module Visera.Core.Algorithm.Ranges;
#define VISERA_MODULE_NAME "Core.Algorithm"

export namespace Visera::Algorithm
{
	template<std::ranges::range Range, typename Predicate> requires std::indirect_unary_predicate<Predicate, std::ranges::iterator_t<Range>>
	[[nodiscard]] constexpr Bool
	NoneOf(Range&& I_Range, Predicate I_Pred)
	{
		return std::ranges::none_of(std::forward<Range>(I_Range), std::move(I_Pred));
	}

	template<std::ranges::range Range,typename Comp = std::ranges::less, typename Proj = std::identity> requires std::sortable<std::ranges::iterator_t<Range>, Comp, Proj>
	constexpr void
	Sort(Range&& I_Range, Comp I_Comp = {})
	{
		std::ranges::sort(std::forward<Range>(I_Range), std::move(I_Comp));
	}

	template<std::ranges::range Range, typename Predicate> requires std::indirect_unary_predicate<Predicate, std::ranges::iterator_t<Range>>
	[[nodiscard]] constexpr auto
	FindIf(Range&& I_Range, Predicate I_Pred)
	{
		return std::ranges::find_if(std::forward<Range>(I_Range), std::move(I_Pred));
	}

	template<std::ranges::viewable_range Range, typename Pattern> requires requires(Range&& r, Pattern&& p) { std::ranges::views::split(std::forward<Range>(r), std::forward<Pattern>(p)); }
	[[nodiscard]] constexpr auto
	Split(Range&& I_Range, Pattern&& I_Pattern)
	{
		return std::ranges::views::split(
			std::forward<Range>(I_Range),
			std::forward<Pattern>(I_Pattern));
	}
}
