module;
#include <Visera-Core.hpp>
#include <ankerl/unordered_dense.h>
export module Visera.Core.Types.Map;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.OS.Thread;

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
        using I_sertResult  = TPair<Iterator, Bool>;

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

        I_sertResult I_sert(const Pair& I_Pair)
        {
            return Map.insert(I_Pair);
        }

        I_sertResult I_sert(Pair&& I_Pair)
        {
            return Map.insert(std::move(I_Pair));
        }

        template<typename... Args>
        I_sertResult Emplace(Args&&... I_Args)
        {
            return Map.emplace(std::forward<Args>(I_Args)...);
        }

        UInt64 Erase(const Key& I_Key)
        {
            return Map.erase(I_Key);
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
            using I_sertResult  = TPair<Iterator, Bool>;

        private:
            mutable FRWLock Lock; // RWLock works well for read-heavy tasks
            MapType Map;

        public:
            // Constructors and Destructor
            TMap() = default;
            ~TMap() = default;

            // Copy constructor with thread safety
            TMap(const TMap& I_Other)
            {
                I_Other.Lock.StartReading();
                Map = I_Other.Map;
                I_Other.Lock.StopReading();
            }

            // Move constructor with thread safety
            TMap(TMap&& I_Other) noexcept
            {
                I_Other.Lock.StartWriting();
                Map = std::move(I_Other.Map);
                I_Other.Lock.StopWriting();
            }

            // Copy assignment with thread safety
            TMap& operator=(const TMap& I_Other)
            {
                if (this != &I_Other)
                {
                    // Lock both maps in consistent order to avoid deadlock
                    if (this < &I_Other)
                    {
                        Lock.StartWriting();
                        I_Other.Lock.StartReading();
                    }
                    else
                    {
                        I_Other.Lock.StartReading();
                        Lock.StartWriting();
                    }

                    Map = I_Other.Map;

                    Lock.StopWriting();
                    I_Other.Lock.StopReading();
                }
                return *this;
            }

            // Move assignment with thread safety
            TMap& operator=(TMap&& I_Other) noexcept
            {
                if (this != &I_Other)
                {
                    // Lock both maps in consistent order to avoid deadlock
                    if (this < &I_Other)
                    {
                        Lock.StartWriting();
                        I_Other.Lock.StartWriting();
                    }
                    else
                    {
                        I_Other.Lock.StartWriting();
                        Lock.StartWriting();
                    }

                    Map = std::move(I_Other.Map);

                    Lock.StopWriting();
                    I_Other.Lock.StopWriting();
                }
                return *this;
            }

            // Initializer list constructor with thread safety
            TMap(std::initializer_list<Pair> I_Init)
                : Map(I_Init)
            {
            }

            // Initializer list assignment with thread safety
            TMap& operator=(std::initializer_list<Pair> I_Init)
            {
                Lock.StartWriting();
                Map = I_Init;
                Lock.StopWriting();
                return *this;
            }

            // Element access
            Value& operator[](const Key& I_Key)
            {
                Lock.StartWriting();
                auto& Result = Map[I_Key];
                Lock.StopWriting();
                return Result;
            }

            Value& operator[](Key&& I_Key)
            {
                Lock.StartWriting();
                auto& Result = Map[std::move(I_Key)];
                Lock.StopWriting();
                return Result;
            }

            Value& At(const Key& I_Key)
            {
                Lock.StartReading();
                try
                {
                    auto& Result = Map.at(I_Key);
                    Lock.StopReading();
                    return Result;
                }
                catch (...)
                {
                    Lock.StopReading();
                    throw;
                }
            }

            const Value& At(const Key& I_Key) const
            {
                Lock.StartReading();
                try
                {
                    const auto& Result = Map.at(I_Key);
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

            I_sertResult I_sert(const Pair& I_Pair)
            {
                Lock.StartWriting();
                auto Result = Map.insert(I_Pair);
                Lock.StopWriting();
                return Result;
            }

            I_sertResult I_sert(Pair&& I_Pair)
            {
                Lock.StartWriting();
                auto Result = Map.insert(std::move(I_Pair));
                Lock.StopWriting();
                return Result;
            }

            template<typename... Args>
            I_sertResult Emplace(Args&&... I_Args)
            {
                Lock.StartWriting();
                auto Result = Map.emplace(std::forward<Args>(I_Args)...);
                Lock.StopWriting();
                return Result;
            }

            UInt64 Erase(const Key& I_Key)
            {
                Lock.StartWriting();
                UInt64 Result = Map.erase(I_Key);
                Lock.StopWriting();
                return Result;
            }

            void Swap(TMap& I_Other)
            {
                // Lock both maps in consistent order to avoid deadlock
                if (this < &I_Other)
                {
                    Lock.StartWriting();
                    I_Other.Lock.StartWriting();
                }
                else
                {
                    I_Other.Lock.StartWriting();
                    Lock.StartWriting();
                }
                
                Map.swap(I_Other.Map);
                
                Lock.StopWriting();
                I_Other.Lock.StopWriting();
            }

            // Lookup
            [[nodiscard]] UInt64 Count(const Key& I_Key) const
            {
                Lock.StartReading();
                UInt64 Result = Map.count(I_Key);
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] Bool Contains(const Key& I_Key) const
            {
                Lock.StartReading();
                Bool Result = Map.contains(I_Key);
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
            Bool operator==(const TMap& I_Other) const
            {
                if (this == &I_Other) return True;
                
                // Lock both maps in consistent order to avoid deadlock
                if (this < &I_Other)
                {
                    Lock.StartReading();
                    I_Other.Lock.StartReading();
                }
                else
                {
                    I_Other.Lock.StartReading();
                    Lock.StartReading();
                }
                
                Bool Result = (Map == I_Other.Map);
                
                Lock.StopReading();
                I_Other.Lock.StopReading();
                
                return Result;
            }

            Bool operator!=(const TMap& I_Other) const
            {
                return !(*this == I_Other);
            }
        };
    }
}