module;
#include <Visera-Core.hpp>
export module Visera.Core.Containers.Cache.LRU;
#define VISERA_MODULE_NAME "Core.Containers"
import Visera.Core.Containers.List;
import Visera.Core.Containers.Map;
import Visera.Core.Types.Optional;
import Visera.Core.Types.Pointer.Unique;
import Visera.Core.Log;

export namespace Visera
{
    namespace Policy
    {
        /** Evict by entry count. Capacity = max number of entries. */
        struct CountBased
        {
            template <typename Value>
            [[nodiscard]] static UInt64 GetWeight(const Value&) noexcept
            { return 1; }
        };

        /** Evict by total byte size. Capacity = max bytes. SizeFunc: invocable as (const Value&) -> UInt64. */
        template <typename SizeFunc>
        struct ByteWeighted
        {
            SizeFunc GetSize;

            explicit ByteWeighted(SizeFunc I_GetSize)
                : GetSize(std::move(I_GetSize))
            { }

            template <typename Value>
            [[nodiscard]] UInt64 GetWeight(const Value& I_Value) const
            { return GetSize(I_Value); }
        };

        template <typename P>
        struct IsCachePolicy : std::false_type { };

        template <>
        struct IsCachePolicy<CountBased> : std::true_type { };

        template <typename F>
        struct IsCachePolicy<ByteWeighted<F>> : std::true_type { };
    }

    /**
     * LRU (Least Recently Used) cache with O(1) get/put/eviction.
     * Policy: CountBased (capacity = count) or ByteWeighted<SizeFunc> (capacity = bytes).
     * Uses TIntrusiveDoubleLinkedList: Head = MRU, Tail = LRU.
     */
    template <typename Key, typename Value, typename Policy = Policy::CountBased>
    class VISERA_CORE_API TLRUCache
    {
        struct FEntryTag { };

        struct FEntry : TIntrusiveDoubleLinkedListNode<FEntry, FEntryTag>
        {
            Key    KeyStorage;
            Value  ValueStorage;
            UInt64 Weight{ 0 };

            template <typename K, typename V>
            FEntry(K&& I_Key, V&& I_Value)
                : KeyStorage(std::forward<K>(I_Key))
                , ValueStorage(std::forward<V>(I_Value))
            { }
        };

        using LinkedListType = TIntrusiveDoubleLinkedList<FEntry, FEntryTag>;
        using MapType        = TMap<Key, FEntry*>;

        MapType        Map;
        LinkedListType LRUList;
        UInt64         Capacity{ 0 };
        UInt64         CurrentWeight{ 0 };
        Policy         PolicyInstance;

        void EvictLRU()
        {
            if (LRUList.IsEmpty()) { return; }

            FEntry* Tail = LRUList.GetTail();
            VISERA_ASSERT(Tail);
            CurrentWeight -= Tail->Weight;
            Map.Erase(Tail->KeyStorage);
            LRUList.Remove(Tail);
            delete Tail;
        }

    public:
        using KeyType      = Key;
        using ValueType    = Value;
        using PolicyType   = Policy;

        explicit TLRUCache(UInt64 I_Capacity)
            requires (Visera::Policy::IsCachePolicy<Policy>::value)
            : Capacity(I_Capacity)
        { }

        TLRUCache(UInt64 I_Capacity, Policy I_Policy)
            : Capacity(I_Capacity)
            , PolicyInstance(std::move(I_Policy))
        { }

        ~TLRUCache() { Clear(); }

        TLRUCache(const TLRUCache&)            = delete;
        TLRUCache& operator=(const TLRUCache&) = delete;

        TLRUCache(TLRUCache&& I_Other) noexcept
            : Map(std::move(I_Other.Map))
            , LRUList(std::move(I_Other.LRUList))
            , Capacity(I_Other.Capacity)
            , CurrentWeight(I_Other.CurrentWeight)
            , PolicyInstance(std::move(I_Other.PolicyInstance))
        {
            I_Other.Map.Clear();
            I_Other.LRUList.Reset();
            I_Other.Capacity = I_Other.CurrentWeight = 0;
        }

        TLRUCache& operator=(TLRUCache&& I_Other) noexcept
        {
            if (this != &I_Other)
            {
                Clear();
                Map           = std::move(I_Other.Map);
                LRUList       = std::move(I_Other.LRUList);
                Capacity      = I_Other.Capacity;
                CurrentWeight = I_Other.CurrentWeight;
                PolicyInstance = std::move(I_Other.PolicyInstance);
                I_Other.Map.Clear();
                I_Other.LRUList.Reset();
                I_Other.Capacity = I_Other.CurrentWeight = 0;
            }
            return *this;
        }

        [[nodiscard]] UInt64 GetCapacity() const { return Capacity; }
        [[nodiscard]] UInt64 GetCurrentWeight() const { return CurrentWeight; }
        [[nodiscard]] UInt64 GetSize() const { return Map.GetSize(); }
        [[nodiscard]] Bool   IsEmpty() const { return Map.IsEmpty(); }

        [[nodiscard]] Value* GetAndTouch(const Key& I_Key)
        {
            auto It = Map.Find(I_Key);
            if (It == Map.end()) { return nullptr; }

            FEntry* Entry = It->second;
            LRUList.MoveToHead(Entry);
            return std::addressof(Entry->ValueStorage);
        }

        [[nodiscard]] Value* Peek(const Key& I_Key)
        {
            auto It = Map.Find(I_Key);
            return It != Map.end() ? std::addressof(It->second->ValueStorage) : nullptr;
        }

        [[nodiscard]] const Value* Peek(const Key& I_Key) const
        {
            auto It = Map.Find(I_Key);
            return It != Map.end() ? std::addressof(It->second->ValueStorage) : nullptr;
        }

        void Put(const Key& I_Key, const Value& I_Value) { PutImpl(I_Key, I_Value); }
        void Put(const Key& I_Key, Value&& I_Value)      { PutImpl(I_Key, std::move(I_Value)); }
        void Put(Key&& I_Key, const Value& I_Value)      { PutImpl(std::move(I_Key), I_Value); }
        void Put(Key&& I_Key, Value&& I_Value)           { PutImpl(std::move(I_Key), std::move(I_Value)); }

        Bool Remove(const Key& I_Key)
        {
            auto It = Map.Find(I_Key);
            if (It == Map.end()) { return False; }

            FEntry* Entry = It->second;
            CurrentWeight -= Entry->Weight;
            Map.Erase(I_Key);
            LRUList.Remove(Entry);
            delete Entry;
            return True;
        }

        [[nodiscard]] Bool Contains(const Key& I_Key) const
        {
            return Map.Contains(I_Key);
        }

        void SetCapacity(UInt64 I_NewCapacity)
        {
            Capacity = I_NewCapacity;
            while (Capacity > 0 && CurrentWeight > Capacity) { EvictLRU(); }
        }

        void Clear()
        {
            for (auto& Pair : Map) { delete Pair.second; }
            Map.Clear();
            LRUList.Reset();
            CurrentWeight = 0;
        }

    private:
        template <typename K, typename V>
        void PutImpl(K&& I_Key, V&& I_Value)
        {
            auto It = Map.Find(I_Key);
            if (It != Map.end())
            {
                FEntry* Entry = It->second;
                CurrentWeight -= Entry->Weight;
                Entry->ValueStorage = std::forward<V>(I_Value);
                Entry->Weight       = PolicyInstance.GetWeight(Entry->ValueStorage);
                CurrentWeight += Entry->Weight;
                LRUList.MoveToHead(Entry);
                while (Capacity > 0 && CurrentWeight > Capacity) { EvictLRU(); }
                return;
            }

            const UInt64 EntryWeight = PolicyInstance.GetWeight(I_Value);
            while (Capacity > 0 && CurrentWeight + EntryWeight > Capacity && !LRUList.IsEmpty())
            { EvictLRU(); }

            if (Capacity == 0 || (Capacity > 0 && CurrentWeight + EntryWeight > Capacity))
            {
                if (Capacity > 0 && EntryWeight > Capacity)
                { LOG_WARN("Entry weight {} exceeds cache capacity {}; item will not be cached.", EntryWeight, Capacity); }
                return;
            }

            auto EntryPtr = MakeUnique<FEntry>(std::forward<K>(I_Key), std::forward<V>(I_Value));
            EntryPtr->Weight = EntryWeight;
            try
            {
                Map.Insert(EntryPtr->KeyStorage, EntryPtr.Get());
                LRUList.AddHead(EntryPtr.Get());
                CurrentWeight += EntryWeight;
                (void)EntryPtr.Release();
            }
            catch (...)
            {
                Map.Erase(EntryPtr->KeyStorage);
                throw;
            }
        }
    };

    /** Alias: count-based LRU (capacity = max entry count). */
    template <typename Key, typename Value>
    using TLRUCountCache = TLRUCache<Key, Value, Policy::CountBased>;
}
