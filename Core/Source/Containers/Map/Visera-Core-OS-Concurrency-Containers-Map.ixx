module;
#include <Visera-Core.hpp>
export module Visera.Core.Containers.Map;
#define VISERA_MODULE_NAME "Core.Containers"
import Visera.Core.OS.Thread;

export namespace Visera
{
    namespace TS
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
            using InsertResult  = TPair<Iterator, bool>;

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
            [[nodiscard]] Bool IsEmpty() const
            {
                Lock.StartReading();
                Bool Result = Map.empty();
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] UInt64 GetSize() const
            {
                Lock.StartReading();
                UInt64 Result = Map.size();
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] UInt64 GetMaxSize() const
            {
                Lock.StartReading();
                UInt64 Result = Map.max_size();
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

            UInt64 Erase(const Key& InKey)
            {
                Lock.StartWriting();
                UInt64 Result = Map.erase(InKey);
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
            [[nodiscard]] UInt64 Count(const Key& InKey) const
            {
                Lock.StartReading();
                UInt64 Result = Map.count(InKey);
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

            // Hash and bucket interface
            [[nodiscard]] UInt64 BucketCount() const
            {
                Lock.StartReading();
                UInt64 Result = Map.bucket_count();
                Lock.StopReading();
                return Result;
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