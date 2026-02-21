module;
#include <Visera-Core.hpp>
#include <cstddef>
#include <functional>
#include <new>
#include <type_traits>
#include <utility>
export module Visera.Core.Types.Function;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
	template<typename Signature>
	using TFunction = std::function<Signature>;

	inline constexpr std::size_t UniqueFunctionDefaultInlineBytes = sizeof(void*) * 3;

	template<typename Signature, std::size_t InlineBytes = UniqueFunctionDefaultInlineBytes>
	class VISERA_CORE_API TUniqueFunction;

	namespace Detail
	{
		template<typename T>
		inline constexpr Bool TAlwaysFalse = False;

		template<std::size_t InlineBytes>
		struct TInlineStorage
		{
			static_assert(InlineBytes > 0, "TUniqueFunction InlineBytes must be greater than zero.");
			alignas(std::max_align_t) std::byte Bytes[InlineBytes];
		};

		template<typename F, typename R, Bool bNoexceptCall, typename... TArgs>
		inline constexpr Bool TIsInvocableForSignature = bNoexceptCall
			? std::is_nothrow_invocable_r_v<R, F, TArgs...>
			: std::is_invocable_r_v<R, F, TArgs...>;

		template<typename TCallable, std::size_t InlineBytes>
		consteval void
		ValidateInlineCapacity()
		{
			static_assert(std::is_move_constructible_v<TCallable>,
				"TUniqueFunction callable must be move constructible.");
			static_assert(std::is_nothrow_move_constructible_v<TCallable>,
				"TUniqueFunction callable must be nothrow move constructible.");
			static_assert(alignof(TCallable) <= alignof(std::max_align_t),
				"TUniqueFunction callable alignment exceeds storage alignment.");

			if constexpr (InlineBytes == UniqueFunctionDefaultInlineBytes && sizeof(TCallable) > UniqueFunctionDefaultInlineBytes)
			{
				static_assert(TAlwaysFalse<TCallable>,
					"TUniqueFunction callable exceeds default inline capacity. "
					"Please explicitly specify a larger InlineBytes template argument.");
			}

			static_assert(sizeof(TCallable) <= InlineBytes,
				"TUniqueFunction callable exceeds configured InlineBytes. Increase InlineBytes.");
		}
	}

#define VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(SIGNATURE, CALL_OPERATOR_QUAL, IS_CONST_CALL, IS_RVALUE_CALL, IS_NOEXCEPT_CALL, NOEXCEPT_SPEC) \
	template<typename R, typename... TArgs, std::size_t InlineBytes> \
	class VISERA_CORE_API TUniqueFunction<SIGNATURE, InlineBytes> \
	{ \
	private: \
		using FInvokeStoragePtr = std::conditional_t<IS_CONST_CALL, const void*, void*>; \
 \
		struct FOps \
		{ \
			R (*Invoke)(FInvokeStoragePtr, TArgs&&...) NOEXCEPT_SPEC; \
			void (*MoveConstruct)(void*, void*) noexcept; \
			void (*Destroy)(void*) noexcept; \
		}; \
 \
		template<typename F> \
		using TInvokeObjectType = std::conditional_t<IS_RVALUE_CALL, \
			std::conditional_t<IS_CONST_CALL, const F&&, F&&>, \
			std::conditional_t<IS_CONST_CALL, const F&, F&>>; \
 \
		template<typename F> \
		static R \
		InvokeTrampoline(FInvokeStoragePtr I_Storage, TArgs&&... I_Args) NOEXCEPT_SPEC \
		{ \
			if constexpr (IS_CONST_CALL) \
			{ \
				const auto& CallableRef = *static_cast<const F*>(I_Storage); \
				if constexpr (IS_RVALUE_CALL) \
				{ \
					return std::invoke(std::move(CallableRef), std::forward<TArgs>(I_Args)...); \
				} \
				else \
				{ \
					return std::invoke(CallableRef, std::forward<TArgs>(I_Args)...); \
				} \
			} \
			else \
			{ \
				auto& CallableRef = *static_cast<F*>(I_Storage); \
				if constexpr (IS_RVALUE_CALL) \
				{ \
					return std::invoke(std::move(CallableRef), std::forward<TArgs>(I_Args)...); \
				} \
				else \
				{ \
					return std::invoke(CallableRef, std::forward<TArgs>(I_Args)...); \
				} \
			} \
		} \
 \
		template<typename F> \
		static void \
		MoveConstructTrampoline(void* I_Dst, void* I_Src) noexcept \
		{ \
			auto* Src = static_cast<F*>(I_Src); \
			::new (I_Dst) F(std::move(*Src)); \
			Src->~F(); \
		} \
 \
		template<typename F> \
		static void \
		DestroyTrampoline(void* I_Storage) noexcept \
		{ \
			static_cast<F*>(I_Storage)->~F(); \
		} \
 \
		template<typename F> \
		static const FOps& \
		GetOps() noexcept \
		{ \
			static const FOps Ops \
			{ \
				&InvokeTrampoline<F>, \
				&MoveConstructTrampoline<F>, \
				&DestroyTrampoline<F> \
			}; \
			return Ops; \
		} \
 \
		[[nodiscard]] inline void* \
		RawStorage() noexcept { return static_cast<void*>(Storage.Bytes); } \
 \
		[[nodiscard]] inline const void* \
		RawStorage() const noexcept { return static_cast<const void*>(Storage.Bytes); } \
 \
		template<typename F, typename... TCtorArgs> \
		void \
		ConstructFromCallable(TCtorArgs&&... I_Args) \
		{ \
			Detail::ValidateInlineCapacity<F, InlineBytes>(); \
			::new (RawStorage()) F(std::forward<TCtorArgs>(I_Args)...); \
			Ops = &GetOps<F>(); \
		} \
 \
		void \
		MoveFrom(TUniqueFunction&& I_Other) noexcept \
		{ \
			if (!I_Other.Ops) { return; } \
			Ops = I_Other.Ops; \
			Ops->MoveConstruct(RawStorage(), I_Other.RawStorage()); \
			I_Other.Ops = nullptr; \
		} \
 \
	public: \
		static constexpr std::size_t InlineStorageBytes = InlineBytes; \
 \
		TUniqueFunction() noexcept = default; \
		TUniqueFunction(std::nullptr_t) noexcept {} \
 \
		~TUniqueFunction() { Reset(); } \
 \
		TUniqueFunction(const TUniqueFunction&) = delete; \
		TUniqueFunction& operator=(const TUniqueFunction&) = delete; \
 \
		TUniqueFunction(TUniqueFunction&& I_Other) noexcept \
		{ \
			MoveFrom(std::move(I_Other)); \
		} \
 \
		TUniqueFunction& \
		operator=(TUniqueFunction&& I_Other) noexcept \
		{ \
			if (this != &I_Other) \
			{ \
				Reset(); \
				MoveFrom(std::move(I_Other)); \
			} \
			return *this; \
		} \
 \
		template<typename F> \
			requires (!std::same_as<std::remove_cvref_t<F>, TUniqueFunction> && \
					  Detail::TIsInvocableForSignature<TInvokeObjectType<std::remove_cvref_t<F>>, R, IS_NOEXCEPT_CALL, TArgs...>) \
		TUniqueFunction(F&& I_Callable) \
		{ \
			using TCallable = std::remove_cvref_t<F>; \
			ConstructFromCallable<TCallable>(std::forward<F>(I_Callable)); \
		} \
 \
		TUniqueFunction& \
		operator=(std::nullptr_t) noexcept \
		{ \
			Reset(); \
			return *this; \
		} \
 \
		template<typename F> \
			requires (!std::same_as<std::remove_cvref_t<F>, TUniqueFunction> && \
					  Detail::TIsInvocableForSignature<TInvokeObjectType<std::remove_cvref_t<F>>, R, IS_NOEXCEPT_CALL, TArgs...>) \
		TUniqueFunction& \
		operator=(F&& I_Callable) \
		{ \
			using TCallable = std::remove_cvref_t<F>; \
			Reset(); \
			ConstructFromCallable<TCallable>(std::forward<F>(I_Callable)); \
			return *this; \
		} \
 \
		void \
		Reset() noexcept \
		{ \
			if (!Ops) { return; } \
			Ops->Destroy(RawStorage()); \
			Ops = nullptr; \
		} \
 \
		[[nodiscard]] explicit \
		operator Bool() const noexcept { return Ops != nullptr; } \
 \
		R \
		operator()(TArgs... I_Args) CALL_OPERATOR_QUAL NOEXCEPT_SPEC \
		{ \
			VISERA_ASSERT(Ops != nullptr); \
			if constexpr (IS_CONST_CALL) \
			{ \
				return Ops->Invoke(RawStorage(), std::forward<TArgs>(I_Args)...); \
			} \
			else \
			{ \
				return Ops->Invoke(RawStorage(), std::forward<TArgs>(I_Args)...); \
			} \
		} \
 \
	private: \
		Detail::TInlineStorage<InlineBytes> Storage {}; \
		const FOps* Ops { nullptr }; \
	};

	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...),, False, False, False, )
	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) noexcept,, False, False, True, noexcept)

	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) const, const, True, False, False, )
	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) const noexcept, const, True, False, True, noexcept)

	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) &, &, False, False, False, )
	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) & noexcept, &, False, False, True, noexcept)

	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) const &, const &, True, False, False, )
	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) const & noexcept, const &, True, False, True, noexcept)

	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) &&, &&, False, True, False, )
	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) && noexcept, &&, False, True, True, noexcept)

	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) const &&, const &&, True, True, False, )
	VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION(R(TArgs...) const && noexcept, const &&, True, True, True, noexcept)

#undef VISERA_DEFINE_TUNIQUE_FUNCTION_SPECIALIZATION
}