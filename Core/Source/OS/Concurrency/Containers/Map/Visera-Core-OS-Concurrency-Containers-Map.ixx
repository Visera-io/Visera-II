module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Concurrency.Containers.Map;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.OS.Concurrency.Locks;

export namespace Visera
{
    namespace TS
    {
        template<typename Key, typename Value>
        class VISERA_CORE_API TMap
        {
        public:
            using KeyType = Key;
            using ValueType = Value;
            using SizeType = std::size_t;
            using MapType = ankerl::unordered_dense::map<Key, Value>;
            using Iterator = typename MapType::iterator;
            using ConstIterator = typename MapType::const_iterator;
            using Pair = std::pair<Key, Value>;
            using InsertResult = std::pair<Iterator, bool>;

        private:
            mutable FRWLock Lock; // RWLock works well for read-heavy tasks
            MapType Map;

        public:
            // Constructors and Destructor
            TMap() = default;
            ~TMap() = default;

            // Copy constructor with thread safety
            TMap(const TMap& Other)
            {
                Other.Lock.StartReading();
                Map = Other.Map;
                Other.Lock.StopReading();
            }

            // Move constructor with thread safety
            TMap(TMap&& Other) noexcept
            {
                Other.Lock.StartWriting();
                Map = std::move(Other.Map);
                Other.Lock.StopWriting();
            }

            // Copy assignment with thread safety
            TMap& operator=(const TMap& Other)
            {
                if (this != &Other)
                {
                    // Lock both maps in consistent order to avoid deadlock
                    if (this < &Other)
                    {
                        Lock.StartWriting();
                        Other.Lock.StartReading();
                    }
                    else
                    {
                        Other.Lock.StartReading();
                        Lock.StartWriting();
                    }
                    
                    Map = Other.Map;
                    
                    Lock.StopWriting();
                    Other.Lock.StopReading();
                }
                return *this;
            }

            // Move assignment with thread safety
            TMap& operator=(TMap&& Other) noexcept
            {
                if (this != &Other)
                {
                    // Lock both maps in consistent order to avoid deadlock
                    if (this < &Other)
                    {
                        Lock.StartWriting();
                        Other.Lock.StartWriting();
                    }
                    else
                    {
                        Other.Lock.StartWriting();
                        Lock.StartWriting();
                    }
                    
                    Map = std::move(Other.Map);
                    
                    Lock.StopWriting();
                    Other.Lock.StopWriting();
                }
                return *this;
            }

            // Element access
            Value& operator[](const Key& InKey)
            {
                Lock.StartWriting();
                auto& Result = Map[InKey];
                Lock.StopWriting();
                return Result;
            }

            Value& operator[](Key&& InKey)
            {
                Lock.StartWriting();
                auto& Result = Map[std::move(InKey)];
                Lock.StopWriting();
                return Result;
            }

            Value& At(const Key& InKey)
            {
                Lock.StartReading();
                try
                {
                    auto& Result = Map.at(InKey);
                    Lock.StopReading();
                    return Result;
                }
                catch (...)
                {
                    Lock.StopReading();
                    throw;
                }
            }

            const Value& At(const Key& InKey) const
            {
                Lock.StartReading();
                try
                {
                    const auto& Result = Map.at(InKey);
                    Lock.StopReading();
                    return Result;
                }
                catch (...)
                {
                    Lock.StopReading();
                    throw;
                }
            }

            // Capacity
            [[nodiscard]] Bool Empty() const
            {
                Lock.StartReading();
                Bool Result = Map.empty();
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] SizeType Size() const
            {
                Lock.StartReading();
                SizeType Result = Map.size();
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] SizeType MaxSize() const
            {
                Lock.StartReading();
                SizeType Result = Map.max_size();
                Lock.StopReading();
                return Result;
            }

            // Modifiers
            void Clear()
            {
                Lock.StartWriting();
                Map.clear();
                Lock.StopWriting();
            }

            InsertResult Insert(const Pair& InPair)
            {
                Lock.StartWriting();
                auto Result = Map.insert(InPair);
                Lock.StopWriting();
                return Result;
            }

            InsertResult Insert(Pair&& InPair)
            {
                Lock.StartWriting();
                auto Result = Map.insert(std::move(InPair));
                Lock.StopWriting();
                return Result;
            }

            template<typename... Args>
            InsertResult Emplace(Args&&... InArgs)
            {
                Lock.StartWriting();
                auto Result = Map.emplace(std::forward<Args>(InArgs)...);
                Lock.StopWriting();
                return Result;
            }

            template<typename... Args>
            InsertResult TryEmplace(const Key& InKey, Args&&... InArgs)
            {
                Lock.StartWriting();
                auto Result = Map.try_emplace(InKey, std::forward<Args>(InArgs)...);
                Lock.StopWriting();
                return Result;
            }

            template<typename... Args>
            InsertResult TryEmplace(Key&& InKey, Args&&... InArgs)
            {
                Lock.StartWriting();
                auto Result = Map.try_emplace(std::move(InKey), std::forward<Args>(InArgs)...);
                Lock.StopWriting();
                return Result;
            }

            SizeType Erase(const Key& InKey)
            {
                Lock.StartWriting();
                SizeType Result = Map.erase(InKey);
                Lock.StopWriting();
                return Result;
            }

            void Swap(TMap& Other)
            {
                // Lock both maps in consistent order to avoid deadlock
                if (this < &Other)
                {
                    Lock.StartWriting();
                    Other.Lock.StartWriting();
                }
                else
                {
                    Other.Lock.StartWriting();
                    Lock.StartWriting();
                }
                
                Map.swap(Other.Map);
                
                Lock.StopWriting();
                Other.Lock.StopWriting();
            }

            // Lookup
            [[nodiscard]] SizeType Count(const Key& InKey) const
            {
                Lock.StartReading();
                SizeType Result = Map.count(InKey);
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] Bool Contains(const Key& InKey) const
            {
                Lock.StartReading();
                Bool Result = Map.contains(InKey);
                Lock.StopReading();
                return Result;
            }

            // Thread-safe find operations with value copying
            [[nodiscard]] Bool Find(const Key& InKey, Value& OutValue) const
            {
                Lock.StartReading();
                auto It = Map.find(InKey);
                if (It != Map.end())
                {
                    OutValue = It->second;
                    Lock.StopReading();
                    return True;
                }
                Lock.StopReading();
                return False;
            }

            // Thread-safe find with optional return
            [[nodiscard]] std::optional<Value> FindOptional(const Key& InKey) const
            {
                Lock.StartReading();
                auto It = Map.find(InKey);
                if (It != Map.end())
                {
                    std::optional<Value> Result = It->second;
                    Lock.StopReading();
                    return Result;
                }
                Lock.StopReading();
                return std::nullopt;
            }

            // Thread-safe iteration with callback
            template<typename Func>
            void ForEach(Func&& InFunc) const
            {
                Lock.StartReading();
                try
                {
                    for (const auto& [Key, Value] : Map)
                    {
                        InFunc(Key, Value);
                    }
                    Lock.StopReading();
                }
                catch (...)
                {
                    Lock.StopReading();
                    throw;
                }
            }

            // Thread-safe modify operation with callback
            template<typename Func>
            void ModifyIf(const Key& InKey, Func&& InFunc)
            {
                Lock.StartWriting();
                auto It = Map.find(InKey);
                if (It != Map.end())
                {
                    try
                    {
                        InFunc(It->second);
                        Lock.StopWriting();
                    }
                    catch (...)
                    {
                        Lock.StopWriting();
                        throw;
                    }
                }
                else
                {
                    Lock.StopWriting();
                }
            }

            // Bulk operations for better performance
            template<typename InputIt>
            void InsertRange(InputIt First, InputIt Last)
            {
                Lock.StartWriting();
                Map.insert(First, Last);
                Lock.StopWriting();
            }

            void InsertOrAssign(const Key& InKey, const Value& InValue)
            {
                Lock.StartWriting();
                Map.insert_or_assign(InKey, InValue);
                Lock.StopWriting();
            }

            void InsertOrAssign(const Key& InKey, Value&& InValue)
            {
                Lock.StartWriting();
                Map.insert_or_assign(InKey, std::move(InValue));
                Lock.StopWriting();
            }

            void InsertOrAssign(Key&& InKey, const Value& InValue)
            {
                Lock.StartWriting();
                Map.insert_or_assign(std::move(InKey), InValue);
                Lock.StopWriting();
            }

            void InsertOrAssign(Key&& InKey, Value&& InValue)
            {
                Lock.StartWriting();
                Map.insert_or_assign(std::move(InKey), std::move(InValue));
                Lock.StopWriting();
            }

            // Reserve for performance optimization
            void Reserve(SizeType InSize)
            {
                Lock.StartWriting();
                Map.reserve(InSize);
                Lock.StopWriting();
            }

            // Thread-safe copy of all data
            [[nodiscard]] MapType GetCopy() const
            {
                Lock.StartReading();
                MapType Result = Map;
                Lock.StopReading();
                return Result;
            }

            // Hash and bucket interface
            [[nodiscard]] SizeType BucketCount() const
            {
                Lock.StartReading();
                SizeType Result = Map.bucket_count();
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] Float LoadFactor() const
            {
                Lock.StartReading();
                Float Result = Map.load_factor();
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] Float MaxLoadFactor() const
            {
                Lock.StartReading();
                Float Result = Map.max_load_factor();
                Lock.StopReading();
                return Result;
            }

            void MaxLoadFactor(Float InLoadFactor)
            {
                Lock.StartWriting();
                Map.max_load_factor(InLoadFactor);
                Lock.StopWriting();
            }

            // Advanced thread-safe operations
            
            // Conditional insert - only insert if key doesn't exist
            [[nodiscard]] Bool InsertIfAbsent(const Key& InKey, const Value& InValue)
            {
                Lock.StartWriting();
                auto [It, Inserted] = Map.try_emplace(InKey, InValue);
                Lock.StopWriting();
                return Inserted;
            }

            [[nodiscard]] Bool InsertIfAbsent(Key&& InKey, Value&& InValue)
            {
                Lock.StartWriting();
                auto [It, Inserted] = Map.try_emplace(std::move(InKey), std::move(InValue));
                Lock.StopWriting();
                return Inserted;
            }

            // Update if exists, with return indicating success
            [[nodiscard]] Bool UpdateIfExists(const Key& InKey, const Value& InValue)
            {
                Lock.StartWriting();
                auto It = Map.find(InKey);
                if (It != Map.end())
                {
                    It->second = InValue;
                    Lock.StopWriting();
                    return True;
                }
                Lock.StopWriting();
                return False;
            }

            [[nodiscard]] Bool UpdateIfExists(const Key& InKey, Value&& InValue)
            {
                Lock.StartWriting();
                auto It = Map.find(InKey);
                if (It != Map.end())
                {
                    It->second = std::move(InValue);
                    Lock.StopWriting();
                    return True;
                }
                Lock.StopWriting();
                return False;
            }

            // Get all keys (thread-safe)
            [[nodiscard]] TArray<Key> GetKeys() const
            {
                Lock.StartReading();
                TArray<Key> Keys;
                Keys.reserve(Map.size());
                for (const auto& [Key, Value] : Map)
                {
                    Keys.push_back(Key);
                }
                Lock.StopReading();
                return Keys;
            }

            // Get all values (thread-safe)
            [[nodiscard]] TArray<Value> GetValues() const
            {
                Lock.StartReading();
                TArray<Value> Values;
                Values.reserve(Map.size());
                for (const auto& [Key, Value] : Map)
                {
                    Values.push_back(Value);
                }
                Lock.StopReading();
                return Values;
            }

            // Thread-safe filter operation
            template<typename Predicate>
            [[nodiscard]] TMap FilterCopy(Predicate&& InPredicate) const
            {
                Lock.StartReading();
                TMap Result;
                for (const auto& [Key, Value] : Map)
                {
                    if (InPredicate(Key, Value))
                    {
                        Result.Map.emplace(Key, Value);
                    }
                }
                Lock.StopReading();
                return Result;
            }

            // Thread-safe transform operation
            template<typename TransformFunc>
            [[nodiscard]] auto Transform(TransformFunc&& InFunc) const
            {
                using ResultType = std::invoke_result_t<TransformFunc, const Key&, const Value&>;
                
                Lock.StartReading();
                TArray<ResultType> Results;
                Results.reserve(Map.size());
                for (const auto& [Key, Value] : Map)
                {
                    Results.push_back(InFunc(Key, Value));
                }
                Lock.StopReading();
                return Results;
            }

            // Erase if predicate matches
            template<typename Predicate>
            SizeType EraseIf(Predicate&& InPredicate)
            {
                Lock.StartWriting();
                SizeType Count = 0;
                for (auto It = Map.begin(); It != Map.end();)
                {
                    if (InPredicate(It->first, It->second))
                    {
                        It = Map.erase(It);
                        ++Count;
                    }
                    else
                    {
                        ++It;
                    }
                }
                Lock.StopWriting();
                return Count;
            }

            // Thread-safe merge from another map
            void Merge(const TMap& Other)
            {
                // Lock both maps in consistent order to avoid deadlock
                if (this < &Other)
                {
                    Lock.StartWriting();
                    Other.Lock.StartReading();
                }
                else
                {
                    Other.Lock.StartReading();
                    Lock.StartWriting();
                }
                
                for (const auto& [Key, Value] : Other.Map)
                {
                    Map.try_emplace(Key, Value);
                }
                
                Lock.StopWriting();
                Other.Lock.StopReading();
            }

            // Try to lock for reading with timeout (non-blocking)
            template<typename Func>
            [[nodiscard]] Bool TryExecuteRead(Func&& InFunc) const
            {
                if (Lock.TryToRead())
                {
                    try
                    {
                        InFunc(Map);
                        Lock.StopReading();
                        return True;
                    }
                    catch (...)
                    {
                        Lock.StopReading();
                        throw;
                    }
                }
                return False;
            }

            // Try to lock for writing with timeout (non-blocking)
            template<typename Func>
            [[nodiscard]] Bool TryExecuteWrite(Func&& InFunc)
            {
                if (Lock.TryToWrite())
                {
                    try
                    {
                        InFunc(Map);
                        Lock.StopWriting();
                        return True;
                    }
                    catch (...)
                    {
                        Lock.StopWriting();
                        throw;
                    }
                }
                return False;
            }

            // Comparison operators (thread-safe)
            Bool operator==(const TMap& Other) const
            {
                if (this == &Other) return True;
                
                // Lock both maps in consistent order to avoid deadlock
                if (this < &Other)
                {
                    Lock.StartReading();
                    Other.Lock.StartReading();
                }
                else
                {
                    Other.Lock.StartReading();
                    Lock.StartReading();
                }
                
                Bool Result = (Map == Other.Map);
                
                Lock.StopReading();
                Other.Lock.StopReading();
                
                return Result;
            }

            Bool operator!=(const TMap& Other) const
            {
                return !(*this == Other);
            }
        };
    }
}