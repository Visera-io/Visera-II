module;
#include <Visera-Core.hpp>
#include <nlohmann/json.hpp>
export module Visera.Core.Types.JSON;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Log;

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
        Contains(FStringView I_Key) const noexcept { return Data.contains(FString(I_Key)); }
        void
        Clear() noexcept { Data = Json{}; LastError.clear(); }
        [[nodiscard]] FString
        Dump(Bool bPretty = True) const { return bPretty ? Data.dump(4) : Data.dump(); }
        [[nodiscard]] const FString&
        GetLastError() const noexcept { return LastError; }
        void
        Set(FStringView I_Key, FStringView I_Value) { Data[FString(I_Key)] = FString(I_Value); }
        void
        Set(FStringView I_Key, Double I_Value) { Data[FString(I_Key)] = I_Value; }
        void
        Set(FStringView I_Key, Bool I_Value) { Data[FString(I_Key)] = static_cast<bool>(I_Value); }
        // ---- Get (safe) ----
        [[nodiscard]] FString
        GetString(FStringView I_Key, FStringView I_DefaultValue = "") const
        {
            const auto It = Data.find(FString(I_Key));
            if (It == Data.end() || !It->is_string()) { return FString(I_DefaultValue); }
            try { return It->get<FString>(); } catch (...) { return FString(I_DefaultValue); }
        }

        [[nodiscard]] Bool
        TryGetString(FStringView I_Key, FString* O_Value) const noexcept
        {
            CHECK(O_Value != nullptr);
            const auto It = Data.find(FString(I_Key));
            if (It == Data.end() || !It->is_string()) { return False; }
            try { *O_Value = It->get<FString>(); return True; } catch (...) { return False; }
        }

        [[nodiscard]] Double
        GetNumber(FStringView I_Key, Double I_DefaultValue = 0.0) const noexcept
        {
            Double Value = 0.0;
            return TryGetNumber(I_Key, &Value) ? Value : I_DefaultValue;
        }

        [[nodiscard]] Bool
        TryGetNumber(FStringView I_Key, Double* O_Value) const noexcept
        {
            CHECK(O_Value != nullptr);
            const auto It = Data.find(FString(I_Key));
            if (It == Data.end() || !It->is_number()) { return False; }
            try { *O_Value = It->get<Double>(); return True; } catch (...) { return False; }
        }

        [[nodiscard]] Bool
        GetBool(FStringView I_Key, Bool I_DefaultValue = False) const noexcept
        {
            Bool Value = False;
            return TryGetBool(I_Key, &Value) ? Value : I_DefaultValue;
        }

        [[nodiscard]] Bool
        TryGetBool(FStringView I_Key, Bool* O_Value) const noexcept
        {
            CHECK(O_Value != nullptr);
            const auto It = Data.find(FString(I_Key));
            if (It == Data.end() || !It->is_boolean()) { return False; }
            try { *O_Value = static_cast<Bool>(It->get<bool>()); return True; } catch (...) { return False; }
        }

                [[nodiscard]] Bool
        ContainsPath(FStringView I_Path) const noexcept
        {
            const Json* Node = nullptr;
            return TryResolvePathConst(Data, I_Path, &Node);
        }

        [[nodiscard]] FString
        GetStringPath(FStringView I_Path, FStringView I_DefaultValue = "") const
        {
            const Json* Node = nullptr;
            if (!TryResolvePathConst(Data, I_Path, &Node) || Node == nullptr || !Node->is_string()) { return FString(I_DefaultValue); }
            try { return Node->get<FString>(); } catch (...) { return FString(I_DefaultValue); }
        }

        [[nodiscard]] Bool
        TryGetStringPath(FStringView I_Path, FString* O_Value) const noexcept
        {
            CHECK(O_Value != nullptr);
            const Json* Node = nullptr;
            if (!TryResolvePathConst(Data, I_Path, &Node) || Node == nullptr || !Node->is_string()) { return False; }
            try { *O_Value = Node->get<FString>(); return True; } catch (...) { return False; }
        }

        [[nodiscard]] Double
        GetNumberPath(FStringView I_Path, Double I_DefaultValue = 0.0) const noexcept
        {
            Double Value = 0.0;
            return TryGetNumberPath(I_Path, &Value) ? Value : I_DefaultValue;
        }

        [[nodiscard]] Bool
        TryGetNumberPath(FStringView I_Path, Double* O_Value) const noexcept
        {
            CHECK(O_Value != nullptr);
            const Json* Node = nullptr;
            if (!TryResolvePathConst(Data, I_Path, &Node) || Node == nullptr || !Node->is_number()) { return False; }
            try { *O_Value = Node->get<Double>(); return True; } catch (...) { return False; }
        }

        [[nodiscard]] Bool
        GetBoolPath(FStringView I_Path, Bool I_DefaultValue = False) const noexcept
        {
            Bool Value = False;
            return TryGetBoolPath(I_Path, &Value) ? Value : I_DefaultValue;
        }

        [[nodiscard]] Bool
        TryGetBoolPath(FStringView I_Path, Bool* O_Value) const noexcept
        {
            CHECK(O_Value != nullptr);
            const Json* Node = nullptr;
            if (!TryResolvePathConst(Data, I_Path, &Node) || Node == nullptr || !Node->is_boolean()) { return False; }
            try { *O_Value = static_cast<Bool>(Node->get<bool>()); return True; } catch (...) { return False; }
        }

        // ---- Set with dotted path ----
        Bool
        SetPath(FStringView I_Path, FStringView I_Value) noexcept
        {
            Json* Node = nullptr;
            FString Key{};
            if (!TryResolvePathForWrite(Data, I_Path, &Node, &Key) || Node == nullptr) { return False; }
            (*Node)[Key] = FString(I_Value);
            return True;
        }

        Bool
        SetPath(FStringView I_Path, Double I_Value) noexcept
        {
            Json* Node = nullptr;
            FString Key{};
            if (!TryResolvePathForWrite(Data, I_Path, &Node, &Key) || Node == nullptr) { return False; }
            (*Node)[Key] = I_Value;
            return True;
        }

        Bool
        SetPath(FStringView I_Path, Bool I_Value) noexcept
        {
            Json* Node = nullptr;
            FString Key{};
            if (!TryResolvePathForWrite(Data, I_Path, &Node, &Key) || Node == nullptr) { return False; }
            (*Node)[Key] = static_cast<bool>(I_Value);
            return True;
        }

        Bool
        SetPath(std::string_view I_Path, const char* I_Value) noexcept
        {  return SetPath(I_Path, std::string_view{ I_Value ? I_Value : "" }); }

        // ---- Get (view, UNSAFE by design) ----
        // This returns a view into json's internal string storage; it becomes dangling if Data is modified.
        [[nodiscard]] Bool
        TryGetStringView(FStringView I_Key, FStringView* O_Value) const noexcept
        {
            CHECK(O_Value != nullptr);
            const auto It = Data.find(FString(I_Key));
            if (It == Data.end() || !It->is_string()) { return False; }
            try { *O_Value = It->get_ref<const FString&>(); return True; } catch (...) { return False; }
        }

        [[nodiscard]] Json&
        GetNative() noexcept { return Data; }
        [[nodiscard]] const Json&
        GetNative() const noexcept { return Data; }

        FJSON() = default;
        explicit FJSON(FStringView I_JSONData) { Parse(I_JSONData); }

        [[nodiscard]] Bool
        Parse(FStringView I_JSONData) noexcept
        {
            LastError.clear();
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
                LOG_ERROR("FJSON::Parse failed: {}", LastError);
                Data = Json{};
                return False;
            }
        }

    private:
        Json    Data{};
        FString LastError{};

    private:
                [[nodiscard]] static Bool
        TrySplitPathOnce(FStringView I_Path, FStringView* O_Head, FStringView* O_Tail) noexcept
        {
            CHECK(O_Head != nullptr && O_Tail != nullptr);
            if (I_Path.empty()) { return False; }
            const size_t Dot = I_Path.find('.');
            if (Dot == FStringView::npos)
            {
                *O_Head = I_Path;
                *O_Tail = FStringView{};
                return True;
            }
            *O_Head = I_Path.substr(0, Dot);
            *O_Tail = I_Path.substr(Dot + 1);
            return True;
        }

        [[nodiscard]] static Bool
        IsValidPathKey(FStringView I_Key) noexcept
        {
            if (I_Key.empty()) { return False; }
            // Reject empty segments like "a..b" or ".a" or "a."
            return True;
        }

        [[nodiscard]] static Bool
        TryResolvePathConst(const Json& I_Root, FStringView I_Path, const Json** O_Node) noexcept
        {
            CHECK(O_Node != nullptr);
            *O_Node = nullptr;

            FStringView Path = I_Path;
            const Json* Cur = &I_Root;

            FStringView Head{}, Tail{};
            while (TrySplitPathOnce(Path, &Head, &Tail))
            {
                if (!IsValidPathKey(Head)) { return False; }
                if (!Cur->is_object()) { return False; }

                auto It = Cur->find(FString(Head));
                if (It == Cur->end()) { return False; }

                Cur = &(*It);
                if (Tail.empty()) { *O_Node = Cur; return True; }
                Path = Tail;
            }
            return False;
        }

        // For SetPath: resolve all but last segment, creating intermediate objects when missing.
        [[nodiscard]] static Bool
        TryResolvePathForWrite(Json& IO_Root, FStringView I_Path, Json** O_ParentObject, FString* O_LastKey) noexcept
        {
            CHECK(O_ParentObject != nullptr && O_LastKey != nullptr);
            *O_ParentObject = nullptr;
            O_LastKey->clear();

            FStringView Path = I_Path;
            if (Path.empty()) { return False; }

            FStringView Head{}, Tail{};
            Json* Cur = &IO_Root;

            // Special case: no dot => parent is root object, last key is the path.
            if (Path.find('.') == FStringView::npos)
            {
                if (!Cur->is_object() && !Cur->is_null()) { return False; }
                if (Cur->is_null()) { *Cur = Json::object(); }
                if (!IsValidPathKey(Path)) { return False; }
                *O_ParentObject = Cur;
                *O_LastKey = FString(Path);
                return True;
            }

            while (TrySplitPathOnce(Path, &Head, &Tail))
            {
                if (!IsValidPathKey(Head)) { return False; }

                // If this is the last segment, return parent + last key.
                if (Tail.empty())
                {
                    if (!Cur->is_object() && !Cur->is_null()) { return False; }
                    if (Cur->is_null()) { *Cur = Json::object(); }
                    *O_ParentObject = Cur;
                    *O_LastKey = FString(Head);
                    return True;
                }

                // Ensure current is object.
                if (!Cur->is_object() && !Cur->is_null()) { return False; }
                if (Cur->is_null()) { *Cur = Json::object(); }

                // Descend/create.
                Json& Next = (*Cur)[FString(Head)];
                if (!Next.is_object() && !Next.is_null()) { return False; }
                if (Next.is_null()) { Next = Json::object(); }

                Cur = &Next;
                Path = Tail;
            }

            return False;
        }
    };
}
