module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.JSON;
#define VISERA_MODULE_NAME "Core.Types"
export import :Path;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.Text;
       import Visera.Core.Containers.Array;
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
        FJSON&
        Set(FStringView I_Key, FStringView I_Value) { Root[I_Key.GetNative()] = FString(I_Value); return *this; }
        FJSON&
        Set(FStringView I_Key, Double I_Value) { Root[I_Key.GetNative()] = I_Value; return *this; }
        FJSON&
        Set(FStringView I_Key, Int64 I_Value) { Root[I_Key.GetNative()] = static_cast<std::int64_t>(I_Value); return *this; }
        FJSON&
        Set(FStringView I_Key, Bool I_Value) { Root[I_Key.GetNative()] = static_cast<bool>(I_Value); return *this; }
        FJSON&
        Set(FStringView I_Key, const FJSON& I_Value) { Root[I_Key.GetNative()] = I_Value.Root; return *this; }
        template<Concepts::JSONPath TPath> FJSON&
        Set(const TPath& I_Path, FStringView I_Value)
        {
            Json Value;
            Value = FString(I_Value);
            return SetPathValue(I_Path, std::move(Value));
        }

        template<Concepts::JSONPath TPath> FJSON&
        Set(const TPath& I_Path, Double I_Value)
        {
            Json Value;
            Value = I_Value;
            return SetPathValue(I_Path, std::move(Value));
        }

        template<Concepts::Integral T, Concepts::JSONPath TPath> FJSON&
        Set(const TPath& I_Path, T I_Value)
        {
            Json Value;
            Value = static_cast<std::int64_t>(I_Value);
            return SetPathValue(I_Path, std::move(Value));
        }

        template<Concepts::Boolean T, Concepts::JSONPath TPath> FJSON&
        Set(const TPath& I_Path, Bool I_Value)
        {
            Json Value;
            Value = static_cast<bool>(I_Value);
            return SetPathValue(I_Path, std::move(Value));
        }

        template<Concepts::JSONPath TPath> FJSON&
        Set(const TPath& I_Path, const FJSON& I_Value)
        {
            return SetPathValue(I_Path, Json(I_Value.Root));
        }
        FJSON&
        Set(FStringView I_Key, const TArray<FJSON>& I_Array)
        {
            Json Array = Json::array();
            for (const auto& Item : I_Array)
            { Array.push_back(Item.Root); }
            Root[I_Key.GetNative()] = Array;
            return *this;
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

        template<Concepts::JSONPath TPath> [[nodiscard]] FString
        GetString(const TPath& I_Path, FStringView I_DefaultValue = "") const
        {
            const auto Opt = TryGetString(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : FString(I_DefaultValue);
        }

        template<Concepts::JSONPath TPath> [[nodiscard]] TOptional<FString>
        TryGetString(const TPath& I_Path) const noexcept
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

        template<Concepts::JSONPath TPath> [[nodiscard]] FPath
        GetPath(const TPath& I_Path, const FPath& I_DefaultValue = FPath{""}) const
        {
            const auto Opt = TryGetPath(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        template<Concepts::JSONPath TPath> [[nodiscard]] TOptional<FPath>
        TryGetPath(const TPath& I_Path) const noexcept
        {
            const Json* Ptr = FindPath(Root, I_Path);
            if (!Ptr || !Ptr->is_string()) { return NullOpt; }
            try { return TOptional<FPath>(FPath{GetStringFromJsonValue(*Ptr)}); } catch (...) { return NullOpt; }
        }

        template<Concepts::FloatingPoint T>
        [[nodiscard]] T
        GetNumber(FStringView I_Key, T I_DefaultValue = T{0}) const noexcept
        {
            const auto Opt = TryGetNumber<T>(I_Key);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        template<Concepts::Integral T>
        [[nodiscard]] T
        GetNumber(FStringView I_Key, T I_DefaultValue = T{0}) const noexcept
        {
            const auto Opt = TryGetNumber<T>(I_Key);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        template<Concepts::FloatingPoint T>
        [[nodiscard]] TOptional<T>
        TryGetNumber(FStringView I_Key) const noexcept
        {
            const auto It = Root.find(I_Key.GetNative());
            if (It == Root.end() || !It->is_number()) { return NullOpt; }
            try { return TOptional<T>(It->get<T>()); } catch (...) { return NullOpt; }
        }

        template<Concepts::Integral T>
        [[nodiscard]] TOptional<T>
        TryGetNumber(FStringView I_Key) const noexcept
        {
            const auto It = Root.find(I_Key.GetNative());
            if (It == Root.end() || !It->is_number()) { return NullOpt; }
            try { return TOptional<T>(It->get<T>()); } catch (...) { return NullOpt; }
        }

        // Template version: returns the same type as DefaultValue
        template<Concepts::FloatingPoint T, Concepts::JSONPath TPath>
        [[nodiscard]] T
        GetNumber(const TPath& I_Path, T I_DefaultValue = T{0}) const noexcept
        {
            const auto Opt = TryGetNumber<T>(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        template<Concepts::Integral T, Concepts::JSONPath TPath>
        [[nodiscard]] T
        GetNumber(const TPath& I_Path, T I_DefaultValue = T{0}) const noexcept
        {
            const auto Opt = TryGetNumber<T>(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        template<Concepts::FloatingPoint T, Concepts::JSONPath TPath>
        [[nodiscard]] TOptional<T>
        TryGetNumber(const TPath& I_Path) const noexcept
        {
            const Json* p = FindPath(Root, I_Path);
            if (!p || !p->is_number()) { return NullOpt; }
            try { return TOptional<T>(p->get<T>()); } catch (...) { return NullOpt; }
        }

        template<Concepts::Integral T, Concepts::JSONPath TPath>
        [[nodiscard]] TOptional<T>
        TryGetNumber(const TPath& I_Path) const noexcept
        {
            const Json* p = FindPath(Root, I_Path);
            if (!p || !p->is_number()) { return NullOpt; }
            try { return TOptional<T>(p->get<T>()); } catch (...) { return NullOpt; }
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

        template<Concepts::JSONPath TPath> [[nodiscard]] Bool
        GetBool(const TPath& I_Path, Bool I_DefaultValue = False) const noexcept
        {
            const auto Opt = TryGetBool(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : I_DefaultValue;
        }

        template<Concepts::JSONPath TPath> [[nodiscard]] TOptional<Bool>
        TryGetBool(const TPath& I_Path) const noexcept
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

        template<Concepts::JSONPath TPath> [[nodiscard]] FJSON
        GetObject(const TPath& I_Path) const noexcept
        {
            const auto Opt = TryGetObject(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : FJSON{};
        }

        template<Concepts::JSONPath TPath> [[nodiscard]] TOptional<FJSON>
        TryGetObject(const TPath& I_Path) const noexcept
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

        template<typename T, Concepts::JSONPath TPath>
        [[nodiscard]] TArray<T>
        GetArray(const TPath& I_Path) const noexcept
        {
            const auto Opt = TryGetArray<T>(I_Path);
            return Opt.HasValue() ? std::move(Opt).GetValue() : TArray<T>{};
        }

        template<typename T, Concepts::JSONPath TPath>
        [[nodiscard]] TOptional<TArray<T>>
        TryGetArray(const TPath& I_Path) const noexcept
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

        template<Concepts::JSONPath TPath> FJSON&
        SetPathValue(const TPath& I_Path, Json I_Value)
        {
            if constexpr (std::same_as<std::remove_cvref_t<TPath>, FJSONPath>)
            {
                if (!I_Path.IsValid()) { return *this; }
                const auto& Tokens = I_Path.GetTokens();
                if (Tokens.IsEmpty()) { return *this; }
                if (!Root.is_object()) { Root = Json::object(); }
                Json* Current = std::addressof(Root);
                for (UInt64 I = 0; I < Tokens.GetSize() - 1; ++I)
                {
                    const auto& Token = Tokens[I];
                    if (Token.Type == FJSONPath::FToken::EType::Key)
                    {
                        auto& Next = (*Current)[Token.GetString().GetNative()];
                        if (!Next.is_object()) { Next = Json::object(); }
                        Current = std::addressof(Next);
                    }
                    else
                    {
                        if (!Current->is_array()) { *Current = Json::array(); }
                        while (Token.Index >= Current->size()) { Current->push_back(Json{}); }
                        Current = std::addressof((*Current)[Token.Index]);
                    }
                }
                const auto& Last = Tokens.Back();
                if (Last.Type == FJSONPath::FToken::EType::Key)
                {
                    if (!Current->is_object()) { *Current = Json::object(); }
                    (*Current)[Last.GetString().GetNative()] = std::move(I_Value);
                }
                else
                {
                    if (!Current->is_array()) { *Current = Json::array(); }
                    while (Last.Index >= Current->size()) { Current->push_back(Json{}); }
                    (*Current)[Last.Index] = std::move(I_Value);
                }
            }
            else
            {
                if (!I_Path.Valid || I_Path.Count == 0 || I_Path.Tokens == nullptr) { return *this; }
                if (!Root.is_object()) { Root = Json::object(); }
                Json* Current = std::addressof(Root);
                for (UInt32 I = 0; I < I_Path.Count - 1; ++I)
                {
                    const auto& Token = I_Path.Tokens[I];
                    if (Token.Type == FStaticJSONPathToken::EType::Key)
                    {
                        auto& Next = (*Current)[Token.GetString().GetNative()];
                        if (!Next.is_object()) { Next = Json::object(); }
                        Current = std::addressof(Next);
                    }
                    else
                    {
                        if (!Current->is_array()) { *Current = Json::array(); }
                        while (Token.Index >= Current->size()) { Current->push_back(Json{}); }
                        Current = std::addressof((*Current)[Token.Index]);
                    }
                }
                const auto& Last = I_Path.Tokens[I_Path.Count - 1];
                if (Last.Type == FStaticJSONPathToken::EType::Key)
                {
                    if (!Current->is_object()) { *Current = Json::object(); }
                    (*Current)[Last.GetString().GetNative()] = std::move(I_Value);
                }
                else
                {
                    if (!Current->is_array()) { *Current = Json::array(); }
                    while (Last.Index >= Current->size()) { Current->push_back(Json{}); }
                    (*Current)[Last.Index] = std::move(I_Value);
                }
            }
            return *this;
        }

        /** Resolves I_Path against I_Root. Returns pointer to the target value, or nullptr if not found or path invalid. */
        template<Concepts::JSONPath TPath> [[nodiscard]] static const Json*
        FindPath(const Json& I_Root, const TPath& I_Path) noexcept
        {
            if constexpr (std::same_as<std::remove_cvref_t<TPath>, FJSONPath>)
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
                        const auto It = Current->find(Token.GetString().GetNative());
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
            else
            {
                if (!I_Path.Valid)
                {
                    return nullptr;
                }
                if (I_Path.Count > 0 && I_Path.Tokens == nullptr)
                {
                    return nullptr;
                }
                const Json* Current = std::addressof(I_Root);
                for (UInt32 I = 0; I < I_Path.Count; ++I)
                {
                    const auto& Token = I_Path.Tokens[I];
                    if (Token.Type == FStaticJSONPathToken::EType::Key)
                    {
                        if (!Current->is_object())
                        {
                            return nullptr;
                        }
                        const auto It = Current->find(Token.GetString().GetNative());
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
        }
    };
    static_assert(sizeof(TOptional<FJSON>) == sizeof(FJSON));

    Bool operator==(const FJSON& I_Lhs, FIntrusiveUnsetOptionalState) noexcept
    { return I_Lhs.IsNull(); }
}
VISERA_MAKE_FORMATTER(Visera::FJSON, {}, "{}", I_Formatee.Dump());