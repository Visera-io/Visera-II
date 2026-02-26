module;
#include <Visera-Core.hpp>
#include <memory>
export module Visera.Core.Meta.Cast;
#define VISERA_MODULE_NAME "Core.Meta"
import Visera.Core.Types.Pointer.Shared;
import Visera.Core.Types.Pointer.Unique;

export namespace Visera
{
	/** TSharedPtr<U> -> TSharedPtr<T> via dynamic_pointer_cast; returns empty on failure. */
	template<typename T, typename U>
	[[nodiscard]] TSharedPtr<T>
    TryCast(const TSharedPtr<U>& I_Ptr) noexcept
	{ return TSharedPtr<T>(std::dynamic_pointer_cast<T>(I_Ptr.GetNative())); }

	/** Cast<T>: uses TryCast<T>; returns nullptr on failure. */
	template<typename T, typename U>
	[[nodiscard]] TSharedPtr<T>
    Cast(const TSharedPtr<U>& I_Ptr) noexcept
	{ return TryCast<T>(I_Ptr); }

	/** TUniquePtr<U> -> TUniquePtr<T> via dynamic_cast; takes ownership on success, discards on failure. */
	template<typename T, typename U, typename Deleter>
	[[nodiscard]] TUniquePtr<T>
	TryCast(TUniquePtr<U, Deleter>&& I_Ptr) noexcept
	{
		U* raw = I_Ptr.Release();
		T* casted = dynamic_cast<T*>(raw);
		if (!casted)
		{
			if (raw) { TUniquePtr<U> reclaim(raw); }
			return TUniquePtr<T>();
		}
		return TUniquePtr<T>(casted);
	}

	/** Cast<T>: uses TryCast<T>; returns empty on failure. */
	template<typename T, typename U, typename Deleter>
	[[nodiscard]] TUniquePtr<T>
	Cast(TUniquePtr<U, Deleter>&& I_Ptr) noexcept
	{ return TryCast<T>(std::move(I_Ptr)); }

	/** Raw pointer U* -> T* via dynamic_cast; returns nullptr on failure. */
	template<typename T, typename U>
	[[nodiscard]] T*
    TryCast(U* I_Ptr) noexcept
	{ return dynamic_cast<T*>(I_Ptr); }

	/** Cast<T>: uses TryCast<T>; returns nullptr on failure. */
	template<typename T, typename U>
	[[nodiscard]] T*
    Cast(U* I_Ptr) noexcept
	{ return TryCast<T>(I_Ptr); }
}