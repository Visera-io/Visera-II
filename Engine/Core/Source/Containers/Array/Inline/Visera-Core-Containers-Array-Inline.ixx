/** @file Visera-Core-Containers-Array-Inline.ixx
 *  @brief Module Visera.Core.Containers.Array.Inline — inline storage array TInlineArray<T, N>.
 *  Fixed capacity N, no heap allocation; storage is embedded. Use when max count is known at compile time. */
module;
#include <Visera-Core.hpp>
#include <memory>
#include <iterator>
#include <algorithm>
#include <functional>
#include <initializer_list>
export module Visera.Core.Containers.Array.Inline;
#define VISERA_MODULE_NAME "Core.Containers"
import Visera.Core.OS.Memory;
import Visera.Core.Types.Optional;

export namespace Visera
{
    /** Contiguous array with inline storage for up to N elements. No heap allocation; capacity is fixed at compile time.
     *  Logical size is 0..N; GetSize() is the current count. Use for per-frame buffers, small fixed slot sets, etc. */
    template<typename T, size_t N>
    class VISERA_CORE_API TInlineArray
    {
    public:
        using ValueType             = T;
        using SizeType              = size_t;
        using Iterator              = T*;
        using ConstIterator         = const T*;
        using ReverseIterator       = std::reverse_iterator<Iterator>;
        using ConstReverseIterator  = std::reverse_iterator<ConstIterator>;
        using Reference             = T&;
        using ConstReference        = const T&;

    private:
        SizeType Size{ 0 };
        alignas(T) std::byte Storage[(N != 0) ? (N * sizeof(T)) : 1];

        [[nodiscard]] T* DataUnsafe() noexcept
        {
            if constexpr (N == 0) return nullptr;
            return std::launder(reinterpret_cast<T*>(std::addressof(Storage[0])));
        }

        [[nodiscard]] const T* DataUnsafe() const noexcept
        {
            if constexpr (N == 0) return nullptr;
            return std::launder(reinterpret_cast<const T*>(std::addressof(Storage[0])));
        }

    public:
        // Constructors and destructor
        TInlineArray() = default;

        TInlineArray(std::initializer_list<T> I_Init)
            requires std::copy_constructible<T>
        {
            VISERA_ASSERT(I_Init.size() <= N);
            Size = 0;
            for (const T& E : I_Init) { PushBack(E); }
        }

        ~TInlineArray()
        {
            if constexpr (N > 0 && !std::is_trivially_destructible_v<T>)
            {
                std::destroy_n(DataUnsafe(), Size);
            }
        }

        TInlineArray(const TInlineArray& I_Other)
            requires std::copy_constructible<T>
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                Size = I_Other.Size;
                if (Size > 0)
                    Memory::Memcpy(DataUnsafe(), I_Other.DataUnsafe(), static_cast<UInt64>(Size) * sizeof(T));
            }
            else
            {
                std::uninitialized_copy_n(I_Other.begin(), I_Other.Size, DataUnsafe());
                Size = I_Other.Size;
            }
        }

        TInlineArray(const TInlineArray&)
            requires (!std::copy_constructible<T>)
            = delete;

        TInlineArray& operator=(const TInlineArray& I_Other)
            requires (std::copy_constructible<T> && std::is_copy_assignable_v<T>)
        {
            if (this == &I_Other) return *this;
            T* D = DataUnsafe();
            const T* S = I_Other.DataUnsafe();
            const SizeType NewSize = I_Other.Size;
            const SizeType OldSize = Size;
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                if (NewSize > 0)
                    Memory::Memcpy(D, S, static_cast<UInt64>(NewSize) * sizeof(T));
                Size = NewSize;
            }
            else
            {
                if (NewSize <= OldSize)
                {
                    for (SizeType i = 0; i < NewSize; ++i)
                        D[i] = S[i];
                    if constexpr (!std::is_trivially_destructible_v<T>)
                        std::destroy_n(D + NewSize, OldSize - NewSize);
                }
                else
                {
                    VISERA_ASSERT(NewSize <= N);
                    for (SizeType i = 0; i < OldSize; ++i)
                        D[i] = S[i];
                    std::uninitialized_copy_n(S + OldSize, NewSize - OldSize, D + OldSize);
                }
                Size = NewSize;
            }
            return *this;
        }

        TInlineArray& operator=(const TInlineArray&)
            requires (!(std::copy_constructible<T> && std::is_copy_assignable_v<T>))
            = delete;

        TInlineArray(TInlineArray&& I_Other) noexcept(std::is_nothrow_move_constructible_v<T>)
        {
            T* D = DataUnsafe();
            T* S = I_Other.DataUnsafe();
            const SizeType Count = I_Other.Size;
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                Size = Count;
                if (Count > 0)
                    Memory::Memcpy(D, S, static_cast<UInt64>(Count) * sizeof(T));
                I_Other.Size = 0;
            }
            else
            {
                std::uninitialized_move_n(S, Count, D);
                Size = Count;
                if constexpr (N > 0 && !std::is_trivially_destructible_v<T>)
                    std::destroy_n(S, Count);
                I_Other.Size = 0;
            }
        }

        TInlineArray& operator=(TInlineArray&& I_Other) noexcept(
            std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>)
        {
            if (this == &I_Other) return *this;
            T* D = DataUnsafe();
            T* S = I_Other.DataUnsafe();
            const SizeType NewSize = I_Other.Size;
            const SizeType OldSize = Size;
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                if (NewSize > 0)
                    Memory::Memcpy(D, S, static_cast<UInt64>(NewSize) * sizeof(T));
                Size = NewSize;
                I_Other.Size = 0;
                return *this;
            }
            if (NewSize <= OldSize)
            {
                for (SizeType i = 0; i < NewSize; ++i)
                    D[i] = std::move(S[i]);
                if constexpr (!std::is_trivially_destructible_v<T>)
                    std::destroy_n(D + NewSize, OldSize - NewSize);
            }
            else
            {
                for (SizeType i = 0; i < OldSize; ++i)
                    D[i] = std::move(S[i]);
                std::uninitialized_move_n(S + OldSize, NewSize - OldSize, D + OldSize);
            }
            Size = NewSize;
            I_Other.Clear();
            return *this;
        }

        // Capacity (fixed at N)
        [[nodiscard]] static constexpr SizeType GetCapacity() noexcept { return N; }

        [[nodiscard]] static constexpr SizeType GetMaxSize() noexcept { return N; }

        [[nodiscard]] UInt64 GetSize() const noexcept
        {
            return static_cast<UInt64>(Size);
        }

        [[nodiscard]] Bool IsEmpty() const noexcept
        {
            return Size == 0;
        }

        [[nodiscard]] Bool IsFull() const noexcept
        {
            return Size == N;
        }

        // Element access (operator[] asserts I_Index < Size; At() returns TOptional for bounds-safe access)
        [[nodiscard]] T& operator[](SizeType I_Index)
        {
            VISERA_ASSERT(I_Index < Size);
            return DataUnsafe()[I_Index];
        }

        [[nodiscard]] const T& operator[](SizeType I_Index) const
        {
            VISERA_ASSERT(I_Index < Size);
            return DataUnsafe()[I_Index];
        }

        /** @return TOptional holding a reference to the element if I_Index is in bounds; NullOpt otherwise. */
        [[nodiscard]] TOptional<std::reference_wrapper<T>> At(SizeType I_Index) noexcept
        {
            if (I_Index >= Size) return NullOpt;
            return TOptional<std::reference_wrapper<T>>(std::ref(DataUnsafe()[I_Index]));
        }

        /** @return TOptional holding a const reference to the element if I_Index is in bounds; NullOpt otherwise. */
        [[nodiscard]] TOptional<std::reference_wrapper<const T>> At(SizeType I_Index) const noexcept
        {
            if (I_Index >= Size) return NullOpt;
            return TOptional<std::reference_wrapper<const T>>(std::cref(DataUnsafe()[I_Index]));
        }

        [[nodiscard]] T& Front()
        {
            VISERA_ASSERT(Size > 0);
            return DataUnsafe()[0];
        }

        [[nodiscard]] const T& Front() const
        {
            VISERA_ASSERT(Size > 0);
            return DataUnsafe()[0];
        }

        [[nodiscard]] T& Back()
        {
            VISERA_ASSERT(Size > 0);
            return DataUnsafe()[Size - 1];
        }

        [[nodiscard]] const T& Back() const
        {
            VISERA_ASSERT(Size > 0);
            return DataUnsafe()[Size - 1];
        }

        [[nodiscard]] T* Data() noexcept { return DataUnsafe(); }

        [[nodiscard]] const T* Data() const noexcept { return DataUnsafe(); }

        // Iterators (raw pointers; range-compatible)
        [[nodiscard]] Iterator begin() noexcept { return DataUnsafe(); }
        [[nodiscard]] ConstIterator begin() const noexcept { return DataUnsafe(); }
        [[nodiscard]] ConstIterator cbegin() const noexcept { return DataUnsafe(); }
        [[nodiscard]] Iterator end() noexcept { return DataUnsafe() + Size; }
        [[nodiscard]] ConstIterator end() const noexcept { return DataUnsafe() + Size; }
        [[nodiscard]] ConstIterator cend() const noexcept { return DataUnsafe() + Size; }
        [[nodiscard]] ReverseIterator rbegin() noexcept { return ReverseIterator(end()); }
        [[nodiscard]] ConstReverseIterator rbegin() const noexcept { return ConstReverseIterator(end()); }
        [[nodiscard]] ConstReverseIterator crbegin() const noexcept { return ConstReverseIterator(end()); }
        [[nodiscard]] ReverseIterator rend() noexcept { return ReverseIterator(begin()); }
        [[nodiscard]] ConstReverseIterator rend() const noexcept { return ConstReverseIterator(begin()); }
        [[nodiscard]] ConstReverseIterator crend() const noexcept { return ConstReverseIterator(begin()); }

        // Modifiers (PushBack/EmplaceBack assert Size < N)
        void Clear() noexcept
        {
            if constexpr (N > 0 && !std::is_trivially_destructible_v<T>)
            {
                std::destroy_n(DataUnsafe(), Size);
            }
            Size = 0;
        }

        void PushBack(const T& I_Value)
            requires std::copy_constructible<T>
        {
            VISERA_ASSERT(Size < N);
            new (DataUnsafe() + Size) T(I_Value);
            ++Size;
        }

        void PushBack(T&& I_Value)
        {
            VISERA_ASSERT(Size < N);
            new (DataUnsafe() + Size) T(std::move(I_Value));
            ++Size;
        }

        template<typename... Args>
        T& EmplaceBack(Args&&... I_Args)
        {
            VISERA_ASSERT(Size < N);
            T* P = new (DataUnsafe() + Size) T(std::forward<Args>(I_Args)...);
            ++Size;
            return *P;
        }

        void PopBack()
        {
            VISERA_ASSERT(Size > 0);
            --Size;
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                std::destroy_at(DataUnsafe() + Size);
            }
        }

        void Swap(TInlineArray& I_Other) noexcept
        {
            if (this == &I_Other) return;
            T* D = DataUnsafe();
            T* S = I_Other.DataUnsafe();
            const SizeType MinS = (Size < I_Other.Size) ? Size : I_Other.Size;
            for (SizeType i = 0; i < MinS; ++i)
            {
                using std::swap;
                swap(D[i], S[i]);
            }
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                if (Size < I_Other.Size)
                    Memory::Memcpy(D + MinS, S + MinS, static_cast<UInt64>(I_Other.Size - MinS) * sizeof(T));
                else if (Size > I_Other.Size)
                    Memory::Memcpy(S + MinS, D + MinS, static_cast<UInt64>(Size - MinS) * sizeof(T));
            }
            else
            {
                if (Size < I_Other.Size)
                {
                    std::uninitialized_move_n(S + MinS, I_Other.Size - MinS, D + MinS);
                    if constexpr (!std::is_trivially_destructible_v<T>)
                        std::destroy_n(S + MinS, I_Other.Size - MinS);
                }
                else if (Size > I_Other.Size)
                {
                    std::uninitialized_move_n(D + MinS, Size - MinS, S + MinS);
                    if constexpr (!std::is_trivially_destructible_v<T>)
                        std::destroy_n(D + MinS, Size - MinS);
                }
            }
            std::swap(Size, I_Other.Size);
        }

        [[nodiscard]] Bool operator==(const TInlineArray& I_Other) const
            requires std::equality_comparable<T>
        {
            if (Size != I_Other.Size) return false;
            return std::equal(begin(), end(), I_Other.begin());
        }

        Iterator RemoveAtSwap(Iterator I_Iterator) requires (std::movable<T> && std::assignable_from<T&, T>)
        {
            if (I_Iterator == end()) return end();
            SizeType Idx = static_cast<SizeType>(I_Iterator - DataUnsafe());
            RemoveAtSwap(Idx);
            return (Idx < Size) ? (DataUnsafe() + Idx) : end();
        }

        void RemoveAtSwap(SizeType I_Index) requires (std::movable<T> && std::assignable_from<T&, T>)
        {
            VISERA_ASSERT(I_Index < Size);
            const SizeType LastIndex = Size - 1;
            if (I_Index != LastIndex)
            {
                DataUnsafe()[I_Index] = std::move(DataUnsafe()[LastIndex]);
            }
            PopBack();
        }
    };
}
