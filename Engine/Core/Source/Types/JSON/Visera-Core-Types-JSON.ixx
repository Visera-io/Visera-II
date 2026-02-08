module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.JSON;
#define VISERA_MODULE_NAME "Core.Types"
export import :Path;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.Text;
       import Visera.Core.Types.Array;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Optional;
       import Visera.Core.OS.FileSystem;
       import nlohmann.json;

//#define VISERA_SAFE_MODE;
#if defined(VISERA_SAFE_MODE)
#define CHECK(I_Statement) VISERA_ASSERT(I_Statement)
#else
#define CHECK(I_Statement) VISERA_NO_OPERATION
#endif

export namespace Visera
{
    class FJSON;
    template<> inline constexpr Bool HasIntrusiveUnsetOptionalState<FJSON> = True;

    /** JSON Query Language: compile-time path literal, e.g. json.GetString("config.server.port"_JQL). */
    [[nodiscard]] constexpr FJSONPath
    operator""_JQL(const char* I_Query, size_t I_Length) noexcept
    { return FJSONPath(I_Query, I_Length); }

    class VISERA_CORE_API FJSON
    {
        using Json = nlohmann::json;
    public:
        [[nodiscard]] static TOptional<FJSON>
        Parse(const FString& I_JSONString)
        {
            try   { return TOptional<FJSON>(Json::parse(I_JSONString)); }
            catch (...) { return NullOpt; }
        }

        [[nodiscard]] static TOptional<FJSON>
        Load(const FPath& I_JSONFile)
        {
            if (auto Stream = FFileSystem::OpenIStream(I_JSONFile); Stream)
            {
                try   { return TOptional<FJSON>(Json::parse(*Stream)); }
                catch (...) { return NullOpt; }
            }
            return NullOpt;
        }

        [[nodiscard]] constexpr Bool
        IsNull() const noexcept { return Root.is_null(); }
        [[nodiscard]] constexpr Bool
        IsDiscarded() const noexcept { return Root.is_discarded(); }
        [[nodiscard]] Bool
        Contains(FStringView I_Key) const noexcept { return Root.contains(I_Key.GetNative()); }
        void
        Clear() noexcept { Root = Json{}; }
        [[nodiscard]] FString
        Dump(Bool I_bPretty = True) const { return I_bPretty ? Root.dump(4) : Root.dump(); }
        void
        Set(FStringView I_Key, FStringView I_Value) { Root[I_Key.GetNative()] = FString(I_Value); }
        void
        Set(FStringView I_Key, Double I_Value) { Root[I_Key.GetNative()] = I_Value; }
        void
        Set(FStringView I_Key, Int64 I_Value) { Root[I_Key.GetNative()] = static_cast<std::int64_t>(I_Value); }
        void
        Set(FStringView I_Key, Bool I_Value) { Root[I_Key.GetNative()] = static_cast<bool>(I_Value); }
        void
        Set(FStringView I_Key, const FJSON& I_Value) { Root[I_Key.GetNative()] = I_Value.Root; }
        void
        Set(FStringView I_Key, const TArray<FJSON>& I_Array)
        {
            Json Array = Json::array();
            for (const auto& Item : I_Array)
            { Array.push_back(Item.Root); }
            Root[I_Key.GetNative()] = Array;
        }
        // ---- Get (safe) ----
        [[nodiscard]] FString
        GetString(FStringView I_Key, FStringView I_DefaultValue = "") const
        {
            const auto It = Root.find(I_Key.GetNative());
            if (It == Root.end() || !It->is_string()) { return FString(I_DefaultValue); }
            try { return GetStringFromJsonValue(*It); } catch (...) { return FString(I_DefaultValue); }
        }

        [[nodiscard]] TOptional<FString>
        TryGetString(FStringView I_Key) const noexcept
        {
            const auto It = Root.find(I_Key.GetNative());
            if (It == Root.end() || !It->is_string()) { return NullOpt; }
            try { return TOptional<FString>(GetStringFromJsonValue(*It)); } catch (...) { return NullOpt; }
        }

        [[nodiscard]] FString
        GetString(const FJSONPath& I_Path, FStringView I_DefaultValue = "") const
        {
            const auto Opt = TryGetString(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : FString(I_DefaultValue);
        }

        [[nodiscard]] TOptional<FString>
        TryGetString(const FJSONPath& I_Path) const noexcept
        {
            const Json* Ptr = FindPath(Root, I_Path);
            if (!Ptr || !Ptr->is_string()) { return NullOpt; }
            try { return TOptional<FString>(GetStringFromJsonValue(*Ptr)); } catch (...) { return NullOpt; }
        }

        [[nodiscard]] FPath
        GetPath(FStringView I_Key, const FPath& I_DefaultValue = FPath{""}) const
        {
            const auto Opt = TryGetPath(I_Key);
            return Opt.HasValue()? std::move(Opt).GetValue() : I_DefaultValue;
        }

        [[nodiscard]] TOptional<FPath>
        TryGetPath(FStringView I_Key) const noexcept
        {
            auto Result = TryGetString(I_Key);
            return Result.HasValue()? TOptional<FPath>(FPath{Result.GetValue()}) : NullOpt;
        }

        [[nodiscard]] FPath
        GetPath(const FJSONPath& I_Path, const FPath& I_DefaultValue = FPath{""}) const
        {
            const auto Opt = TryGetPath(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        [[nodiscard]] TOptional<FPath>
        TryGetPath(const FJSONPath& I_Path) const noexcept
        {
            const Json* Ptr = FindPath(Root, I_Path);
            if (!Ptr || !Ptr->is_string()) { return NullOpt; }
            try { return TOptional<FPath>(FPath{GetStringFromJsonValue(*Ptr)}); } catch (...) { return NullOpt; }
        }

        [[nodiscard]] Double
        GetNumber(FStringView I_Key, Double I_DefaultValue = 0.0) const noexcept
        {
            const auto Opt = TryGetNumber(I_Key);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        [[nodiscard]] TOptional<Double>
        TryGetNumber(FStringView I_Key) const noexcept
        {
            const auto It = Root.find(I_Key.GetNative());
            if (It == Root.end() || !It->is_number()) { return NullOpt; }
            try { return TOptional<Double>(It->get<Double>()); } catch (...) { return NullOpt; }
        }

        [[nodiscard]] Double
        GetNumber(const FJSONPath& I_Path, Double I_DefaultValue = 0.0) const noexcept
        {
            const auto Opt = TryGetNumber(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        [[nodiscard]] TOptional<Double>
        TryGetNumber(const FJSONPath& I_Path) const noexcept
        {
            const Json* p = FindPath(Root, I_Path);
            if (!p || !p->is_number()) { return NullOpt; }
            try { return TOptional<Double>(p->get<Double>()); } catch (...) { return NullOpt; }
        }

        [[nodiscard]] Bool
        GetBool(FStringView I_Key, Bool I_DefaultValue = False) const noexcept
        {
            const auto Opt = TryGetBool(I_Key);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        [[nodiscard]] TOptional<Bool>
        TryGetBool(FStringView I_Key) const noexcept
        {
            const auto It = Root.find(I_Key.GetNative());
            if (It == Root.end() || !It->is_boolean()) { return NullOpt; }
            try { return TOptional<Bool>(static_cast<Bool>(It->get<bool>())); } catch (...) { return NullOpt; }
        }

        [[nodiscard]] Bool
        GetBool(const FJSONPath& I_Path, Bool I_DefaultValue = False) const noexcept
        {
            const auto Opt = TryGetBool(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        [[nodiscard]] TOptional<Bool>
        TryGetBool(const FJSONPath& I_Path) const noexcept
        {
            const Json* p = FindPath(Root, I_Path);
            if (!p || !p->is_boolean()) { return NullOpt; }
            try { return TOptional<Bool>(static_cast<Bool>(p->get<bool>())); } catch (...) { return NullOpt; }
        }

        // ---- Get Object ----
        [[nodiscard]] FJSON
        GetObject(FStringView I_Key) const noexcept
        {
            const auto Opt = TryGetObject(I_Key);
            return Opt.HasValue() ? std::move(Opt).GetValue() : FJSON{};
        }

        [[nodiscard]] TOptional<FJSON>
        TryGetObject(FStringView I_Key) const noexcept
        {
            const auto It = Root.find(I_Key.GetNative());
            if (It == Root.end() || !It->is_object()) { return NullOpt; }
            try
            {
                FJSON Result;
                Result.Root = *It;
                return TOptional<FJSON>(std::move(Result));
            }
            catch (...) { return NullOpt; }
        }

        [[nodiscard]] FJSON
        GetObject(const FJSONPath& I_Path) const noexcept
        {
            const auto Opt = TryGetObject(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : FJSON{};
        }

        [[nodiscard]] TOptional<FJSON>
        TryGetObject(const FJSONPath& I_Path) const noexcept
        {
            const Json* Ptr = FindPath(Root, I_Path);
            if (!Ptr || !Ptr->is_object()) { return NullOpt; }
            try
            {
                FJSON Result;
                Result.Root = *Ptr;
                return TOptional<FJSON>(std::move(Result));
            }
            catch (...) { return NullOpt; }
        }

        // ---- Get Array (template) ----
        template<typename T>
        [[nodiscard]] TArray<T>
        GetArray(FStringView I_Key) const noexcept
        {
            const auto Opt = TryGetArray<T>(I_Key);
            return Opt.HasValue() ? std::move(Opt).GetValue() : TArray<T>{};
        }

        template<typename T>
        [[nodiscard]] TOptional<TArray<T>>
        TryGetArray(FStringView I_Key) const noexcept
        {
            const auto It = Root.find(I_Key.GetNative());
            if (It == Root.end() || !It->is_array()) { return NullOpt; }
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
                            Result.PushBack(GetStringFromJsonValue(Item));
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
                    else if constexpr (std::is_same_v<T, FJSON>)
                    {
                        if (Item.is_object() || Item.is_array())
                        {
                            FJSON JsonItem;
                            JsonItem.Root = Item;
                            Result.PushBack(std::move(JsonItem));
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

        template<typename T>
        [[nodiscard]] TArray<T>
        GetArray(const FJSONPath& I_Path) const noexcept
        {
            const auto Opt = TryGetArray<T>(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : TArray<T>{};
        }

        template<typename T>
        [[nodiscard]] TOptional<TArray<T>>
        TryGetArray(const FJSONPath& I_Path) const noexcept
        {
            const Json* Ptr = FindPath(Root, I_Path);
            if (!Ptr || !Ptr->is_array()) { return NullOpt; }
            try
            {
                const auto& Array = *Ptr;
                TArray<T> Result;
                Result.Reserve(Array.size());
                for (const auto& Item : Array)
                {
                    if constexpr (std::is_same_v<T, FString>)
                    {
                        if (Item.is_string())
                        {
                            Result.PushBack(GetStringFromJsonValue(Item));
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
                    else if constexpr (std::is_same_v<T, FJSON>)
                    {
                        if (Item.is_object() || Item.is_array())
                        {
                            FJSON JsonItem;
                            JsonItem.Root = Item;
                            Result.PushBack(std::move(JsonItem));
                        }
                    }
                    else
                    {
                        Result.PushBack(Item.get<T>());
                    }
                }
                return TOptional<TArray<T>>(std::move(Result));
            }
            catch (...) { return NullOpt; }
        }

        [[nodiscard]] Json&
        GetNative() noexcept { return Root; }
        [[nodiscard]] const Json&
        GetNative() const noexcept { return Root; }

        FJSON()                            = default;
        FJSON(const FJSON&)                = default;
        FJSON& operator=(const FJSON&)     = default;
        FJSON(FJSON&&)            noexcept = default;
        FJSON& operator=(FJSON&&) noexcept = default;
        FJSON(Json&& I_NativeJSON) noexcept : Root (std::move(I_NativeJSON)) /* !!! Do NOT use initializer_list {} !!!*/ {}
        FJSON& operator=(Json&& I_NativeJSON) noexcept { Root = std::move(I_NativeJSON); return *this; }

        FJSON(FIntrusiveUnsetOptionalState) noexcept : Root(Json::value_t::null) {}
        VISERA_CORE_API
        friend Bool operator==(const FJSON& I_Lhs, FIntrusiveUnsetOptionalState) noexcept;

    private:
        Json Root;

        static FString
        GetStringFromJsonValue(const Json& I_Value)
        { return FString(I_Value.get<std::string>()); }

        /** Resolves I_Path against I_Root. Returns pointer to the target value, or nullptr if not found or path invalid. */
        [[nodiscard]] static const Json*
        FindPath(const Json& I_Root, const FJSONPath& I_Path) noexcept
        {
            if (!I_Path.IsValid())
            {
                return nullptr;
            }
            const Json* Current = std::addressof(I_Root);
            for (const FJSONPath::FToken& Token : I_Path.GetTokens())
            {
                if (Token.Type == FJSONPath::FToken::EType::Key)
                {
                    if (!Current->is_object())
                    {
                        return nullptr;
                    }
                    const auto It = Current->find(Token.Key.GetNative());
                    if (It == Current->end())
                    {
                        return nullptr;
                    }
                    Current = std::addressof(*It);
                }
                else
                {
                    if (!Current->is_array())
                    {
                        return nullptr;
                    }
                    if (Token.Index >= Current->size())
                    {
                        return nullptr;
                    }
                    Current = std::addressof((*Current)[Token.Index]);
                }
            }
            return Current;
        }
    };
    static_assert(sizeof(TOptional<FJSON>) == sizeof(FJSON));

    Bool operator==(const FJSON& I_Lhs, FIntrusiveUnsetOptionalState) noexcept
    { return I_Lhs.IsNull(); }
}
VISERA_MAKE_FORMATTER(Visera::FJSON, {}, "{}", I_Formatee.Dump());