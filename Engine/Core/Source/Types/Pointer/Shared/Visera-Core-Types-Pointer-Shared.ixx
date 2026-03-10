module;
#include <Visera-Core.hpp>
#include <memory>
export module Visera.Core.Types.Pointer.Shared;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    template<typename T>
    class VISERA_CORE_API TSharedPtr
    {
    public:
        /** @return Raw pointer to the managed object, or nullptr. */
        [[nodiscard]] T* Get() const noexcept { return Self.get(); }
        [[nodiscard]] std::shared_ptr<T>& GetNative() noexcept { return Self; }
        [[nodiscard]] const std::shared_ptr<T>& GetNative() const noexcept { return Self; }
        /** @return Number of TSharedPtr instances sharing ownership. */
        [[nodiscard]] long GetUseCount() const noexcept { return Self.use_count(); }
        /** Strict weak ordering based on control block. Use for ordered containers (e.g. std::map). */
        [[nodiscard]] Bool OwnerBefore(const TSharedPtr& I_Other) const noexcept { return Self.owner_before(I_Other.Self); }
        /** Replaces the managed object (drops current ownership). */
        void Reset() noexcept { Self.reset(); }
        /** Swaps managed objects with I_Other. */
        void Swap(TSharedPtr& I_Other) noexcept { Self.swap(I_Other.Self); }

        [[nodiscard]] T& operator*() const noexcept { return *Self; }
        [[nodiscard]] T* operator->() const noexcept { return Self.get(); }
        [[nodiscard]] explicit operator bool() const noexcept { return Self != nullptr; }

        [[nodiscard]] friend Bool operator==(const TSharedPtr& I_Lhs, const TSharedPtr& I_Rhs) noexcept
        { return I_Lhs.Get() == I_Rhs.Get(); }

        [[nodiscard]] friend Bool operator!=(const TSharedPtr& I_Lhs, const TSharedPtr& I_Rhs) noexcept
        { return !(I_Lhs == I_Rhs); }

        [[nodiscard]] friend Bool operator==(const TSharedPtr& I_Lhs, std::nullptr_t) noexcept
        { return I_Lhs.Get() == nullptr; }

        [[nodiscard]] friend Bool operator!=(const TSharedPtr& I_Lhs, std::nullptr_t) noexcept
        { return !(I_Lhs == nullptr); }

        [[nodiscard]] friend Bool operator==(std::nullptr_t, const TSharedPtr& I_Rhs) noexcept
        { return I_Rhs.Get() == nullptr; }

        [[nodiscard]] friend Bool operator!=(std::nullptr_t, const TSharedPtr& I_Rhs) noexcept
        { return !(nullptr == I_Rhs); }

    private:
        std::shared_ptr<T> Self;

    public:
        TSharedPtr() = default;
        TSharedPtr(std::nullptr_t) noexcept : Self(nullptr) {}
        /** Creates a new control block for I_Ptr. For types inheriting FEnableSharedFromThis, prefer MakeShared or construction from an existing TSharedPtr to avoid multiple control blocks. */
        explicit TSharedPtr(T* I_Ptr) : Self(I_Ptr) {}
        TSharedPtr(const TSharedPtr&) = default;
        TSharedPtr(TSharedPtr&&) noexcept = default;
        template<typename U>
        requires std::convertible_to<U*, T*>
        TSharedPtr(const TSharedPtr<U>& I_Other) noexcept : Self(I_Other.GetNative()) {}
        template<typename U>
        requires std::convertible_to<U*, T*>
        TSharedPtr(TSharedPtr<U>&& I_Other) noexcept : Self(std::move(I_Other.GetNative())) {}
        TSharedPtr(std::shared_ptr<T> I_Other) noexcept : Self(std::move(I_Other)) {}

        TSharedPtr& operator=(const TSharedPtr&) = default;
        TSharedPtr& operator=(TSharedPtr&&) noexcept = default;
        template<typename U>
        requires std::convertible_to<U*, T*>
        TSharedPtr& operator=(const TSharedPtr<U>& I_Other) noexcept { Self = I_Other.GetNative(); return *this; }
        template<typename U>
        requires std::convertible_to<U*, T*>
        TSharedPtr& operator=(TSharedPtr<U>&& I_Other) noexcept { Self = std::move(I_Other.GetNative()); return *this; }
        TSharedPtr& operator=(std::nullptr_t) noexcept { Self.reset(); return *this; }
        TSharedPtr& operator=(std::shared_ptr<T> I_Other) noexcept { Self = std::move(I_Other); return *this; }

        ~TSharedPtr() = default;
    };

    /** Alias for const TSharedPtr<T>&. Use for function parameters that accept a shared pointer by reference without taking ownership. Distinct from TUniqueRef (non-owning raw view). */
    template<typename T>
    using TSharedRef   = const TSharedPtr<T>&;

    template<typename T, typename... Args>
    [[nodiscard]] TSharedPtr<T> MakeShared(Args&&... I_Args)
    { return TSharedPtr<T>(std::make_shared<T>(std::forward<Args>(I_Args)...)); }

    /** Base class for types that need to obtain a TSharedPtr from this. Use WeakFromThis(*this) when a weak reference is needed. Must be created via MakeShared or from an existing TSharedPtr; do not construct TSharedPtr from raw pointer for this type. */
    template<typename T>
    class VISERA_CORE_API FEnableSharedFromThis : public std::enable_shared_from_this<T>
    {
    public:
        [[nodiscard]] TSharedPtr<T> SharedFromThis() { return TSharedPtr<T>(this->shared_from_this()); }
        [[nodiscard]] TSharedPtr<T> SharedFromThis() const { return TSharedPtr<T>(this->shared_from_this()); }
    };
}
