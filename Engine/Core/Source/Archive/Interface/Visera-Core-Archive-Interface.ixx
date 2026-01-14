module;
#include <Visera-Core.hpp>
export module Visera.Core.Archive.Interface;
#define VISERA_MODULE_NAME "Core.Archive"

export namespace Visera
{
    /**
     * @brief Key wrapper for explicit key specification in archives.
     *
     * Used to explicitly specify a key for the next value in the serialization chain.
     * Alternative to passing strings directly to operator<<.
     *
     * @example
     * @code
     * FArchiveJSON Archive(EMode::Saving);
     * Archive << FArchiveKey("myKey") << 42;
     * // Equivalent to: Archive << "myKey" << 42;
     * @endcode
     */
    struct VISERA_CORE_API FArchiveKey
    {
        FStringView Key;
        explicit FArchiveKey(FStringView I_Key) : Key(I_Key) {}
    };

    /**
     * @brief Helper function to create FArchiveKey (optional, for clarity).
     *
     * @param I_Key The key string to wrap
     * @return FArchiveKey wrapper object
     *
     * @example
     * @code
     * FArchiveJSON Archive(EMode::Saving);
     * Archive << Key("myKey") << 123;
     * @endcode
     */
    [[nodiscard]] inline FArchiveKey Key(FStringView I_Key) { return FArchiveKey(I_Key); }

    /**
     * @brief Base abstract class for serialization archives (UE-style interface).
     * 
     * Provides a unified interface for serializing data using the operator<< pattern.
     * Supports both loading (reading) and saving (writing) operations.
     * 
     * @example Saving data (writing)
     * @code
     * FArchiveJSON Archive(EMode::Saving);
     * Archive << "name" << "John Doe"
     *        << "age" << 42
     *        << "active" << true
     *        << "score" << 95.5;
     * FString jsonOutput = Archive.Dump();
     * // Result: {"name":"John Doe","age":42,"active":true,"score":95.5}
     * @endcode
     * 
     * @example Loading data (reading)
     * @code
     * FString jsonData = R"({"name":"Jane","age":30,"active":false,"score":87.2})";
     * FArchiveJSON Archive(jsonData, EMode::Loading);
     * FString name;
     * Int32 age;
     * Bool active;
     * Double score;
     * Archive << "name" << name
     *        << "age" << age
     *        << "active" << active
     *        << "score" << score;
     * // name = "Jane", age = 30, active = false, score = 87.2
     * @endcode
     * 
     * @example Using explicit Key helper
     * @code
     * FArchiveJSON Archive(EMode::Saving);
     * Archive << Key("myKey") << 123;
     * @endcode
     * 
     * @example Nested objects with scope management
     * @code
     * FArchiveJSON Archive(EMode::Saving);
     * Archive.PushScope("player");
     * Archive << "name" << "Alice"
     *        << "level" << 10;
     * Archive.PushScope("stats");
     * Archive << "health" << 100
     *        << "mana" << 50;
     * Archive.PopScope();  // Pop "stats"
     * Archive.PopScope();  // Pop "player"
     * // Result: {"player":{"name":"Alice","level":10,"stats":{"health":100,"mana":50}}}
     * @endcode
     */
    class VISERA_CORE_API IArchive
    {
    public:
        enum class EMode { Loading, Saving };
        
        virtual ~IArchive() = default;
        
        [[nodiscard]] virtual Bool IsLoading() const = 0;
        [[nodiscard]] virtual Bool IsSaving() const = 0;
        
        // Serialization methods (pure virtual)
        virtual void Serialize(FStringView I_Key, Bool& IO_Value) = 0;
        virtual void Serialize(FStringView I_Key, Int32& IO_Value) = 0;
        virtual void Serialize(FStringView I_Key, UInt32& IO_Value) = 0;
        virtual void Serialize(FStringView I_Key, Double& IO_Value) = 0;
        virtual void Serialize(FStringView I_Key, FString& IO_Value) = 0;
        
        // Scope management for nested structures
        virtual Bool PushScope(FStringView I_Key) = 0;
        virtual void PopScope() = 0;

        // UE-style operator<< for serialization
        // Key specification - stores key for next value, or value if key already set
        IArchive& operator<<(FStringView I_String)
        {
            if (PendingKey.empty())
            {
                // Treat as key - store for next value
                PendingKey = FString(I_String);
            }
            else
            {
                // Treat as value - serialize using pending key
                FString Temp = FString(I_String);
                Serialize(PendingKey, Temp);
                PendingKey.clear();
            }
            return *this;
        }

        IArchive& operator<<(const FArchiveKey& I_Key)
        {
            PendingKey = FString(I_Key.Key);
            return *this;
        }

        // Value serialization (non-const for loading)
        IArchive& operator<<(Bool& IO_Value)
        {
            if (!PendingKey.empty())
            {
                Serialize(PendingKey, IO_Value);
                PendingKey.clear();
            }
            return *this;
        }

        IArchive& operator<<(Int32& IO_Value)
        {
            if (!PendingKey.empty())
            {
                Serialize(PendingKey, IO_Value);
                PendingKey.clear();
            }
            return *this;
        }

        IArchive& operator<<(UInt32& IO_Value)
        {
            if (!PendingKey.empty())
            {
                Serialize(PendingKey, IO_Value);
                PendingKey.clear();
            }
            return *this;
        }

        IArchive& operator<<(Double& IO_Value)
        {
            if (!PendingKey.empty())
            {
                Serialize(PendingKey, IO_Value);
                PendingKey.clear();
            }
            return *this;
        }

        IArchive& operator<<(FString& IO_Value)
        {
            if (!PendingKey.empty())
            {
                Serialize(PendingKey, IO_Value);
                PendingKey.clear();
            }
            return *this;
        }

        // Const value serialization (for saving only) - uses temporary non-const reference
        IArchive& operator<<(Bool I_Value)
        {
            if (!PendingKey.empty())
            {
                Bool Temp = I_Value;
                Serialize(PendingKey, Temp);
                PendingKey.clear();
            }
            return *this;
        }

        IArchive& operator<<(Int32 I_Value)
        {
            if (!PendingKey.empty())
            {
                Int32 Temp = I_Value;
                Serialize(PendingKey, Temp);
                PendingKey.clear();
            }
            return *this;
        }

        IArchive& operator<<(UInt32 I_Value)
        {
            if (!PendingKey.empty())
            {
                UInt32 Temp = I_Value;
                Serialize(PendingKey, Temp);
                PendingKey.clear();
            }
            return *this;
        }

        IArchive& operator<<(Double I_Value)
        {
            if (!PendingKey.empty())
            {
                Double Temp = I_Value;
                Serialize(PendingKey, Temp);
                PendingKey.clear();
            }
            return *this;
        }

        IArchive& operator<<(const FString& I_Value)
        {
            if (!PendingKey.empty())
            {
                FString Temp = I_Value;
                Serialize(PendingKey, Temp);
                PendingKey.clear();
            }
            return *this;
        }

    protected:
        FString PendingKey;  // Stores the key for the next value serialization
    };
}
