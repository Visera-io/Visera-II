module;
#include <Visera-Core.hpp>
#include <ankerl/unordered_dense.h>
export module Visera.Core.Types.Set;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.OS.Thread;

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

    namespace TS
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
            mutable FRWLock Lock; // RWLock works well for read-heavy tasks
            SetType Set;

        public:
            // Constructors and Destructor
            TSet() = default;
            ~TSet() = default;

            // Copy constructor with thread safety
            TSet(const TSet& Other)
            {
                Other.Lock.StartReading();
                Set = Other.Set;
                Other.Lock.StopReading();
            }

            // Move constructor with thread safety
            TSet(TSet&& Other) noexcept
            {
                Other.Lock.StartWriting();
                Set = std::move(Other.Set);
                Other.Lock.StopWriting();
            }

            // Copy assignment with thread safety
            TSet& operator=(const TSet& Other)
            {
                if (this != &Other)
                {
                    // Lock both sets in consistent order to avoid deadlock
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

                    Set = Other.Set;

                    Lock.StopWriting();
                    Other.Lock.StopReading();
                }
                return *this;
            }

            // Move assignment with thread safety
            TSet& operator=(TSet&& Other) noexcept
            {
                if (this != &Other)
                {
                    // Lock both sets in consistent order to avoid deadlock
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

                    Set = std::move(Other.Set);

                    Lock.StopWriting();
                    Other.Lock.StopWriting();
                }
                return *this;
            }

            // Initializer list constructor with thread safety
            TSet(std::initializer_list<T> I_Init)
                : Set(I_Init)
            {
            }

            // Initializer list assignment with thread safety
            TSet& operator=(std::initializer_list<T> I_Init)
            {
                Lock.StartWriting();
                Set = I_Init;
                Lock.StopWriting();
                return *this;
            }

            // Capacity
            [[nodiscard]] Bool IsEmpty() const
            {
                Lock.StartReading();
                Bool Result = Set.empty();
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] UInt64 GetSize() const
            {
                Lock.StartReading();
                UInt64 Result = Set.size();
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] UInt64 GetMaxSize() const
            {
                Lock.StartReading();
                UInt64 Result = Set.max_size();
                Lock.StopReading();
                return Result;
            }

            // Modifiers
            void Clear()
            {
                Lock.StartWriting();
                Set.clear();
                Lock.StopWriting();
            }

            InsertResult Insert(const T& InValue)
            {
                Lock.StartWriting();
                auto Result = Set.insert(InValue);
                Lock.StopWriting();
                return Result;
            }

            InsertResult Insert(T&& InValue)
            {
                Lock.StartWriting();
                auto Result = Set.insert(std::move(InValue));
                Lock.StopWriting();
                return Result;
            }

            template<typename... Args>
            InsertResult Emplace(Args&&... InArgs)
            {
                Lock.StartWriting();
                auto Result = Set.emplace(std::forward<Args>(InArgs)...);
                Lock.StopWriting();
                return Result;
            }

            UInt64 Erase(const T& InValue)
            {
                Lock.StartWriting();
                UInt64 Result = Set.erase(InValue);
                Lock.StopWriting();
                return Result;
            }

            void Swap(TSet& Other)
            {
                // Lock both sets in consistent order to avoid deadlock
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
                
                Set.swap(Other.Set);
                
                Lock.StopWriting();
                Other.Lock.StopWriting();
            }

            // Lookup
            [[nodiscard]] UInt64 Count(const T& InValue) const
            {
                Lock.StartReading();
                UInt64 Result = Set.count(InValue);
                Lock.StopReading();
                return Result;
            }

            [[nodiscard]] Bool Contains(const T& InValue) const
            {
                Lock.StartReading();
                Bool Result = Set.contains(InValue);
                Lock.StopReading();
                return Result;
            }

            // Hash and bucket interface
            [[nodiscard]] UInt64 BucketCount() const
            {
                Lock.StartReading();
                UInt64 Result = Set.bucket_count();
                Lock.StopReading();
                return Result;
            }

            // Comparison operators (thread-safe)
            Bool operator==(const TSet& Other) const
            {
                if (this == &Other) return True;
                
                // Lock both sets in consistent order to avoid deadlock
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
                
                Bool Result = (Set == Other.Set);
                
                Lock.StopReading();
                Other.Lock.StopReading();
                
                return Result;
            }

            Bool operator!=(const TSet& Other) const
            {
                return !(*this == Other);
            }
        };
    }
}