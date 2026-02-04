module;
#include <Visera-Core.hpp>
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
        [[nodiscard]] explicit operator bool() const noexcept { return Self != nullptr; }

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

    template<typename T>
    using TUniqueRef   = const TUniquePtr<T>&;

    template<typename T, typename... Args>
    [[nodiscard]] TUniquePtr<T> MakeUnique(Args&&... I_Args)
    { return TUniquePtr<T>(std::make_unique<T>(std::forward<Args>(I_Args)...)); }
}
