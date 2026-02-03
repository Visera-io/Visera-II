module;
#include <Visera-Core.hpp>
#include <nlohmann/json.hpp>
export module Visera.Core.Types.JSON;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Array;
import Visera.Core.Types.Text;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;

//#define VISERA_SAFE_MODE;
#if defined(VISERA_SAFE_MODE)
#define CHECK(I_Statement) VISERA_ASSERT(I_Statement)
#else
#define CHECK(I_Statement) VISERA_NO_OPERATION
#endif

export namespace Visera
{
    class VISERA_CORE_API FJSON
    {
    public:
        using Json = nlohmann::json;

        [[nodiscard]] Bool
        Contains(FStringView I_Key) const noexcept { return Data.contains(I_Key.Data()); }
        void
        Clear() noexcept { Data = Json{}; LastError.Clear(); }
        [[nodiscard]] FString
        Dump(Bool I_bPretty = True) const { return I_bPretty ? Data.dump(4) : Data.dump(); }
        [[nodiscard]] const FString&
        GetLastError() const noexcept { return LastError; }
        void
        Set(FStringView I_Key, FStringView I_Value) { Data[I_Key.Data()] = FString(I_Value); }
        void
        Set(FStringView I_Key, Double I_Value) { Data[I_Key.Data()] = I_Value; }
        void
        Set(FStringView I_Key, Bool I_Value) { Data[I_Key.Data()] = static_cast<bool>(I_Value); }
        // ---- Get (safe) ----
        [[nodiscard]] FString
        GetString(FStringView I_Key, FStringView I_DefaultValue = "") const
        {
            const auto It = Data.find(I_Key.Data());
            if (It == Data.end() || !It->is_string()) { return FString(I_DefaultValue); }
            try { return It->get<std::string>(); } catch (...) { return FString(I_DefaultValue); }
        }

        [[nodiscard]] TOptional<FString>
        TryGetString(FStringView I_Key) const noexcept
        {
            const auto It = Data.find(I_Key.Data());
            if (It == Data.end() || !It->is_string()) { return NullOpt; }
            try { return TOptional<FString>(It->get<std::string>()); } catch (...) { return NullOpt; }
        }

        [[nodiscard]] Double
        GetNumber(FStringView I_Key, Double I_DefaultValue = 0.0) const noexcept
        {
            const auto Opt = TryGetNumber(I_Key.Data());
            return Opt.HasValue() ? Opt.GetValue() : I_DefaultValue;
        }

        [[nodiscard]] TOptional<Double>
        TryGetNumber(FStringView I_Key) const noexcept
        {
            const auto It = Data.find(I_Key.Data());
            if (It == Data.end() || !It->is_number()) { return NullOpt; }
            try { return TOptional<Double>(It->get<Double>()); } catch (...) { return NullOpt; }
        }

        [[nodiscard]] Bool
        GetBool(FStringView I_Key, Bool I_DefaultValue = False) const noexcept
        {
            const auto Opt = TryGetBool(I_Key.Data());
            return Opt.HasValue() ? Opt.GetValue() : I_DefaultValue;
        }

        [[nodiscard]] TOptional<Bool>
        TryGetBool(FStringView I_Key) const noexcept
        {
            const auto It = Data.find(I_Key.Data());
            if (It == Data.end() || !It->is_boolean()) { return NullOpt; }
            try { return TOptional<Bool>(static_cast<Bool>(It->get<bool>())); } catch (...) { return NullOpt; }
        }

        // ---- Get Object ----
        [[nodiscard]] FJSON
        GetObject(FStringView I_Key) const noexcept
        {
            const auto Opt = TryGetObject(I_Key.Data());
            return Opt.HasValue() ? Opt.GetValue() : FJSON{};
        }

        [[nodiscard]] TOptional<FJSON>
        TryGetObject(FStringView I_Key) const noexcept
        {
            const auto It = Data.find(I_Key.Data());
            if (It == Data.end() || !It->is_object()) { return NullOpt; }
            try
            {
                FJSON Result;
                Result.Data = *It;
                return TOptional<FJSON>(std::move(Result));
            }
            catch (...) { return NullOpt; }
        }

        // ---- Get Array (template) ----
        template<typename T>
        [[nodiscard]] TArray<T>
        GetArray(FStringView I_Key) const noexcept
        {
            const auto Opt = TryGetArray<T>(I_Key.Data());
            return Opt.HasValue() ? Opt.GetValue() : TArray<T>{};
        }

        template<typename T>
        [[nodiscard]] TOptional<TArray<T>>
        TryGetArray(FStringView I_Key) const noexcept
        {
            const auto It = Data.find(I_Key.Data());
            if (It == Data.end() || !It->is_array()) { return NullOpt; }
            try
            {
                const auto& Array = *It;
                TArray<T> Result;
                Result.Reserve(Array.size());
                for (const auto& Item : Array)
                {
                    if constexpr (std::is_same_v<T, FString>)
                    {
                        if (Item.is_string())
                        {
                            Result.PushBack(Item.get<std::string>());
                        }
                    }
                    else if constexpr (std::is_same_v<T, Double> || std::is_same_v<T, Float>)
                    {
                        if (Item.is_number())
                        {
                            Result.PushBack(Item.get<T>());
                        }
                    }
                    else if constexpr (std::is_same_v<T, Int32> || std::is_same_v<T, Int64> || 
                                       std::is_same_v<T, UInt32> || std::is_same_v<T, UInt64>)
                    {
                        if (Item.is_number_integer())
                        {
                            Result.PushBack(Item.get<T>());
                        }
                    }
                    else if constexpr (std::is_same_v<T, Bool>)
                    {
                        if (Item.is_boolean())
                        {
                            Result.PushBack(static_cast<Bool>(Item.get<bool>()));
                        }
                    }
                    else
                    {
                        // Generic case: try to get the value directly
                        Result.PushBack(Item.get<T>());
                    }
                }
                return TOptional<TArray<T>>(std::move(Result));
            }
            catch (...) { return NullOpt; }
        }

        [[nodiscard]] Json&
        GetNative() noexcept { return Data; }
        [[nodiscard]] const Json&
        GetNative() const noexcept { return Data; }

        FJSON() = default;
        explicit FJSON(FStringView         I_JSONString) { Bool bSuccessed = Parse(I_JSONString); VISERA_ASSERT(bSuccessed); }
        explicit FJSON(const TArray<FByte> I_JSONData)   { Bool bSuccessed = Parse(reinterpret_cast<const char*>(I_JSONData.Data())); VISERA_ASSERT(bSuccessed); }

    private:
        Json    Data{};
        FString LastError{};

    private:
        [[nodiscard]] Bool
        Parse(FStringView I_JSONData) noexcept
        {
            LastError.Clear();
            try
            {
                // nlohmann::json::parse expects iterators to a contiguous char range; data may not be null-terminated.
                const FString Text{ I_JSONData };
                Data = Json::parse(Text.begin(), Text.end());
                return True;
            }
            catch (const std::exception& I_E)
            {
                LastError = I_E.what();
                Data = Json{};
                return False;
            }
        }

    };
}
