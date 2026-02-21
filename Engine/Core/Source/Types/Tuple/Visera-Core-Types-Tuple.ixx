module;
#include <Visera-Core.hpp>
#include <tuple>
#include <utility>
export module Visera.Core.Types.Tuple;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
	template<typename T1, typename T2>
	using TPair = std::pair<T1, T2>;

	template<typename... Args>
	using TTuple = std::tuple<Args...>;

	template<typename T1, typename T2>
	[[nodiscard]] constexpr TPair<std::unwrap_ref_decay_t<T1>, std::unwrap_ref_decay_t<T2>>
	MakePair(T1&& I_First, T2&& I_Second)
	{
		return std::make_pair(std::forward<T1>(I_First), std::forward<T2>(I_Second));
	}

	template<typename... Args>
	[[nodiscard]] constexpr TTuple<std::unwrap_ref_decay_t<Args>...>
	MakeTuple(Args&&... I_Args)
	{
		return std::make_tuple(std::forward<Args>(I_Args)...);
	}

	template<size_t I, typename... Args>
	[[nodiscard]] constexpr decltype(auto) Get(TTuple<Args...>& I_Tuple) noexcept
	{
		return std::get<I>(I_Tuple);
	}

	template<size_t I, typename... Args>
	[[nodiscard]] constexpr decltype(auto) Get(TTuple<Args...>&& I_Tuple) noexcept
	{
		return std::get<I>(std::move(I_Tuple));
	}

	template<size_t I, typename... Args>
	[[nodiscard]] constexpr decltype(auto) Get(const TTuple<Args...>& I_Tuple) noexcept
	{
		return std::get<I>(I_Tuple);
	}

	template<size_t I, typename... Args>
	[[nodiscard]] constexpr decltype(auto) Get(const TTuple<Args...>&& I_Tuple) noexcept
	{
		return std::get<I>(std::move(I_Tuple));
	}

	template<typename T, typename... Args>
	[[nodiscard]] constexpr decltype(auto) Get(TTuple<Args...>& I_Tuple) noexcept
	{
		return std::get<T>(I_Tuple);
	}

	template<typename T, typename... Args>
	[[nodiscard]] constexpr decltype(auto) Get(TTuple<Args...>&& I_Tuple) noexcept
	{
		return std::get<T>(std::move(I_Tuple));
	}

	template<typename T, typename... Args>
	[[nodiscard]] constexpr decltype(auto) Get(const TTuple<Args...>& I_Tuple) noexcept
	{
		return std::get<T>(I_Tuple);
	}

	template<typename T, typename... Args>
	[[nodiscard]] constexpr decltype(auto) Get(const TTuple<Args...>&& I_Tuple) noexcept
	{
		return std::get<T>(std::move(I_Tuple));
	}

	template<size_t I, typename T1, typename T2>
	[[nodiscard]] constexpr decltype(auto) Get(TPair<T1, T2>& I_Pair) noexcept
	{
		return std::get<I>(I_Pair);
	}

	template<size_t I, typename T1, typename T2>
	[[nodiscard]] constexpr decltype(auto) Get(TPair<T1, T2>&& I_Pair) noexcept
	{
		return std::get<I>(std::move(I_Pair));
	}

	template<size_t I, typename T1, typename T2>
	[[nodiscard]] constexpr decltype(auto) Get(const TPair<T1, T2>& I_Pair) noexcept
	{
		return std::get<I>(I_Pair);
	}

	template<size_t I, typename T1, typename T2>
	[[nodiscard]] constexpr decltype(auto) Get(const TPair<T1, T2>&& I_Pair) noexcept
	{
		return std::get<I>(std::move(I_Pair));
	}

	template<typename... Args>
	[[nodiscard]] constexpr TTuple<Args&...> Tie(Args&... I_Args) noexcept
	{
		return std::tie(I_Args...);
	}
}