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

	/** Comparer that uses operator<. Used as default for BinarySearch so user types with operator< work (e.g. across namespaces). */
	struct Less
	{
		template<typename Left, typename Right>
		[[nodiscard]] constexpr Bool operator()(Left&& I_Left, Right&& I_Right) const
		{
			return static_cast<Bool>(std::forward<Left>(I_Left) < std::forward<Right>(I_Right));
		}
	};

	/** Binary search: returns the subrange of elements equal to I_Value in the sorted range (O(log N)). Proj projects range elements for comparison; Comp defaults to Less (operator<). */
	template<std::ranges::forward_range Range, typename T, typename Proj = std::identity, typename Comp = Less>
		requires std::indirect_strict_weak_order<Comp, std::projected<std::ranges::iterator_t<Range>, Proj>, const T*>
		&& std::indirect_strict_weak_order<Comp, const T*, std::projected<std::ranges::iterator_t<Range>, Proj>>
	[[nodiscard]] constexpr auto
	BinarySearch(Range&& I_Range, const T& I_Value, Proj I_Proj = {}, Comp I_Comp = {})
	{
		return std::ranges::equal_range(std::forward<Range>(I_Range), I_Value, std::move(I_Comp), std::move(I_Proj));
	}

	template<std::ranges::range Range, typename T>
	constexpr void
	Fill(Range&& I_Range, const T& I_Value)
	{
		std::ranges::fill(std::forward<Range>(I_Range), I_Value);
	}
}
