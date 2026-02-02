module;
#include <Visera-Core.hpp>
#include <optional>
export module Visera.Core.Types.Optional;
#define VISERA_MODULE_NAME "Core.Types.Optional"

export namespace Visera
{
    /** Sentinel type for constructing an optional with no value (unset). */
    struct FNullOptType
    {
        struct FPrivateTag {};
        explicit constexpr FNullOptType(FPrivateTag = {}) {}
    };
    inline constexpr FNullOptType NullOpt{};

    /** Tag type for in-place construction of the optional value. */
    struct FInPlaceType
    {
        struct FPrivateTag {};
        explicit constexpr FInPlaceType(FPrivateTag = {}) {}
    };
    inline constexpr FInPlaceType InPlace{};

    /** Sentinel type for types that support intrusive unset state (no separate bool). */
    struct FIntrusiveUnsetOptionalState {};

    /** Trait: True if T can represent unset via FIntrusiveUnsetOptionalState (constructible and comparable). */
    template<typename T> inline constexpr Bool
    HasIntrusiveUnsetOptionalState = False;

    namespace Private
    {
        struct FEmpty {};

        struct FOptional
        {
            template<typename Derived>
            [[nodiscard]] static constexpr Bool HasValue(const Derived* I_This)
            {
                if constexpr (Derived::bUsingIntrusiveUnsetState)
                {
                    return !(I_This->TypedValue == FIntrusiveUnsetOptionalState{});
                }
                else
                {
                    return I_This->bHasValue;
                }
            }
        };
    }

    /**
     * Optional value: when HasValue() is True, GetValue() is meaningful; otherwise it is not.
     */
    template<typename OptionalType>
    class VISERA_CORE_API TOptional
    {
    public:
        using ElementType = OptionalType;

        /** @return True when the value is set; False if GetValue() is undefined. */
        [[nodiscard]] constexpr Bool HasValue() const
        {
            return FOptional::HasValue(this);
        }

        [[nodiscard]] VISERA_FORCEINLINE explicit constexpr operator bool() const
        {
            return HasValue();
        }

        /** @return The optional value; undefined when HasValue() is False. */
        [[nodiscard]] constexpr OptionalType& GetValue()
        {
            VISERA_ASSERT(HasValue() && "It is an error to call GetValue() on an unset TOptional. Check HasValue() or use Get(DefaultValue) instead.");
            return TypedValue;
        }

        [[nodiscard]] VISERA_FORCEINLINE constexpr const OptionalType& GetValue() const
        {
            return const_cast<TOptional*>(this)->GetValue();
        }

        [[nodiscard]] constexpr OptionalType* operator->()
        {
            return std::addressof(GetValue());
        }

        [[nodiscard]] VISERA_FORCEINLINE constexpr const OptionalType* operator->() const
        {
            return const_cast<TOptional*>(this)->operator->();
        }

        [[nodiscard]] constexpr OptionalType& operator*()
        {
            return GetValue();
        }

        [[nodiscard]] VISERA_FORCEINLINE constexpr const OptionalType& operator*() const
        {
            return const_cast<TOptional*>(this)->operator*();
        }

        /** @return The value when set; I_DefaultValue otherwise. */
        [[nodiscard]] constexpr const OptionalType& Get(const OptionalType& I_DefaultValue) const
        {
            return HasValue() ? TypedValue : I_DefaultValue;
        }

        /** @return Pointer to the value when set, nullptr otherwise. */
        [[nodiscard]] constexpr OptionalType* GetPtrOrNull()
        {
            return HasValue() ? std::addressof(TypedValue) : nullptr;
        }

        [[nodiscard]] VISERA_FORCEINLINE constexpr const OptionalType* GetPtrOrNull() const
        {
            return const_cast<TOptional*>(this)->GetPtrOrNull();
        }

        void Reset()
        {
            if (HasValue())
            {
                DestroyValue();
                if constexpr (bUsingIntrusiveUnsetState)
                {
                    std::construct_at(std::addressof(TypedValue), FIntrusiveUnsetOptionalState{});
                }
                else
                {
                    bHasValue = False;
                }
            }
        }

        template<typename... ArgsType>
        OptionalType& Emplace(ArgsType&&... I_Args)
        {
            if constexpr (bUsingIntrusiveUnsetState)
            {
                DestroyValue();
            }
            else
            {
                if (HasValue())
                {
                    DestroyValue();
                }
            }

            OptionalType* const Result = std::construct_at(std::addressof(TypedValue), std::forward<ArgsType>(I_Args)...);

            if constexpr (!bUsingIntrusiveUnsetState)
            {
                bHasValue = True;
            }
            else
            {
                VISERA_ASSERT(HasValue() && "TOptional::Emplace(...) - optionals should not be unset by emplacement");
            }

            return *Result;
        }

        [[nodiscard]] friend constexpr Bool operator==(const TOptional& I_Lhs, const TOptional& I_Rhs)
        {
            const Bool bLhsSet = I_Lhs.HasValue();
            const Bool bRhsSet = I_Rhs.HasValue();
            if (bLhsSet != bRhsSet)
            {
                return False;
            }
            if (!bLhsSet)
            {
                return True;
            }
            return I_Lhs.TypedValue == I_Rhs.TypedValue;
        }

        [[nodiscard]] friend constexpr Bool operator!=(const TOptional& I_Lhs, const TOptional& I_Rhs)
        {
            return !(I_Lhs == I_Rhs);
        }

    private:
        static constexpr bool bUsingIntrusiveUnsetState = HasIntrusiveUnsetOptionalState<OptionalType>;
        using  FOptional = Private::FOptional;
        friend FOptional;

        union
        {
            OptionalType TypedValue;
        };
        [[no_unique_address]] std::conditional_t<bUsingIntrusiveUnsetState, Private::FEmpty, Bool>
        bHasValue = False;

        constexpr void DestroyValue()
        {
            std::destroy_at(std::addressof(TypedValue));
        }

    public:
        /** Construct with a value (copy). */
        [[nodiscard]] constexpr TOptional(const OptionalType& I_Value)
            : TOptional(InPlace, I_Value)
        {}

        /** Construct with a value (move). */
        [[nodiscard]] constexpr TOptional(OptionalType&& I_Value)
            : TOptional(InPlace, std::move(I_Value))
        {}

        /** Construct in-place with arguments. */
        template<typename... ArgTypes>
        [[nodiscard]] explicit constexpr TOptional(FInPlaceType, ArgTypes&&... I_Args)
            : TypedValue(std::forward<ArgTypes>(I_Args)...)
        {
            if constexpr (!bUsingIntrusiveUnsetState)
            {
                bHasValue = True;
            }
            else
            {
                VISERA_ASSERT(HasValue() && "TOptional::TOptional(InPlace, ...) - optionals should not be unset by emplacement");
            }
        }

        /** Construct unset (from NullOpt). */
        [[nodiscard]] constexpr TOptional(FNullOptType) : TOptional() {}

        /** Construct unset (from std::nullopt); enables compatibility with std::nullopt. */
        [[nodiscard]] constexpr TOptional(std::nullopt_t) : TOptional() {}

        /** Construct unset (intrusive: value holds sentinel state). */
        [[nodiscard]] constexpr TOptional() requires bUsingIntrusiveUnsetState
            : TypedValue(FIntrusiveUnsetOptionalState{})
        {}

        /** Construct unset (non-intrusive). */
        [[nodiscard]] constexpr TOptional() requires (!bUsingIntrusiveUnsetState)
        {}

        /** Destructor: trivial when element type is trivially destructible. */
        constexpr ~TOptional() requires std::is_trivially_destructible_v<OptionalType> = default;

        constexpr ~TOptional() requires (!std::is_trivially_destructible_v<OptionalType>)
        {
            if (HasValue())
            {
                DestroyValue();
            }
        }

        /** Copy constructor. */
        [[nodiscard]] TOptional(const TOptional& I_Other) : TOptional()
        {
            const Bool bLocalHasValue = I_Other.HasValue();
            if constexpr (!bUsingIntrusiveUnsetState)
            {
                bHasValue = bLocalHasValue;
            }
            if (bLocalHasValue)
            {
                std::construct_at(std::addressof(TypedValue), I_Other.TypedValue);
            }
        }

        /** Move constructor. */
        [[nodiscard]] TOptional(TOptional&& I_Other) noexcept : TOptional()
        {
            const Bool bLocalHasValue = I_Other.HasValue();
            if constexpr (!bUsingIntrusiveUnsetState)
            {
                bHasValue = bLocalHasValue;
            }
            if (bLocalHasValue)
            {
                std::construct_at(std::addressof(TypedValue), std::move(I_Other.TypedValue));
            }
        }

        TOptional& operator=(const TOptional& I_Other)
        {
            if (std::addressof(I_Other) != this)
            {
                if (I_Other.HasValue())
                {
                    Emplace(I_Other.GetValue());
                }
                else
                {
                    Reset();
                }
            }
            return *this;
        }

        TOptional& operator=(TOptional&& I_Other) noexcept
        {
            if (std::addressof(I_Other) != this)
            {
                if (I_Other.HasValue())
                {
                    Emplace(std::move(I_Other.GetValue()));
                }
                else
                {
                    Reset();
                }
            }
            return *this;
        }

        TOptional& operator=(const OptionalType& I_Value)
        {
            if (std::addressof(I_Value) != std::addressof(TypedValue))
            {
                Emplace(I_Value);
            }
            return *this;
        }

        TOptional& operator=(OptionalType&& I_Value)
        {
            if (std::addressof(I_Value) != std::addressof(TypedValue))
            {
                Emplace(std::move(I_Value));
            }
            return *this;
        }
    };
}
