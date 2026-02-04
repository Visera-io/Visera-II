module;
#include <Visera-Core.hpp>
#include <ankerl/unordered_dense.h>
export module Visera.Core.Types.Set;
#define VISERA_MODULE_NAME "Core.Types"

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
        using InsertResult  = TPair<Iterator, bool>;

    private:
        SetType Set;

    public:
        // Constructors and Destructor
        TSet() = default;
        ~TSet() = default;

        // Copy constructor
        TSet(const TSet& Other) = default;

        // Move constructor
        TSet(TSet&& Other) noexcept = default;

        // Copy assignment
        TSet& operator=(const TSet& Other) = default;

        // Move assignment
        TSet& operator=(TSet&& Other) noexcept = default;

        // Initializer list constructor
        TSet(std::initializer_list<T> I_Init)
            : Set(I_Init)
        {
        }

        // Initializer list assignment
        TSet& operator=(std::initializer_list<T> I_Init)
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
            return Set.size();
        }

        [[nodiscard]] UInt64 GetMaxSize() const
        {
            return Set.max_size();
        }

        // Modifiers
        void Clear()
        {
            Set.clear();
        }

        InsertResult Insert(const T& InValue)
        {
            return Set.insert(InValue);
        }

        InsertResult Insert(T&& InValue)
        {
            return Set.insert(std::move(InValue));
        }

        template<typename... Args>
        InsertResult Emplace(Args&&... InArgs)
        {
            return Set.emplace(std::forward<Args>(InArgs)...);
        }

        UInt64 Erase(const T& InValue)
        {
            return Set.erase(InValue);
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

        void Swap(TSet& Other)
        {
            Set.swap(Other.Set);
        }

        // Lookup
        [[nodiscard]] UInt64 Count(const T& InValue) const
        {
            return Set.count(InValue);
        }

        [[nodiscard]] Bool Contains(const T& InValue) const
        {
            return Set.contains(InValue);
        }

        // Hash and bucket interface
        [[nodiscard]] UInt64 BucketCount() const
        {
            return Set.bucket_count();
        }

        // Iterator access
        Iterator begin() { return Set.begin(); }
        ConstIterator begin() const { return Set.begin(); }
        ConstIterator cbegin() const { return Set.cbegin(); }
        
        Iterator end() { return Set.end(); }
        ConstIterator end() const { return Set.end(); }
        ConstIterator cend() const { return Set.cend(); }

        // Comparison operators
        Bool operator==(const TSet& Other) const
        {
            return Set == Other.Set;
        }

        Bool operator!=(const TSet& Other) const
        {
            return !(*this == Other);
        }
    };
}