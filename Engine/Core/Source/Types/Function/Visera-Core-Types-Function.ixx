module;
#include <Visera-Core.hpp>
#include <functional>
export module Visera.Core.Types.Function;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
	template<typename Signature>
	using TFunction = std::function<Signature>;

	template<typename Signature>
	using TUniqueFunction = std::move_only_function<Signature>;
}