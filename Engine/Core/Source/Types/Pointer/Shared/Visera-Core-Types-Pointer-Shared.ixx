module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Pointer.Shared;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    template<typename U> class FWeakPtr;

    template<typename T>
    class VISERA_CORE_API TSharedPtr
    {
        template<typename U> friend class FWeakPtr;
        template<typename U> friend class TSharedPtr;

    public:
        /** @return Raw pointer to the managed object, or nullptr. */
        [[nodiscard]] T* Get() const noexcept { return Self.get(); }
        /** @return Number of TSharedPtr instances sharing ownership. */
        [[nodiscard]] long GetUseCount() const noexcept { return Self.use_count(); }
        /** Replaces the managed object (drops current ownership). */
        void Reset() noexcept { Self.reset(); }
        /** Swaps managed objects with I_Other. */
        void Swap(TSharedPtr& I_Other) noexcept { Self.swap(I_Other.Self); }

        [[nodiscard]] T& operator*() const noexcept { return *Self; }
        [[nodiscard]] T* operator->() const noexcept { return Self.get(); }
        [[nodiscard]] explicit operator bool() const noexcept { return Self != nullptr; }

    private:
        std::shared_ptr<T> Self;

    public:
        TSharedPtr() = default;
        TSharedPtr(std::nullptr_t) noexcept : Self(nullptr) {}
        explicit TSharedPtr(T* I_Ptr) : Self(I_Ptr) {}
        TSharedPtr(const TSharedPtr&) = default;
        TSharedPtr(TSharedPtr&&) noexcept = default;
        template<typename U>
        requires std::convertible_to<U*, T*>
        TSharedPtr(const TSharedPtr<U>& I_Other) noexcept : Self(I_Other.Self) {}
        template<typename U>
        requires std::convertible_to<U*, T*>
        TSharedPtr(TSharedPtr<U>&& I_Other) noexcept : Self(std::move(I_Other.Self)) {}
        TSharedPtr(std::shared_ptr<T> I_Other) noexcept : Self(std::move(I_Other)) {}

        TSharedPtr& operator=(const TSharedPtr&) = default;
        TSharedPtr& operator=(TSharedPtr&&) noexcept = default;
        template<typename U>
        requires std::convertible_to<U*, T*>
        TSharedPtr& operator=(const TSharedPtr<U>& I_Other) noexcept { Self = I_Other.Self; return *this; }
        template<typename U>
        requires std::convertible_to<U*, T*>
        TSharedPtr& operator=(TSharedPtr<U>&& I_Other) noexcept { Self = std::move(I_Other.Self); return *this; }
        TSharedPtr& operator=(std::nullptr_t) noexcept { Self.reset(); return *this; }
        TSharedPtr& operator=(std::shared_ptr<T> I_Other) noexcept { Self = std::move(I_Other); return *this; }

        ~TSharedPtr() = default;
    };

    template<typename T>
    using TSharedRef   = const TSharedPtr<T>&;

    template<typename T, typename... Args>
    [[nodiscard]] TSharedPtr<T> MakeShared(Args&&... I_Args)
    { return TSharedPtr<T>(std::make_shared<T>(std::forward<Args>(I_Args)...)); }
}
