module;
#include <Visera-Core.hpp>
#include <memory>
export module Visera.Core.Types.Pointer.Weak;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Pointer.Shared;

export namespace Visera
{
    template<typename T>
    class VISERA_CORE_API TWeakPtr
    {
        template<typename U> friend class TWeakPtr;

    public:
        /** Creates a new TSharedPtr sharing ownership if the object is still alive. */
        [[nodiscard]] TSharedPtr<T> Lock() const noexcept { return TSharedPtr<T>(Self.lock()); }
        [[nodiscard]] std::weak_ptr<T>& GetNative() noexcept { return Self; }
        [[nodiscard]] const std::weak_ptr<T>& GetNative() const noexcept { return Self; }
        /** @return Number of TSharedPtr instances that were sharing ownership (approximate). */
        [[nodiscard]] long GetUseCount() const noexcept { return Self.use_count(); }
        /** @return True if no TSharedPtr shares ownership. */
        [[nodiscard]] Bool IsExpired() const noexcept { return Self.expired(); }
        /** Releases the reference to the managed object. */
        void Reset() noexcept { Self.reset(); }
        /** Swaps managed objects with I_Other. */
        void Swap(TWeakPtr& I_Other) noexcept { Self.swap(I_Other.Self); }

        [[nodiscard]] explicit operator bool() const noexcept { return !IsExpired(); }

        [[nodiscard]] friend Bool operator==(const TWeakPtr& I_Lhs, const TWeakPtr& I_Rhs) noexcept
        {
            return !I_Lhs.Self.owner_before(I_Rhs.Self) &&
                   !I_Rhs.Self.owner_before(I_Lhs.Self);
        }

        [[nodiscard]] friend Bool operator!=(const TWeakPtr& I_Lhs, const TWeakPtr& I_Rhs) noexcept
        { return !(I_Lhs == I_Rhs); }

        [[nodiscard]] friend Bool operator==(const TWeakPtr& I_Lhs, std::nullptr_t) noexcept
        { return I_Lhs.IsExpired(); }

        [[nodiscard]] friend Bool operator!=(const TWeakPtr& I_Lhs, std::nullptr_t) noexcept
        { return !(I_Lhs == nullptr); }

        [[nodiscard]] friend Bool operator==(std::nullptr_t, const TWeakPtr& I_Rhs) noexcept
        { return I_Rhs.IsExpired(); }

        [[nodiscard]] friend Bool operator!=(std::nullptr_t, const TWeakPtr& I_Rhs) noexcept
        { return !(nullptr == I_Rhs); }

        /** Strict weak ordering based on ownership. */
        [[nodiscard]] friend Bool operator<(const TWeakPtr& I_Lhs, const TWeakPtr& I_Rhs) noexcept
        { return I_Lhs.Self.owner_before(I_Rhs.Self); }

    private:
        std::weak_ptr<T> Self;

    public:
        TWeakPtr() = default;
        TWeakPtr(std::nullptr_t) noexcept : Self() {}
        TWeakPtr(const TSharedPtr<T>& I_Shared) noexcept : Self(I_Shared.GetNative()) {}
        TWeakPtr(TSharedPtr<T>&& I_Shared) noexcept : Self(I_Shared.GetNative()) {}
        template<typename U>
        requires std::convertible_to<U*, T*>
        TWeakPtr(const TSharedPtr<U>& I_Shared) noexcept : Self(I_Shared.GetNative()) {}
        template<typename U>
        requires std::convertible_to<U*, T*>
        TWeakPtr(TSharedPtr<U>&& I_Shared) noexcept : Self(I_Shared.GetNative()) {}
        TWeakPtr(const TWeakPtr&) = default;
        TWeakPtr(TWeakPtr&&) noexcept = default;
        template<typename U>
        requires std::convertible_to<U*, T*>
        TWeakPtr(const TWeakPtr<U>& I_Other) noexcept : Self(I_Other.GetNative()) {}
        template<typename U>
        requires std::convertible_to<U*, T*>
        TWeakPtr(TWeakPtr<U>&& I_Other) noexcept : Self(std::move(I_Other.GetNative())) {}
        TWeakPtr(std::weak_ptr<T> I_Other) noexcept : Self(std::move(I_Other)) {}

        TWeakPtr& operator=(const TSharedPtr<T>& I_Shared) noexcept { Self = I_Shared.GetNative(); return *this; }
        template<typename U>
        requires std::convertible_to<U*, T*>
        TWeakPtr& operator=(const TSharedPtr<U>& I_Shared) noexcept { Self = I_Shared.GetNative(); return *this; }
        TWeakPtr& operator=(const TWeakPtr&) = default;
        TWeakPtr& operator=(TWeakPtr&&) noexcept = default;
        template<typename U>
        requires std::convertible_to<U*, T*>
        TWeakPtr& operator=(const TWeakPtr<U>& I_Other) noexcept { Self = I_Other.GetNative(); return *this; }
        template<typename U>
        requires std::convertible_to<U*, T*>
        TWeakPtr& operator=(TWeakPtr<U>&& I_Other) noexcept { Self = std::move(I_Other.GetNative()); return *this; }
        TWeakPtr& operator=(std::nullptr_t) noexcept { Self.reset(); return *this; }
        TWeakPtr& operator=(std::weak_ptr<T> I_Other) noexcept { Self = std::move(I_Other); return *this; }

        ~TWeakPtr() = default;
    };
}
