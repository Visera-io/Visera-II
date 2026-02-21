module;
#include <Visera-Core.hpp>
#include <memory>
export module Visera.Core.Types.Pointer.Unique;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    template<typename T, typename Deleter = std::default_delete<T>>
    class VISERA_CORE_API TUniquePtr
    {
        template<typename U, typename D> friend class TUniquePtr;

    public:
        /** @return Raw pointer to the owned object, or nullptr. */
        [[nodiscard]] T* Get() const noexcept { return Self.get(); }
        [[nodiscard]] std::unique_ptr<T, Deleter>& GetNative() noexcept { return Self; }
        [[nodiscard]] const std::unique_ptr<T, Deleter>& GetNative() const noexcept { return Self; }
        /** Releases ownership and returns the raw pointer. */
        [[nodiscard]] T* Release() noexcept { return Self.release(); }
        /** Replaces the managed object. */
        void Reset(T* I_Ptr = nullptr) noexcept { Self.reset(I_Ptr); }
        /** Swaps managed objects with I_Other. */
        void Swap(TUniquePtr& I_Other) noexcept { Self.swap(I_Other.Self); }

        [[nodiscard]] T& operator*()  const noexcept { return *Self; }
        [[nodiscard]] T* operator->() const noexcept { return Self.get(); }
        [[nodiscard]] explicit operator bool() const noexcept { return Self.get() != nullptr; }

        [[nodiscard]] friend Bool operator==(const TUniquePtr& I_Lhs, const TUniquePtr& I_Rhs) noexcept
        { return I_Lhs.Get() == I_Rhs.Get(); }

        [[nodiscard]] friend Bool operator!=(const TUniquePtr& I_Lhs, const TUniquePtr& I_Rhs) noexcept
        { return !(I_Lhs == I_Rhs); }

        [[nodiscard]] friend Bool operator==(const TUniquePtr& I_Lhs, std::nullptr_t) noexcept
        { return I_Lhs.Get() == nullptr; }

        [[nodiscard]] friend Bool operator!=(const TUniquePtr& I_Lhs, std::nullptr_t) noexcept
        { return !(I_Lhs == nullptr); }

        [[nodiscard]] friend Bool operator==(std::nullptr_t, const TUniquePtr& I_Rhs) noexcept
        { return I_Rhs.Get() == nullptr; }

        [[nodiscard]] friend Bool operator!=(std::nullptr_t, const TUniquePtr& I_Rhs) noexcept
        { return !(nullptr == I_Rhs); }

    private:
        std::unique_ptr<T, Deleter> Self;

    public:
        TUniquePtr() = default;
        TUniquePtr(std::nullptr_t) noexcept : Self(nullptr) {}
        explicit TUniquePtr(T* I_Ptr) noexcept : Self(I_Ptr) {}
        TUniquePtr(TUniquePtr&&) noexcept = default;
        template<typename U, typename OtherDeleter>
        requires std::convertible_to<U*, T*>
        TUniquePtr(TUniquePtr<U, OtherDeleter>&& I_Other) noexcept : Self(std::move(I_Other.GetNative())) {}
        template<typename OtherDeleter>
        TUniquePtr(std::unique_ptr<T, OtherDeleter>&& I_Other) noexcept : Self(std::move(I_Other)) {}

        TUniquePtr& operator=(TUniquePtr&&) noexcept = default;
        template<typename U, typename OtherDeleter>
        requires std::convertible_to<U*, T*>
        TUniquePtr& operator=(TUniquePtr<U, OtherDeleter>&& I_Other) noexcept { Self = std::move(I_Other.GetNative()); return *this; }
        TUniquePtr& operator=(std::nullptr_t) noexcept { Reset(); return *this; }
        template<typename OtherDeleter>
        TUniquePtr& operator=(std::unique_ptr<T, OtherDeleter>&& I_Other) noexcept { Self = std::move(I_Other); return *this; }

        ~TUniquePtr() = default;
    };

    /** Non-owning reference to an object held by TUniquePtr. Supports polymorphism: can be constructed from TUniquePtr<Derived> when returning TUniqueRef<Base>. */
    template<typename T>
    class VISERA_CORE_API TUniqueRef
    {
    public:
        TUniqueRef() noexcept : Ptr(nullptr) {}
        TUniqueRef(const TUniquePtr<T>& I_Ptr) noexcept : Ptr(I_Ptr.Get()) {}
        template<typename U, typename Deleter>
        requires std::convertible_to<U*, T*>
        TUniqueRef(const TUniquePtr<U, Deleter>& I_Ptr) noexcept : Ptr(I_Ptr.Get()) {}

        [[nodiscard]] T* Get() const noexcept { return Ptr; }
        [[nodiscard]] T& operator*()  const noexcept { return *Ptr; }
        [[nodiscard]] T* operator->() const noexcept { return Ptr; }
        [[nodiscard]] explicit operator bool() const noexcept { return Ptr != nullptr; }

        [[nodiscard]] friend Bool operator==(const TUniqueRef& I_Lhs, const TUniqueRef& I_Rhs) noexcept
        { return I_Lhs.Ptr == I_Rhs.Ptr; }
        [[nodiscard]] friend Bool operator!=(const TUniqueRef& I_Lhs, const TUniqueRef& I_Rhs) noexcept
        { return !(I_Lhs == I_Rhs); }
        [[nodiscard]] friend Bool operator==(const TUniqueRef& I_Lhs, std::nullptr_t) noexcept
        { return I_Lhs.Ptr == nullptr; }
        [[nodiscard]] friend Bool operator!=(const TUniqueRef& I_Lhs, std::nullptr_t) noexcept
        { return !(I_Lhs == nullptr); }
        [[nodiscard]] friend Bool operator==(std::nullptr_t, const TUniqueRef& I_Rhs) noexcept
        { return I_Rhs.Ptr == nullptr; }
        [[nodiscard]] friend Bool operator!=(std::nullptr_t, const TUniqueRef& I_Rhs) noexcept
        { return !(nullptr == I_Rhs); }

    private:
        T* Ptr;
    };

    template<typename T, typename... Args>
    [[nodiscard]] TUniquePtr<T> MakeUnique(Args&&... I_Args)
    { return TUniquePtr<T>(std::make_unique<T>(std::forward<Args>(I_Args)...)); }

    /** Creates TUniquePtr<Base> from Derived; use when storing a base pointer for polymorphic TUniqueRef<Base> return. */
    template<typename Base, typename Derived, typename... Args>
    requires std::convertible_to<Derived*, Base*>
    [[nodiscard]] TUniquePtr<Base> MakeUniqueAsBase(Args&&... I_Args)
    { return TUniquePtr<Base>(std::make_unique<Derived>(std::forward<Args>(I_Args)...)); }
}
