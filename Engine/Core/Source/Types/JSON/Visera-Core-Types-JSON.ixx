module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.JSON;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.OS.FileSystem;
import charted.core;
import charted.json;

export namespace Visera
{
    using FJSONRoute = charted::DynamicRoute;
    template <charted::StringLiteral Route>
    using TJSONRoute = charted::StaticRoute<Route>;

    namespace Concepts
    {
        template <typename T>
        concept JSONRoute = charted::concepts::Route<T>;
    }

    class FJSON;
    template <> inline constexpr Bool HasIntrusiveUnsetOptionalState<FJSON> = True;

    class VISERA_CORE_API FJSON
    {
        using Json = charted::Json;

    public:
        [[nodiscard]] static TOptional<FJSON>
        Parse(FStringView I_JSONString)
        {
            if (auto Parsed = Json::Parse(I_JSONString.GetNative()); Parsed.has_value())
            {
                return TOptional<FJSON>(FJSON(std::move(Parsed.value())));
            }
            return NullOpt;
        }

        [[nodiscard]] static TOptional<FJSON>
        Load(const FPath& I_JSONFile)
        {
            if (auto Stream = FFileSystem::OpenIStream(I_JSONFile); Stream)
            {
                if (auto Parsed = Json::Parse(*Stream); Parsed.has_value())
                {
                    return TOptional<FJSON>(FJSON(std::move(Parsed.value())));
                }
            }
            return NullOpt;
        }

        [[nodiscard]] Bool IsNull() const noexcept { return Root.IsNull(); }
        [[nodiscard]] Bool IsDiscarded() const noexcept { return Root.IsDiscarded(); }
        [[nodiscard]] Bool Contains(FStringView I_Key) const noexcept { return Root.Contains(I_Key.GetNative()); }
        template <charted::concepts::Route RouteType>
        [[nodiscard]] Bool Contains(const RouteType& I_Route) const noexcept { return Root.Contains(I_Route); }

        void Clear() noexcept { Root.Clear(); }
        [[nodiscard]] FString Dump(Bool I_bPretty = True) const { return FString(Root.Dump(I_bPretty)); }

        FJSON& Set(FStringView I_Key, FStringView I_Value)
        {
            Root.Set(I_Key.GetNative(), std::string_view(I_Value.GetNative()));
            return *this;
        }

        FJSON& Set(FStringView I_Key, const FString& I_Value)
        {
            Root.Set(I_Key.GetNative(), I_Value.GetNative());
            return *this;
        }

        template <Concepts::FloatingPoint T>
        FJSON& Set(FStringView I_Key, T I_Value)
        {
            Root.Set(I_Key.GetNative(), I_Value);
            return *this;
        }

        template <Concepts::Integral T>
        FJSON& Set(FStringView I_Key, T I_Value)
        {
            Root.Set(I_Key.GetNative(), I_Value);
            return *this;
        }

        FJSON& Set(FStringView I_Key, Bool I_Value)
        {
            Root.Set(I_Key.GetNative(), static_cast<bool>(I_Value));
            return *this;
        }

        FJSON& Set(FStringView I_Key, const FJSON& I_Value)
        {
            Root.Set(I_Key.GetNative(), I_Value.Root);
            return *this;
        }

        template <charted::concepts::Route RouteType>
        FJSON& Set(const RouteType& I_Route, FStringView I_Value)
        {
            Root.Set(I_Route, std::string_view(I_Value.GetNative()));
            return *this;
        }

        template <Concepts::FloatingPoint T, charted::concepts::Route RouteType>
        FJSON& Set(const RouteType& I_Route, T I_Value)
        {
            Root.Set(I_Route, I_Value);
            return *this;
        }

        template <Concepts::Integral T, charted::concepts::Route RouteType>
        FJSON& Set(const RouteType& I_Route, T I_Value)
        {
            Root.Set(I_Route, I_Value);
            return *this;
        }

        template <Concepts::Boolean T, charted::concepts::Route RouteType>
        FJSON& Set(const RouteType& I_Route, T I_Value)
        {
            Root.Set(I_Route, static_cast<bool>(I_Value));
            return *this;
        }

        template <charted::concepts::Route RouteType>
        FJSON& Set(const RouteType& I_Route, const FJSON& I_Value)
        {
            Root.Set(I_Route, I_Value.Root);
            return *this;
        }

        [[nodiscard]] TOptional<FString>
        TryGetString(FStringView I_Key) const noexcept
        {
            if (auto Value = Root.TryGet<std::string>(I_Key.GetNative()); Value.has_value())
            {
                return TOptional<FString>(FString(std::move(Value.value())));
            }
            return NullOpt;
        }

        template <charted::concepts::Route RouteType>
        [[nodiscard]] TOptional<FString>
        TryGetString(const RouteType& I_Route) const noexcept
        {
            if (auto Value = Root.TryGet<std::string>(I_Route); Value.has_value())
            {
                return TOptional<FString>(FString(std::move(Value.value())));
            }
            return NullOpt;
        }

        [[nodiscard]] FString GetString(FStringView I_Key, FStringView I_DefaultValue = "") const
        {
            auto Value = TryGetString(I_Key);
            return Value.HasValue() ? std::move(Value.GetValue()) : FString(I_DefaultValue);
        }

        template <charted::concepts::Route RouteType>
        [[nodiscard]] FString GetString(const RouteType& I_Route, FStringView I_DefaultValue = "") const
        {
            auto Value = TryGetString(I_Route);
            return Value.HasValue() ? std::move(Value.GetValue()) : FString(I_DefaultValue);
        }

        [[nodiscard]] TOptional<FPath>
        TryGetPath(FStringView I_Key) const noexcept
        {
            auto Value = TryGetString(I_Key);
            return Value.HasValue() ? TOptional<FPath>(FPath{ std::move(Value.GetValue()) }) : NullOpt;
        }

        template <charted::concepts::Route RouteType>
        [[nodiscard]] TOptional<FPath>
        TryGetPath(const RouteType& I_Route) const noexcept
        {
            auto Value = TryGetString(I_Route);
            return Value.HasValue() ? TOptional<FPath>(FPath{ std::move(Value.GetValue()) }) : NullOpt;
        }

        [[nodiscard]] FPath GetPath(FStringView I_Key, const FPath& I_DefaultValue = FPath{""}) const
        {
            auto Value = TryGetPath(I_Key);
            return Value.HasValue() ? std::move(Value.GetValue()) : I_DefaultValue;
        }

        template <charted::concepts::Route RouteType>
        [[nodiscard]] FPath GetPath(const RouteType& I_Route, const FPath& I_DefaultValue = FPath{""}) const
        {
            auto Value = TryGetPath(I_Route);
            return Value.HasValue() ? std::move(Value.GetValue()) : I_DefaultValue;
        }

        template <Concepts::FloatingPoint T>
        [[nodiscard]] TOptional<T> TryGetNumber(FStringView I_Key) const noexcept
        {
            if (auto Value = Root.TryGet<T>(I_Key.GetNative()); Value.has_value())
            {
                return TOptional<T>(std::move(Value.value()));
            }
            return NullOpt;
        }

        template <Concepts::Integral T>
        [[nodiscard]] TOptional<T> TryGetNumber(FStringView I_Key) const noexcept
        {
            if (auto Value = Root.TryGet<T>(I_Key.GetNative()); Value.has_value())
            {
                return TOptional<T>(std::move(Value.value()));
            }
            return NullOpt;
        }

        template <Concepts::FloatingPoint T, charted::concepts::Route RouteType>
        [[nodiscard]] TOptional<T> TryGetNumber(const RouteType& I_Route) const noexcept
        {
            if (auto Value = Root.TryGet<T>(I_Route); Value.has_value())
            {
                return TOptional<T>(std::move(Value.value()));
            }
            return NullOpt;
        }

        template <Concepts::Integral T, charted::concepts::Route RouteType>
        [[nodiscard]] TOptional<T> TryGetNumber(const RouteType& I_Route) const noexcept
        {
            if (auto Value = Root.TryGet<T>(I_Route); Value.has_value())
            {
                return TOptional<T>(std::move(Value.value()));
            }
            return NullOpt;
        }

        template <Concepts::FloatingPoint T>
        [[nodiscard]] T GetNumber(FStringView I_Key, T I_DefaultValue = T{ 0 }) const noexcept
        {
            auto Value = TryGetNumber<T>(I_Key);
            return Value.HasValue() ? std::move(Value.GetValue()) : I_DefaultValue;
        }

        template <Concepts::Integral T>
        [[nodiscard]] T GetNumber(FStringView I_Key, T I_DefaultValue = T{ 0 }) const noexcept
        {
            auto Value = TryGetNumber<T>(I_Key);
            return Value.HasValue() ? std::move(Value.GetValue()) : I_DefaultValue;
        }

        template <Concepts::FloatingPoint T, charted::concepts::Route RouteType>
        [[nodiscard]] T GetNumber(const RouteType& I_Route, T I_DefaultValue = T{ 0 }) const noexcept
        {
            auto Value = TryGetNumber<T>(I_Route);
            return Value.HasValue() ? std::move(Value.GetValue()) : I_DefaultValue;
        }

        template <Concepts::Integral T, charted::concepts::Route RouteType>
        [[nodiscard]] T GetNumber(const RouteType& I_Route, T I_DefaultValue = T{ 0 }) const noexcept
        {
            auto Value = TryGetNumber<T>(I_Route);
            return Value.HasValue() ? std::move(Value.GetValue()) : I_DefaultValue;
        }

        [[nodiscard]] TOptional<Bool>
        TryGetBool(FStringView I_Key) const noexcept
        {
            if (auto Value = Root.TryGet<bool>(I_Key.GetNative()); Value.has_value())
            {
                return TOptional<Bool>(static_cast<Bool>(Value.value()));
            }
            return NullOpt;
        }

        template <charted::concepts::Route RouteType>
        [[nodiscard]] TOptional<Bool>
        TryGetBool(const RouteType& I_Route) const noexcept
        {
            if (auto Value = Root.TryGet<bool>(I_Route); Value.has_value())
            {
                return TOptional<Bool>(static_cast<Bool>(Value.value()));
            }
            return NullOpt;
        }

        [[nodiscard]] Bool GetBool(FStringView I_Key, Bool I_DefaultValue = False) const noexcept
        {
            auto Value = TryGetBool(I_Key);
            return Value.HasValue() ? std::move(Value.GetValue()) : I_DefaultValue;
        }

        template <charted::concepts::Route RouteType>
        [[nodiscard]] Bool GetBool(const RouteType& I_Route, Bool I_DefaultValue = False) const noexcept
        {
            auto Value = TryGetBool(I_Route);
            return Value.HasValue() ? std::move(Value.GetValue()) : I_DefaultValue;
        }

        [[nodiscard]] TOptional<FJSON>
        TryGetObject(FStringView I_Key) const noexcept
        {
            if (auto Value = Root.TryGet<Json>(I_Key.GetNative()); Value.has_value())
            {
                return TOptional<FJSON>(FJSON(std::move(Value.value())));
            }
            return NullOpt;
        }

        template <charted::concepts::Route RouteType>
        [[nodiscard]] TOptional<FJSON>
        TryGetObject(const RouteType& I_Route) const noexcept
        {
            if (auto Value = Root.TryGet<Json>(I_Route); Value.has_value())
            {
                return TOptional<FJSON>(FJSON(std::move(Value.value())));
            }
            return NullOpt;
        }

        [[nodiscard]] FJSON GetObject(FStringView I_Key) const noexcept
        {
            auto Value = TryGetObject(I_Key);
            return Value.HasValue() ? std::move(Value.GetValue()) : FJSON{};
        }

        template <charted::concepts::Route RouteType>
        [[nodiscard]] FJSON GetObject(const RouteType& I_Route) const noexcept
        {
            auto Value = TryGetObject(I_Route);
            return Value.HasValue() ? std::move(Value.GetValue()) : FJSON{};
        }

        [[nodiscard]] Json& GetNative() noexcept { return Root; }
        [[nodiscard]] const Json& GetNative() const noexcept { return Root; }

        FJSON() = default;
        FJSON(const FJSON&) = default;
        FJSON(FJSON&&) noexcept = default;
        FJSON& operator=(const FJSON&) = default;
        FJSON& operator=(FJSON&&) noexcept = default;
        explicit FJSON(Json&& I_NativeJSON) noexcept : Root(std::move(I_NativeJSON)) {}

        FJSON(FIntrusiveUnsetOptionalState) noexcept : Root() {}
        VISERA_CORE_API friend Bool operator==(const FJSON& I_Lhs, FIntrusiveUnsetOptionalState) noexcept;

    private:
        Json Root{};
    };

    inline Bool operator==(const FJSON& I_Lhs, FIntrusiveUnsetOptionalState) noexcept
    {
        return I_Lhs.IsNull();
    }

    static_assert(sizeof(TOptional<FJSON>) == sizeof(FJSON));
}
VISERA_MAKE_FORMATTER(Visera::FJSON, {}, "{}", I_Formatee.Dump());