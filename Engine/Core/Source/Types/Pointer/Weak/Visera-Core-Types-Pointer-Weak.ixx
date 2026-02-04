module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Pointer.Weak;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Pointer.Shared;

export namespace Visera
{
    template<typename T>
    class VISERA_CORE_API FWeakPtr
    {
        template<typename U> friend class FWeakPtr;

    public:
        /** Creates a new TSharedPtr sharing ownership if the object is still alive. */
        [[nodiscard]] TSharedPtr<T> Lock() const noexcept { return TSharedPtr<T>(Self.lock()); }
        /** @return Number of TSharedPtr instances that were sharing ownership (approximate). */
        [[nodiscard]] long GetUseCount() const noexcept { return Self.use_count(); }
        /** @return True if no TSharedPtr shares ownership. */
        [[nodiscard]] Bool IsExpired() const noexcept { return Self.expired(); }
        /** Releases the reference to the managed object. */
        void Reset() noexcept { Self.reset(); }
        /** Swaps managed objects with I_Other. */
        void Swap(FWeakPtr& I_Other) noexcept { Self.swap(I_Other.Self); }

    private:
        std::weak_ptr<T> Self;

    public:
        FWeakPtr() = default;
        FWeakPtr(std::nullptr_t) noexcept : Self() {}
        FWeakPtr(const TSharedPtr<T>& I_Shared) noexcept : Self(I_Shared.Self) {}
        FWeakPtr(TSharedPtr<T>&&) = delete;
        FWeakPtr(const FWeakPtr&) = default;
        FWeakPtr(FWeakPtr&&) noexcept = default;
        template<typename U>
        requires std::convertible_to<U*, T*>
        FWeakPtr(const FWeakPtr<U>& I_Other) noexcept : Self(I_Other.Self) {}
        template<typename U>
        requires std::convertible_to<U*, T*>
        FWeakPtr(FWeakPtr<U>&& I_Other) noexcept : Self(std::move(I_Other.Self)) {}
        FWeakPtr(std::weak_ptr<T> I_Other) noexcept : Self(std::move(I_Other)) {}

        FWeakPtr& operator=(const TSharedPtr<T>& I_Shared) noexcept { Self = I_Shared.Self; return *this; }
        FWeakPtr& operator=(const FWeakPtr&) = default;
        FWeakPtr& operator=(FWeakPtr&&) noexcept = default;
        template<typename U>
        requires std::convertible_to<U*, T*>
        FWeakPtr& operator=(const FWeakPtr<U>& I_Other) noexcept { Self = I_Other.Self; return *this; }
        template<typename U>
        requires std::convertible_to<U*, T*>
        FWeakPtr& operator=(FWeakPtr<U>&& I_Other) noexcept { Self = std::move(I_Other.Self); return *this; }
        FWeakPtr& operator=(std::nullptr_t) noexcept { Self.reset(); return *this; }
        FWeakPtr& operator=(std::weak_ptr<T> I_Other) noexcept { Self = std::move(I_Other); return *this; }

        ~FWeakPtr() = default;
    };
}
