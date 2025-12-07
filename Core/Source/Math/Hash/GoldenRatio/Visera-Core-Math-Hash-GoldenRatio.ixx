module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Hash.GoldenRatio;
#define VISERA_MODULE_NAME "Core.Math"

export namespace Visera::Math
{
	constexpr UInt64 GoldenRatio = 0x9e3779b97f4a7c15ull;

	template<class T> [[nodiscard]] UInt64
	GoldenRatioHash(UInt64 I_Seed, const T& I_Value) noexcept
	{
        using  DecayType = std::decay_t<T>;
		UInt64 Value{0};

		if constexpr (std::is_enum_v<DecayType>)
		{
			using U = std::underlying_type_t<DecayType>;
			Value = static_cast<UInt64>(static_cast<U>(I_Value));
		}
		else if constexpr (std::is_integral_v<DecayType> || std::is_pointer_v<DecayType>)
		{
			Value = static_cast<UInt64>(I_Value);
		}
		else
		{
			Value = static_cast<UInt64>(std::hash<DecayType>{}(I_Value));
		}

		return I_Seed ^ (Value + GoldenRatio + (I_Seed << 6) + (I_Seed >> 2));
	}

	template<class... Ts> [[nodiscard]] UInt64
	GoldenRatioHashCombine(UInt64 I_Seed, const Ts&... I_Values) noexcept
	{
		((I_Seed = GoldenRatioHash(I_Seed, I_Values)), ...);
		return I_Seed;
	}
} // namespace Visera