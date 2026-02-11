module;
#include <Visera-Core.hpp>
#include <ankerl/unordered_dense.h>
export module Visera.Core.Containers.Set;
#define VISERA_MODULE_NAME "Core.Containers"

export namespace Visera
{
    template<typename T>
    class VISERA_CORE_API TSet
    {
    public:
        using ValueType     = T;
        using SetType       = ankerl::unordered_dense::set<T>;
        using Iterator      = typename SetType::iterator;
        using ConstIterator = typename SetType::const_iterator;
        using InsertResult  = TPair<Iterator, Bool>;

    private:
        SetType Set;

    public:
        // Constructors and Destructor
        TSet() = default;
        ~TSet() = default;

        // Copy constructor: only if T is copy constructible
        TSet(const TSet& I_Other)
            requires std::copy_constructible<T>
            = default;
        TSet(const TSet&)
            requires (!std::copy_constructible<T>)
            = delete;

        // Move constructor
        TSet(TSet&& I_Other) noexcept = default;

        // Copy assignment: only if T is copyable
        TSet& operator=(const TSet& I_Other)
            requires (std::copy_constructible<T> && std::is_copy_assignable_v<T>)
        {
            if (this != &I_Other)
            {
                Set = I_Other.Set;
            }
            return *this;
        }
        TSet& operator=(const TSet&)
            requires (!(std::copy_constructible<T> && std::is_copy_assignable_v<T>))
            = delete;

        // Move assignment
        TSet& operator=(TSet&& I_Other) noexcept = default;

        // Initializer list constructor
        TSet(std::initializer_list<T> I_Init)
            requires std::copy_constructible<T>
            : Set(I_Init)
        {
        }

        // Initializer list assignment
        TSet& operator=(std::initializer_list<T> I_Init)
            requires std::copy_constructible<T>
        {
            Set = I_Init;
            return *this;
        }

        // Capacity
        [[nodiscard]] Bool IsEmpty() const
        {
            return Set.empty();
        }

        [[nodiscard]] UInt64 GetSize() const
        {
            return static_cast<UInt64>(Set.size());
        }

        [[nodiscard]] UInt64 GetMaxSize() const
        {
            return static_cast<UInt64>(Set.max_size());
        }

        void Reserve(UInt64 I_Capacity)
        {
            Set.reserve(static_cast<typename SetType::size_type>(I_Capacity));
        }

        // Modifiers
        void Clear()
        {
            Set.clear();
        }

        InsertResult Insert(const T& I_Value)
            requires std::copy_constructible<T>
        {
            return Set.insert(I_Value);
        }

        InsertResult Insert(T&& I_Value)
        {
            return Set.insert(std::move(I_Value));
        }

        template<typename... Args>
        InsertResult Emplace(Args&&... I_Args)
        {
            return Set.emplace(std::forward<Args>(I_Args)...);
        }

        UInt64 Erase(const T& I_Value)
        {
            return static_cast<UInt64>(Set.erase(I_Value));
        }

        Iterator Erase(ConstIterator I_Iter)
        {
            return Set.erase(I_Iter);
        }

        template<typename Predicate>
        UInt64 EraseIf(Predicate&& I_Pred)
        {
            UInt64 ErasedCount = 0;
            for (auto Iter = Set.begin(); Iter != Set.end(); )
            {
                if (std::invoke(I_Pred, *Iter))
                { Iter = Set.erase(Iter); ++ErasedCount; }
                else
                { ++Iter; }
            }
            return ErasedCount;
        }

        void Swap(TSet& I_Other)
        {
            Set.swap(I_Other.Set);
        }

        // Lookup
        [[nodiscard]] Iterator Find(const T& I_Value)
        {
            return Set.find(I_Value);
        }

        [[nodiscard]] ConstIterator Find(const T& I_Value) const
        {
            return Set.find(I_Value);
        }

        [[nodiscard]] UInt64 Count(const T& I_Value) const
        {
            return static_cast<UInt64>(Set.count(I_Value));
        }

        [[nodiscard]] Bool Contains(const T& I_Value) const
        {
            return Set.contains(I_Value);
        }

        // Hash and bucket interface
        [[nodiscard]] UInt64 BucketCount() const
        {
            return static_cast<UInt64>(Set.bucket_count());
        }

        // Iterator access
        [[nodiscard]] Iterator begin() { return Set.begin(); }
        [[nodiscard]] ConstIterator begin() const { return Set.begin(); }
        [[nodiscard]] ConstIterator cbegin() const { return Set.cbegin(); }
        
        [[nodiscard]] Iterator end() { return Set.end(); }
        [[nodiscard]] ConstIterator end() const { return Set.end(); }
        [[nodiscard]] ConstIterator cend() const { return Set.cend(); }

        // Comparison operators
        [[nodiscard]] Bool operator==(const TSet& I_Other) const
        {
            return Set == I_Other.Set;
        }

        [[nodiscard]] Bool operator!=(const TSet& I_Other) const
        {
            return !(*this == I_Other);
        }
    };
}