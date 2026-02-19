module;
#include <Visera-Core.hpp>
#include <ankerl/unordered_dense.h>
export module Visera.Core.Containers.Map;
#define VISERA_MODULE_NAME "Core.Containers"
import std;

export namespace Visera
{
    template<typename Key, typename Value>
    class VISERA_CORE_API TMap
    {
    public:
        using KeyType       = Key;
        using ValueType     = Value;
        using MapType       = ankerl::unordered_dense::map<Key, Value>;
        using Iterator      = typename MapType::iterator;
        using ConstIterator = typename MapType::const_iterator;
        using Pair          = TPair<Key, Value>;
        using InsertResult  = TPair<Iterator, Bool>;

    private:
        MapType Map;

    public:
        // Constructors and Destructor
        TMap() = default;
        ~TMap() = default;

        // Explicitly disable copy semantics. This keeps TMap usable with move-only Value
        // types (e.g. TUniquePtr) and avoids instantiating copy paths in MapType.
        TMap(const TMap& I_Other) = delete;

        // Move constructor
        TMap(TMap&& I_Other) noexcept = default;

        TMap& operator=(const TMap& I_Other) = delete;

        // Move assignment
        TMap& operator=(TMap&& I_Other) noexcept = default;

        // Initializer list constructor
        TMap(std::initializer_list<Pair> I_Init)
            requires (std::copy_constructible<Key> && std::copy_constructible<Value>)
            : Map(I_Init)
        {
        }

        // Initializer list assignment
        TMap& operator=(std::initializer_list<Pair> I_Init)
            requires (std::copy_constructible<Key> && std::copy_constructible<Value>)
        {
            Map = I_Init;
            return *this;
        }

        // Element access
        [[nodiscard]] Value& operator[](const Key& I_Key)
        {
            return Map[I_Key];
        }

        [[nodiscard]] Value& operator[](Key&& I_Key)
        {
            return Map[std::move(I_Key)];
        }

        [[nodiscard]] Iterator Find(const Key& I_Key)
        {
            return Map.find(I_Key);
        }

        [[nodiscard]] ConstIterator Find(const Key& I_Key) const
        {
            return Map.find(I_Key);
        }

        [[nodiscard]] Value& At(const Key& I_Key)
        {
            return Map.at(I_Key);
        }

        [[nodiscard]] const Value& At(const Key& I_Key) const
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
            return static_cast<UInt64>(Map.size());
        }

        [[nodiscard]] UInt64 GetMaxSize() const
        {
            return static_cast<UInt64>(Map.max_size());
        }

        void Reserve(UInt64 I_Capacity)
        {
            Map.reserve(static_cast<typename MapType::size_type>(I_Capacity));
        }

        // Modifiers
        void Clear()
        {
            Map.clear();
        }

        InsertResult Insert(const Pair& I_Pair)
            requires (std::copy_constructible<Key> && std::copy_constructible<Value>)
        {
            return Map.insert(I_Pair);
        }

        InsertResult Insert(Pair&& I_Pair)
        {
            return Map.insert(std::move(I_Pair));
        }

        InsertResult Insert(const Key& I_Key, const Value& I_Value)
            requires (std::copy_constructible<Key> && std::copy_constructible<Value>)
        {
            return Map.insert({I_Key, I_Value});
        }

        InsertResult Insert(Key&& I_Key, Value&& I_Value)
        {
            return Map.insert({std::move(I_Key), std::move(I_Value)});
        }

        template<typename... Args>
        InsertResult Emplace(Args&&... I_Args)
        {
            return Map.emplace(std::forward<Args>(I_Args)...);
        }

        template<typename... Args>
        InsertResult TryEmplace(const Key& I_Key, Args&&... I_Args)
        {
            return Map.try_emplace(I_Key, std::forward<Args>(I_Args)...);
        }

        template<typename... Args>
        InsertResult TryEmplace(Key&& I_Key, Args&&... I_Args)
        {
            return Map.try_emplace(std::move(I_Key), std::forward<Args>(I_Args)...);
        }

        InsertResult InsertOrAssign(const Key& I_Key, const Value& I_Value)
            requires std::copy_constructible<Value>
        {
            return Map.insert_or_assign(I_Key, I_Value);
        }

        InsertResult InsertOrAssign(const Key& I_Key, Value&& I_Value)
        {
            return Map.insert_or_assign(I_Key, std::move(I_Value));
        }

        InsertResult InsertOrAssign(Key&& I_Key, Value&& I_Value)
        {
            return Map.insert_or_assign(std::move(I_Key), std::move(I_Value));
        }

        UInt64 Erase(const Key& I_Key)
        {
            return static_cast<UInt64>(Map.erase(I_Key));
        }

        Iterator Erase(Iterator I_Iter)
        {
            return Map.erase(I_Iter);
        }

        Iterator Erase(ConstIterator I_Iter)
        {
            return Map.erase(I_Iter);
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
            return static_cast<UInt64>(Map.count(I_Key));
        }

        [[nodiscard]] Bool Contains(const Key& I_Key) const
        {
            return Map.contains(I_Key);
        }

        // Hash and bucket interface
        [[nodiscard]] UInt64 BucketCount() const
        {
            return static_cast<UInt64>(Map.bucket_count());
        }

        // Iterator access
        [[nodiscard]] Iterator begin() { return Map.begin(); }
        [[nodiscard]] ConstIterator begin() const { return Map.begin(); }
        [[nodiscard]] ConstIterator cbegin() const { return Map.cbegin(); }
        
        [[nodiscard]] Iterator end() { return Map.end(); }
        [[nodiscard]] ConstIterator end() const { return Map.end(); }
        [[nodiscard]] ConstIterator cend() const { return Map.cend(); }

        // Comparison operators
        [[nodiscard]] Bool operator==(const TMap& I_Other) const
        {
            return Map == I_Other.Map;
        }

        [[nodiscard]] Bool operator!=(const TMap& I_Other) const
        {
            return !(*this == I_Other);
        }
    };
}