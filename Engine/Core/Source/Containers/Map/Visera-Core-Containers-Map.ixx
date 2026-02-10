module;
#include <Visera-Core.hpp>
#include <ankerl/unordered_dense.h>
export module Visera.Core.Containers.Map;
#define VISERA_MODULE_NAME "Core.Containers"

export namespace Visera
{
    template<typename Key, typename Value>
    class VISERA_CORE_API TMap
    {
    public:
        using KeyType       = Key;
        using ValueType     = Value;
        using MapType       = ankerl::unordered_dense::map<Key, Value>;
        using Iterator      = MapType::iterator;
        using ConstIterator = MapType::const_iterator;
        using Pair          = TPair<Key, Value>;
        using InsertResult  = TPair<Iterator, Bool>;

    private:
        MapType Map;

    public:
        // Constructors and Destructor
        TMap() = default;
        ~TMap() = default;

        // Copy constructor
        TMap(const TMap& I_Other) = default;

        // Move constructor
        TMap(TMap&& I_Other) noexcept = default;

        // Copy assignment
        TMap& operator=(const TMap& I_Other) = default;

        // Move assignment
        TMap& operator=(TMap&& I_Other) noexcept = default;

        // Initializer list constructor
        TMap(std::initializer_list<Pair> I_Init)
            : Map(I_Init)
        {
        }

        // Initializer list assignment
        TMap& operator=(std::initializer_list<Pair> I_Init)
        {
            Map = I_Init;
            return *this;
        }

        // Element access
        Value& operator[](const Key& I_Key)
        {
            return Map[I_Key];
        }

        Value& operator[](Key&& I_Key)
        {
            return Map[std::move(I_Key)];
        }

        auto Find(const Key& I_Key)
        {
            return Map.find(I_Key);
        }

        auto Find(const Key& I_Key) const
        {
            return Map.find(I_Key);
        }

        Value& At(const Key& I_Key)
        {
            return Map.at(I_Key);
        }

        const Value& At(const Key& I_Key) const
        {
            return Map.at(I_Key);
        }

        // Capacity
        [[nodiscard]] Bool IsEmpty() const
        {
            return Map.empty();
        }

        [[nodiscard]] UInt64 GetSize() const
        {
            return Map.size();
        }

        [[nodiscard]] UInt64 GetMaxSize() const
        {
            return Map.max_size();
        }

        // Modifiers
        void Clear()
        {
            Map.clear();
        }

        InsertResult Insert(const Pair& I_Pair)
        {
            return Map.insert(I_Pair);
        }

        InsertResult Insert(Pair&& I_Pair)
        {
            return Map.insert(std::move(I_Pair));
        }

        template<typename... Args>
        InsertResult Emplace(Args&&... I_Args)
        {
            return Map.emplace(std::forward<Args>(I_Args)...);
        }

        UInt64 Erase(const Key& I_Key)
        {
            return Map.erase(I_Key);
        }

        template<typename Predicate>
        UInt64 EraseIf(Predicate&& I_Pred)
        {
            UInt64 ErasedCount = 0;
            for (auto Iter = Map.begin(); Iter != Map.end(); )
            {
                if (std::invoke(I_Pred, Iter->first, Iter->second))
                { Iter = Map.erase(Iter); ++ErasedCount; }
                else
                { ++Iter; }
            }
            return ErasedCount;
        }

        void Swap(TMap& I_Other)
        {
            Map.swap(I_Other.Map);
        }

        // Lookup
        [[nodiscard]] UInt64 Count(const Key& I_Key) const
        {
            return Map.count(I_Key);
        }

        [[nodiscard]] Bool Contains(const Key& I_Key) const
        {
            return Map.contains(I_Key);
        }

        // Hash and bucket interface
        [[nodiscard]] UInt64 BucketCount() const
        {
            return Map.bucket_count();
        }

        // Iterator access
        Iterator begin() { return Map.begin(); }
        ConstIterator begin() const { return Map.begin(); }
        ConstIterator cbegin() const { return Map.cbegin(); }
        
        Iterator end() { return Map.end(); }
        ConstIterator end() const { return Map.end(); }
        ConstIterator cend() const { return Map.cend(); }

        // Comparison operators
        Bool operator==(const TMap& I_Other) const
        {
            return Map == I_Other.Map;
        }

        Bool operator!=(const TMap& I_Other) const
        {
            return !(*this == I_Other);
        }
    };
}