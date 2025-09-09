module;
#include <Visera-Core.hpp>
export module Visera.Core.Hash;
#define VISERA_MODULE_NAME "Core.Hash"
import Visera.Core.Hash.CityHash;

export namespace Visera
{
	static_assert(std::is_same_v<UInt128, Google::uint128>);

	/*<<City Hash by Google>>*/
	inline auto
	CityHash64(FStringView I_StringView)				-> UInt64	{ return Google::CityHash64(I_StringView.data(), I_StringView.size()); }
	inline auto
	CityHash64(FStringView I_StringView, UInt64 I_Seed)	-> UInt64	{ return Google::CityHash64WithSeed(I_StringView.data(), I_StringView.size(), I_Seed); }
	inline auto
	CityHash64(const char* I_String, UInt64 I_Length)	-> UInt64	{ return Google::CityHash64(I_String, I_Length); }
	inline auto
	CityHash64(const char* I_String, UInt64 I_Length, UInt64 I_Seed) -> UInt64	{ return Google::CityHash64WithSeed(I_String, I_Length, I_Seed); }

	inline auto
	CityHash128(FStringView I_StringView)					-> UInt128	{ return Google::CityHash128(I_StringView.data(), I_StringView.size()); }
	inline auto
	CityHash128(FStringView I_StringView, UInt128 I_Seed)	-> UInt128	{ return Google::CityHash128WithSeed(I_StringView.data(), I_StringView.size(), I_Seed); }
	inline auto
	CityHash128(const char* I_String, UInt64 I_Length)		-> UInt128	{ return Google::CityHash128(I_String, I_Length); }
	inline auto
	CityHash128(const char* I_String, UInt64 I_Length, UInt128 I_Seed) -> UInt128 { return Google::CityHash128WithSeed(I_String, I_Length, I_Seed); }

} // namespace Visera