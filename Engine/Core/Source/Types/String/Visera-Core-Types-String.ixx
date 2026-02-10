module;
#include <Visera-Core.hpp>
#include <string>
#include <ranges>
export module Visera.Core.Types.String;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Containers.Array;
import Visera.Core.Types.Optional;
import Visera.Core.Algorithm;

export namespace Visera
{
    class FString;
    template<> inline constexpr Bool HasIntrusiveUnsetOptionalState<FString> = True;

    class FStringView;
    template<> inline constexpr Bool HasIntrusiveUnsetOptionalState<FStringView> = True;

    /**
     * Wrapper around std::string_view that satisfies std::ranges::range.
     * Implicitly converts to/from std::string_view. Use as view-over string data.
     */
    class VISERA_CORE_API FStringView
    {
    public:
        using ViewType             = std::string_view;
        using ValueType            = ViewType::value_type;
        using Iterator             = ViewType::iterator;
        using ConstIterator        = ViewType::const_iterator;
        using ReverseIterator     = ViewType::reverse_iterator;
        using ConstReverseIterator = ViewType::const_reverse_iterator;
        using Reference            = ViewType::reference;
        using ConstReference       = ViewType::const_reference;
        using SizeType             = ViewType::size_type;

        static constexpr SizeType NPos = ViewType::npos;
        
        [[nodiscard]] inline const ViewType&
        GetNative() const { return View; }
        [[nodiscard]] inline ViewType&
        GetNative()       { return View; }

    private:
        ViewType View;

    public:
        constexpr FStringView() noexcept = default;
        constexpr FStringView(FIntrusiveUnsetOptionalState) noexcept {}
        constexpr ~FStringView() = default;

        constexpr FStringView(const ViewType& I_View) noexcept
            : View(I_View)
        {
        }

        constexpr FStringView(const char* I_Str) noexcept
            : View(I_Str ? I_Str : "")
        {
        }

        constexpr FStringView(const char* I_Str, SizeType I_Count) noexcept
            : View(I_Str ? I_Str : "", I_Str ? I_Count : 0)
        {
        }

        FStringView(const FString& I_Str);

        constexpr FStringView(const std::string& I_Str) noexcept
            : View(I_Str)
        {
        }

        constexpr FStringView& operator=(const FStringView& I_Other) noexcept = default;

        constexpr FStringView& operator=(const ViewType& I_View) noexcept
        {
            View = I_View;
            return *this;
        }

        constexpr FStringView& operator=(const char* I_Str) noexcept
        {
            View = I_Str ? I_Str : "";
            return *this;
        }

        [[nodiscard]] constexpr operator ViewType() const noexcept
        {
            return View;
        }

        [[nodiscard]] constexpr Bool IsEmpty() const noexcept
        {
            return View.empty();
        }

        [[nodiscard]] constexpr UInt64 GetSize() const noexcept
        {
            return View.size();
        }

        [[nodiscard]] constexpr const char* Data() const noexcept
        {
            return View.data();
        }

        [[nodiscard]] constexpr ConstReference operator[](SizeType I_Index) const noexcept
        {
            return View[I_Index];
        }

        [[nodiscard]] constexpr ConstReference At(SizeType I_Index) const
        {
            return View.at(I_Index);
        }

        [[nodiscard]] constexpr ConstReference Front() const noexcept
        {
            return View.front();
        }

        [[nodiscard]] constexpr ConstReference Back() const noexcept
        {
            return View.back();
        }

        // Required for std::ranges and range-based for; names fixed by C++ standard.
        [[nodiscard]] constexpr ConstIterator begin() const noexcept
        {
            return View.begin();
        }

        [[nodiscard]] constexpr ConstIterator cbegin() const noexcept
        {
            return View.cbegin();
        }

        [[nodiscard]] constexpr ConstIterator end() const noexcept
        {
            return View.end();
        }

        [[nodiscard]] constexpr ConstIterator cend() const noexcept
        {
            return View.cend();
        }

        [[nodiscard]] constexpr ConstReverseIterator rbegin() const noexcept
        {
            return View.rbegin();
        }

        [[nodiscard]] constexpr ConstReverseIterator crbegin() const noexcept
        {
            return View.crbegin();
        }

        [[nodiscard]] constexpr ConstReverseIterator rend() const noexcept
        {
            return View.rend();
        }

        [[nodiscard]] constexpr ConstReverseIterator crend() const noexcept
        {
            return View.crend();
        }

        [[nodiscard]] constexpr FStringView Substr(SizeType I_Pos = 0, SizeType I_Count = ViewType::npos) const noexcept
        {
            return FStringView(View.substr(I_Pos, I_Count));
        }

        [[nodiscard]] constexpr FStringView SubString(SizeType I_Pos = 0, SizeType I_Count = ViewType::npos) const noexcept
        {
            return Substr(I_Pos, I_Count);
        }

        [[nodiscard]] constexpr SizeType Find(FStringView I_Sv, SizeType I_Pos = 0) const noexcept
        {
            return View.find(I_Sv.GetNative(), I_Pos);
        }

        [[nodiscard]] constexpr SizeType Find(char I_Ch, SizeType I_Pos = 0) const noexcept
        {
            return View.find(I_Ch, I_Pos);
        }

        [[nodiscard]] constexpr SizeType Find(const char* I_Str, SizeType I_Pos, SizeType I_Count) const noexcept
        {
            return View.find(I_Str, I_Pos, I_Count);
        }

        [[nodiscard]] constexpr SizeType Find(const char* I_Str, SizeType I_Pos = 0) const noexcept
        {
            return View.find(I_Str, I_Pos);
        }

        [[nodiscard]] constexpr SizeType FindLast(FStringView I_Sv, SizeType I_Pos = ViewType::npos) const noexcept
        {
            return View.rfind(I_Sv.GetNative(), I_Pos);
        }

        [[nodiscard]] constexpr SizeType FindLast(char I_Ch, SizeType I_Pos = ViewType::npos) const noexcept
        {
            return View.rfind(I_Ch, I_Pos);
        }

        [[nodiscard]] constexpr SizeType FindLast(const char* I_Str, SizeType I_Pos = ViewType::npos) const noexcept
        {
            return View.rfind(I_Str ? I_Str : "", I_Pos);
        }

        [[nodiscard]] constexpr Bool StartsWith(FStringView I_Sv) const noexcept
        {
            return View.starts_with(I_Sv.GetNative());
        }

        [[nodiscard]] constexpr Bool StartsWith(char I_Ch) const noexcept
        {
            return View.starts_with(I_Ch);
        }

        [[nodiscard]] constexpr Bool EndsWith(FStringView I_Sv) const noexcept
        {
            return View.ends_with(I_Sv.GetNative());
        }

        [[nodiscard]] constexpr Bool EndsWith(char I_Ch) const noexcept
        {
            return View.ends_with(I_Ch);
        }

        [[nodiscard]] constexpr Bool Contains(FStringView I_Sv) const noexcept
        {
            return View.contains(I_Sv.GetNative());
        }

        [[nodiscard]] constexpr Bool Contains(char I_Ch) const noexcept
        {
            return View.contains(I_Ch);
        }

        VISERA_CORE_API
        friend Bool operator==(const FStringView& I_Lhs, FIntrusiveUnsetOptionalState) noexcept;
    };

    inline Bool operator==(const FStringView& I_Lhs, FIntrusiveUnsetOptionalState) noexcept
    { return I_Lhs.IsEmpty(); }

    /**
     * Wrapper around std::string that satisfies std::ranges::range and provides
     * Split() and other range-based helpers. Use with std::ranges::views::split etc.
     */
    class VISERA_CORE_API FString
    {
    public:
        using StringType            = std::string;
        using ValueType             = StringType::value_type;
        using Iterator              = StringType::iterator;
        using ConstIterator         = StringType::const_iterator;
        using ReverseIterator       = StringType::reverse_iterator;
        using ConstReverseIterator  = StringType::const_reverse_iterator;
        using Reference             = StringType::reference;
        using ConstReference        = StringType::const_reference;
        using SizeType              = StringType::size_type;

        static constexpr SizeType NPos = StringType::npos;

        template<typename... Args> [[nodiscard]] static FString
        Format(fmt::format_string<Args...> I_Fmt, Args &&... I_Args)
        { return fmt::format(I_Fmt, std::forward<Args>(I_Args)...); }

        [[nodiscard]] inline const StringType&
        GetNative() const { return String; }
        [[nodiscard]] inline StringType&
        GetNative()       { return String; }

    private:
        StringType String;

    public:
        FString()  = default;
        FString(FIntrusiveUnsetOptionalState) noexcept {}
        ~FString() = default;
        FString(const StringType& I_Str) : String(I_Str) {}
        FString(StringType&& I_Str) noexcept : String(std::move(I_Str)) {}
        FString(const char* I_Str) : String(I_Str ? I_Str : "") {}
        FString(const char* I_Str, SizeType I_Count) : String(I_Str ? I_Str : "", I_Str ? I_Count : 0) {}
        FString(FStringView I_Sv) : String(I_Sv.GetNative()) { }
        FString(std::string_view I_Sv) : String(I_Sv) { }
        template<typename InputIt>
        FString(InputIt I_First, InputIt I_Last) : String(I_First, I_Last) {}
        FString(SizeType I_Count, char I_Ch) : String(I_Count, I_Ch) { }
        FString(std::initializer_list<char> I_Init) : String(I_Init) {}
        FString(const FString& I_Other) : String(I_Other.String) { }
        template<Concepts::Byte ByteType>
        FString(const TArray<ByteType>& I_Bytes) { String.assign(reinterpret_cast<const char*>(I_Bytes.Data()), I_Bytes.GetSize()); }

        FString& operator=(const FString& I_Other)
        {
            if (this != &I_Other)
            {
                String = I_Other.String;
            }
            return *this;
        }

        FString(FString&& I_Other) noexcept = default;

        FString& operator=(FString&& I_Other) noexcept = default;

        FString& operator=(const StringType& I_Str)
        {
            String = I_Str;
            return *this;
        }

        FString& operator=(StringType&& I_Str) noexcept
        {
            String = std::move(I_Str);
            return *this;
        }

        FString& operator=(const char* I_Str)
        {
            String = I_Str ? I_Str : "";
            return *this;
        }

        FString& operator=(FStringView I_Sv)
        {
            String = I_Sv.GetNative();
            return *this;
        }

        FString& operator=(std::string_view I_Sv)
        {
            String = I_Sv;
            return *this;
        }

        FString& operator=(std::initializer_list<char> I_Init)
        {
            String = I_Init;
            return *this;
        }

        /** Implicit conversion to std::string_view (non-owned view). */
        [[nodiscard]] operator std::string_view() const noexcept
        {
            return std::string_view(String);
        }

        /** Implicit conversion to std::string (copy). */
        [[nodiscard]] operator std::string() const
        {
            return String;
        }

        // Capacity
        [[nodiscard]] Bool IsEmpty() const
        {
            return String.empty();
        }

        [[nodiscard]] UInt64 GetSize() const
        {
            return static_cast<UInt64>(String.size());
        }

        [[nodiscard]] UInt64 GetMaxSize() const
        {
            return static_cast<UInt64>(String.max_size());
        }

        void Reserve(SizeType I_NewCapacity)
        {
            String.reserve(I_NewCapacity);
        }

        [[nodiscard]] UInt64 GetCapacity() const
        {
            return static_cast<UInt64>(String.capacity());
        }

        void ShrinkToFit()
        {
            String.shrink_to_fit();
        }

        // Element access
        [[nodiscard]] char& operator[](SizeType I_Index)
        {
            return String[I_Index];
        }

        [[nodiscard]] const char& operator[](SizeType I_Index) const
        {
            return String[I_Index];
        }

        [[nodiscard]] char& At(SizeType I_Index)
        {
            return String.at(I_Index);
        }

        [[nodiscard]] const char& At(SizeType I_Index) const
        {
            return String.at(I_Index);
        }

        [[nodiscard]] char& Front()
        {
            return String.front();
        }

        [[nodiscard]] const char& Front() const
        {
            return String.front();
        }

        [[nodiscard]] char& Back()
        {
            return String.back();
        }

        [[nodiscard]] const char& Back() const
        {
            return String.back();
        }

        [[nodiscard]] char* Data()
        {
            return String.data();
        }

        [[nodiscard]] const char* Data() const
        {
            return String.data();
        }

        // Required for std::ranges and range-based for; names fixed by C++ standard.
        [[nodiscard]] Iterator begin()
        {
            return String.begin();
        }

        [[nodiscard]] ConstIterator begin() const
        {
            return String.begin();
        }

        [[nodiscard]] ConstIterator cbegin() const
        {
            return String.cbegin();
        }

        [[nodiscard]] Iterator end()
        {
            return String.end();
        }

        [[nodiscard]] ConstIterator end() const
        {
            return String.end();
        }

        [[nodiscard]] ConstIterator cend() const
        {
            return String.cend();
        }

        [[nodiscard]] ReverseIterator rbegin()
        {
            return String.rbegin();
        }

        [[nodiscard]] ConstReverseIterator rbegin() const
        {
            return String.rbegin();
        }

        [[nodiscard]] ConstReverseIterator crbegin() const
        {
            return String.crbegin();
        }

        [[nodiscard]] ReverseIterator rend()
        {
            return String.rend();
        }

        [[nodiscard]] ConstReverseIterator rend() const
        {
            return String.rend();
        }

        [[nodiscard]] ConstReverseIterator crend() const
        {
            return String.crend();
        }

        // Modifiers
        void Clear()
        {
            String.clear();
        }

        FString& operator+=(const FString& I_Other)
        {
            String += I_Other.String;
            return *this;
        }

        FString& operator+=(const StringType& I_Str)
        {
            String += I_Str;
            return *this;
        }

        FString& operator+=(const char* I_Str)
        {
            String += (I_Str ? I_Str : "");
            return *this;
        }

        FString& operator+=(const char I_Ch)
        {
            String += I_Ch;
            return *this;
        }

        FString& operator+=(FStringView I_Sv)
        {
            String += I_Sv.GetNative();
            return *this;
        }

        FString& Append(const char* I_Str)
        {
            String.append(I_Str ? I_Str : "");
            return *this;
        }

        FString& Append(const char* I_Str, SizeType I_Count)
        {
            if (I_Str)
            {
                String.append(I_Str, I_Count);
            }
            return *this;
        }

        FString& Append(FStringView I_Sv)
        {
            String.append(I_Sv.GetNative());
            return *this;
        }

        FString& Append(const char I_Ch)
        {
            String.append(1, I_Ch);
            return *this;
        }

        FString& Append(SizeType I_Count, char I_Ch)
        {
            String.append(I_Count, I_Ch);
            return *this;
        }

        template<typename InputIt>
        FString& Append(InputIt I_First, InputIt I_Last)
        {
            String.append(I_First, I_Last);
            return *this;
        }

        void PushBack(char I_Ch)
        {
            String.push_back(I_Ch);
        }

        void PopBack()
        {
            String.pop_back();
        }

        FString& Assign(const StringType& I_Str)
        {
            String.assign(I_Str);
            return *this;
        }

        FString& Assign(const char* I_Str)
        {
            String.assign(I_Str ? I_Str : "");
            return *this;
        }

        FString& Assign(const char* I_Str, SizeType I_Count)
        {
            if (I_Str)
            {
                String.assign(I_Str, I_Count);
            }
            return *this;
        }

        FString& Assign(FStringView I_Sv)
        {
            String.assign(I_Sv.GetNative());
            return *this;
        }

        FString& Assign(SizeType I_Count, char I_Ch)
        {
            String.assign(I_Count, I_Ch);
            return *this;
        }

        template<typename InputIt>
        FString& Assign(InputIt I_First, InputIt I_Last)
        {
            String.assign(I_First, I_Last);
            return *this;
        }

        Iterator Insert(ConstIterator I_Pos, char I_Ch)
        {
            return String.insert(I_Pos, I_Ch);
        }

        FString& Insert(SizeType I_Index, const StringType& I_Str)
        {
            String.insert(I_Index, I_Str);
            return *this;
        }

        FString& Insert(SizeType I_Index, const char* I_Str)
        {
            if (I_Str)
            {
                String.insert(I_Index, I_Str);
            }
            return *this;
        }

        FString& Insert(SizeType I_Index, const char* I_Str, SizeType I_Count)
        {
            if (I_Str)
            {
                String.insert(I_Index, I_Str, I_Count);
            }
            return *this;
        }

        FString& Insert(SizeType I_Index, FStringView I_Sv)
        {
            String.insert(I_Index, I_Sv.GetNative());
            return *this;
        }

        FString& Insert(SizeType I_Index, SizeType I_Count, char I_Ch)
        {
            String.insert(I_Index, I_Count, I_Ch);
            return *this;
        }

        Iterator Insert(ConstIterator I_Pos, SizeType I_Count, char I_Ch)
        {
            return String.insert(I_Pos, I_Count, I_Ch);
        }

        template<typename InputIt>
        Iterator Insert(ConstIterator I_Pos, InputIt I_First, InputIt I_Last)
        {
            return String.insert(I_Pos, I_First, I_Last);
        }

        FString& Erase(SizeType I_Index = 0, SizeType I_Count = StringType::npos)
        {
            String.erase(I_Index, I_Count);
            return *this;
        }

        Iterator Erase(ConstIterator I_Pos)
        {
            return String.erase(I_Pos);
        }

        Iterator Erase(ConstIterator I_First, ConstIterator I_Last)
        {
            return String.erase(I_First, I_Last);
        }

        void Swap(FString& I_Other)
        {
            String.swap(I_Other.String);
        }

        void Resize(SizeType I_Count)
        {
            String.resize(I_Count);
        }

        void Resize(SizeType I_Count, char I_Ch)
        {
            String.resize(I_Count, I_Ch);
        }

        // --- String search operations ---

        [[nodiscard]] SizeType Find(FStringView I_Sv, SizeType I_Pos = 0) const noexcept
        {
            return String.find(I_Sv.GetNative(), I_Pos);
        }

        [[nodiscard]] SizeType Find(char I_Ch, SizeType I_Pos = 0) const noexcept
        {
            return String.find(I_Ch, I_Pos);
        }

        [[nodiscard]] SizeType Find(const char* I_Str, SizeType I_Pos, SizeType I_Count) const noexcept
        {
            return String.find(I_Str ? I_Str : "", I_Pos, I_Count);
        }

        [[nodiscard]] SizeType Find(const char* I_Str, SizeType I_Pos = 0) const noexcept
        {
            return String.find(I_Str ? I_Str : "", I_Pos);
        }

        [[nodiscard]] SizeType FindLast(FStringView I_Sv, SizeType I_Pos = StringType::npos) const noexcept
        {
            return String.rfind(I_Sv.GetNative(), I_Pos);
        }

        [[nodiscard]] SizeType FindLast(char I_Ch, SizeType I_Pos = StringType::npos) const noexcept
        {
            return String.rfind(I_Ch, I_Pos);
        }

        [[nodiscard]] SizeType FindLast(const char* I_Str, SizeType I_Pos = StringType::npos) const noexcept
        {
            return String.rfind(I_Str ? I_Str : "", I_Pos);
        }

        [[nodiscard]] FString Substr(SizeType I_Pos = 0, SizeType I_Count = StringType::npos) const
        {
            return FString(String.substr(I_Pos, I_Count));
        }

        [[nodiscard]] FString SubString(SizeType I_Pos = 0, SizeType I_Count = StringType::npos) const
        {
            return Substr(I_Pos, I_Count);
        }

        // --- std::ranges support: Split by delimiter ---

        /**
         * Split this string by delimiter and return parts as TArray<FString>.
         * Uses Algorithm::Split. Empty segments are included (e.g. "a,,b" -> ["a","","b"]).
         */
        [[nodiscard]] TArray<FString> Split(char I_Delimiter) const
        {
            TArray<FString> Result;
            auto Split = Algorithm::Split(String, I_Delimiter);
            for (auto SubRange : Split)
            {
                Result.PushBack(FString(SubRange.begin(), SubRange.end()));
            }
            return Result;
        }

        /**
         * Split this string by multi-character delimiter and return parts as TArray<FString>.
         */
        [[nodiscard]] TArray<FString> Split(FStringView I_Delimiter) const
        {
            TArray<FString> Result;
            std::string_view Delim = I_Delimiter.GetNative();
            if (Delim.empty())
            {
                Result.PushBack(*this);
                return Result;
            }
            auto Split = Algorithm::Split(String, Delim);
            for (auto SubRange : Split)
            {
                Result.PushBack(FString(SubRange.begin(), SubRange.end()));
            }
            return Result;
        }

        /**
         * Split this string by delimiter and return a lazy range of subranges (each is a range of char).
         * Use with std::ranges or for (auto part : s.Split(',')) then construct FString(part.begin(), part.end()).
         */
        [[nodiscard]] auto Subranges(char I_Delimiter) const
        {
            return Algorithm::Split(String, I_Delimiter);
        }

        [[nodiscard]] auto Subranges(FStringView I_Delimiter) const
        {
            return Algorithm::Split(String, I_Delimiter.GetNative());
        }

        [[nodiscard]] TArray<FStringView>
        SplitToViews(char I_Delimiter) const &
        {
            TArray<FStringView> Result; Result.Reserve(4);

            const std::string_view S = String;

            if (S.empty())
            {
                Result.PushBack(FStringView("")); // [""]
                return Result;
            }

            SizeType Pos = 0;

            while (True)
            {
                const SizeType Found = static_cast<SizeType>(S.find(I_Delimiter, Pos));

                if (Found == std::string_view::npos)
                {
                    Result.PushBack(FStringView(S.data() + Pos, S.size() - Pos));
                    break;
                }

                Result.PushBack(FStringView(S.data() + Pos, Found - Pos));
                Pos = Found + 1; // char delimiter
            }

            return Result;
        }
        [[nodiscard]] TArray<FStringView>
        SplitToViews(char I_Delimiter) const && = delete;

        [[nodiscard]] TArray<FStringView>
        SplitToViews(FStringView I_Delimiter) const &
        {
            TArray<FStringView> Result; Result.Reserve(4);

            const std::string_view S = String;
            const std::string_view D = I_Delimiter.GetNative();

            if (D.empty())
            {
                Result.PushBack(FStringView(S));
                return Result;
            }

            if (S.empty())
            {
                Result.PushBack(FStringView("")); // [""]
                return Result;
            }

            SizeType Pos = 0;

            while (True)
            {
                const SizeType Found = S.find(D, Pos);

                if (Found == std::string_view::npos)
                {
                    Result.PushBack(FStringView(S.data() + Pos, S.size() - Pos));
                    break;
                }

                Result.PushBack(FStringView(S.data() + Pos, Found - Pos));
                Pos = Found + D.size();
            }

            return Result;
        }
        [[nodiscard]] TArray<FStringView>
        SplitToViews(FStringView I_Delimiter) const && = delete;

        VISERA_CORE_API
        friend Bool operator==(const FString& I_Lhs, FIntrusiveUnsetOptionalState) noexcept;
    };

    inline Bool operator==(const FString& I_Lhs, FIntrusiveUnsetOptionalState) noexcept
    { return I_Lhs.IsEmpty(); }
    static_assert(sizeof(TOptional<FString>) == sizeof(FString));
    static_assert(sizeof(TOptional<FStringView>) == sizeof(FStringView));

    inline FStringView::FStringView(const FString& I_Str)
        : View(I_Str.GetNative())
    {
    }

    // Non-member operators
    [[nodiscard]] inline FString operator+(const FString& I_Lhs, const FString& I_Rhs)
    {
        FString Result(I_Lhs);
        Result += I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FString operator+(const FString& I_Lhs, const char* I_Rhs)
    {
        FString Result(I_Lhs);
        Result += (I_Rhs ? I_Rhs : "");
        return Result;
    }

    [[nodiscard]] inline FString operator+(const char* I_Lhs, const FString& I_Rhs)
    {
        FString Result(I_Lhs ? I_Lhs : "");
        Result += I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FString operator+(const FString& I_Lhs, char I_Rhs)
    {
        FString Result(I_Lhs);
        Result += I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FString operator+(char I_Lhs, const FString& I_Rhs)
    {
        FString Result(1, I_Lhs);
        Result += I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FString operator+(const FString& I_Lhs, FStringView I_Rhs)
    {
        FString Result(I_Lhs);
        Result += I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FString operator+(FStringView I_Lhs, const FString& I_Rhs)
    {
        FString Result(I_Lhs.GetNative());
        Result += I_Rhs;
        return Result;
    }

    [[nodiscard]] inline Bool operator==(const FString& I_Lhs, const FString& I_Rhs) noexcept
    {
        return I_Lhs.GetNative() == I_Rhs.GetNative();
    }

    [[nodiscard]] inline Bool operator==(const FString& I_Lhs, const std::string& I_Rhs) noexcept
    {
        return I_Lhs.GetNative() == I_Rhs;
    }

    [[nodiscard]] inline Bool operator==(const std::string& I_Lhs, const FString& I_Rhs) noexcept
    {
        return I_Lhs == I_Rhs.GetNative();
    }

    [[nodiscard]] inline Bool operator==(const FString& I_Lhs, const char* I_Rhs)
    {
        return std::string_view(I_Lhs.GetNative()) == (I_Rhs ? std::string_view(I_Rhs) : std::string_view());
    }

    [[nodiscard]] inline Bool operator==(const char* I_Lhs, const FString& I_Rhs)
    {
        return (I_Lhs ? std::string_view(I_Lhs) : std::string_view()) == std::string_view(I_Rhs.GetNative());
    }

    [[nodiscard]] inline Bool operator==(const FString& I_Lhs, FStringView I_Rhs) noexcept
    {
        return std::string_view(I_Lhs.GetNative()) == I_Rhs.GetNative();
    }

    [[nodiscard]] inline Bool operator==(FStringView I_Lhs, const FString& I_Rhs) noexcept
    {
        return I_Lhs.GetNative() == std::string_view(I_Rhs.GetNative());
    }

    [[nodiscard]] inline Bool operator!=(const FString& I_Lhs, const FString& I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator!=(const FString& I_Lhs, const std::string& I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator!=(const std::string& I_Lhs, const FString& I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator!=(const FString& I_Lhs, const char* I_Rhs)
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator!=(const char* I_Lhs, const FString& I_Rhs)
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator!=(const FString& I_Lhs, FStringView I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator!=(FStringView I_Lhs, const FString& I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator==(FStringView I_Lhs, FStringView I_Rhs) noexcept
    {
        return I_Lhs.GetNative() == I_Rhs.GetNative();
    }

    [[nodiscard]] inline Bool operator==(FStringView I_Lhs, const char* I_Rhs) noexcept
    {
        return I_Lhs.GetNative() == (I_Rhs ? std::string_view(I_Rhs) : std::string_view());
    }

    [[nodiscard]] inline Bool operator==(const char* I_Lhs, FStringView I_Rhs) noexcept
    {
        return (I_Lhs ? std::string_view(I_Lhs) : std::string_view()) == I_Rhs.GetNative();
    }

    [[nodiscard]] inline Bool operator!=(FStringView I_Lhs, FStringView I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator!=(FStringView I_Lhs, const char* I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator!=(const char* I_Lhs, FStringView I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    // Ordering: FString
    [[nodiscard]] inline Bool operator<(const FString& I_Lhs, const FString& I_Rhs) noexcept
    {
        return I_Lhs.GetNative() < I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator<(const FString& I_Lhs, const std::string& I_Rhs) noexcept
    {
        return I_Lhs.GetNative() < I_Rhs;
    }
    [[nodiscard]] inline Bool operator<(const std::string& I_Lhs, const FString& I_Rhs) noexcept
    {
        return I_Lhs < I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator<(const FString& I_Lhs, const char* I_Rhs)
    {
        return std::string_view(I_Lhs.GetNative()) < (I_Rhs ? std::string_view(I_Rhs) : std::string_view());
    }
    [[nodiscard]] inline Bool operator<(const char* I_Lhs, const FString& I_Rhs)
    {
        return (I_Lhs ? std::string_view(I_Lhs) : std::string_view()) < std::string_view(I_Rhs.GetNative());
    }
    [[nodiscard]] inline Bool operator<(const FString& I_Lhs, FStringView I_Rhs) noexcept
    {
        return std::string_view(I_Lhs.GetNative()) < I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator<(FStringView I_Lhs, const FString& I_Rhs) noexcept
    {
        return I_Lhs.GetNative() < std::string_view(I_Rhs.GetNative());
    }

    [[nodiscard]] inline Bool operator<=(const FString& I_Lhs, const FString& I_Rhs) noexcept
    {
        return I_Lhs.GetNative() <= I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator<=(const FString& I_Lhs, const std::string& I_Rhs) noexcept
    {
        return std::string_view(I_Lhs.GetNative()) <= std::string_view(I_Rhs);
    }
    [[nodiscard]] inline Bool operator<=(const std::string& I_Lhs, const FString& I_Rhs) noexcept
    {
        return std::string_view(I_Lhs) <= std::string_view(I_Rhs.GetNative());
    }
    [[nodiscard]] inline Bool operator<=(const FString& I_Lhs, const char* I_Rhs)
    {
        return std::string_view(I_Lhs.GetNative()) <= (I_Rhs ? std::string_view(I_Rhs) : std::string_view());
    }
    [[nodiscard]] inline Bool operator<=(const char* I_Lhs, const FString& I_Rhs)
    {
        return (I_Lhs ? std::string_view(I_Lhs) : std::string_view()) <= std::string_view(I_Rhs.GetNative());
    }
    [[nodiscard]] inline Bool operator<=(const FString& I_Lhs, FStringView I_Rhs) noexcept
    {
        return std::string_view(I_Lhs.GetNative()) <= I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator<=(FStringView I_Lhs, const FString& I_Rhs) noexcept
    {
        return I_Lhs.GetNative() <= std::string_view(I_Rhs.GetNative());
    }

    [[nodiscard]] inline Bool operator>(const FString& I_Lhs, const FString& I_Rhs) noexcept
    {
        return std::string_view(I_Lhs.GetNative()) > std::string_view(I_Rhs.GetNative());
    }
    [[nodiscard]] inline Bool operator>(const FString& I_Lhs, const std::string& I_Rhs) noexcept
    {
        return std::string_view(I_Lhs.GetNative()) > std::string_view(I_Rhs);
    }
    [[nodiscard]] inline Bool operator>(const std::string& I_Lhs, const FString& I_Rhs) noexcept
    {
        return std::string_view(I_Lhs) > std::string_view(I_Rhs.GetNative());
    }
    [[nodiscard]] inline Bool operator>(const FString& I_Lhs, const char* I_Rhs)
    {
        return std::string_view(I_Lhs.GetNative()) > (I_Rhs ? std::string_view(I_Rhs) : std::string_view());
    }
    [[nodiscard]] inline Bool operator>(const char* I_Lhs, const FString& I_Rhs)
    {
        return (I_Lhs ? std::string_view(I_Lhs) : std::string_view()) > std::string_view(I_Rhs.GetNative());
    }
    [[nodiscard]] inline Bool operator>(const FString& I_Lhs, FStringView I_Rhs) noexcept
    {
        return std::string_view(I_Lhs.GetNative()) > I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator>(FStringView I_Lhs, const FString& I_Rhs) noexcept
    {
        return I_Lhs.GetNative() > std::string_view(I_Rhs.GetNative());
    }

    [[nodiscard]] inline Bool operator>=(const FString& I_Lhs, const FString& I_Rhs) noexcept
    {
        return std::string_view(I_Lhs.GetNative()) >= std::string_view(I_Rhs.GetNative());
    }
    [[nodiscard]] inline Bool operator>=(const FString& I_Lhs, const std::string& I_Rhs) noexcept
    {
        return std::string_view(I_Lhs.GetNative()) >= std::string_view(I_Rhs);
    }
    [[nodiscard]] inline Bool operator>=(const std::string& I_Lhs, const FString& I_Rhs) noexcept
    {
        return std::string_view(I_Lhs) >= std::string_view(I_Rhs.GetNative());
    }
    [[nodiscard]] inline Bool operator>=(const FString& I_Lhs, const char* I_Rhs)
    {
        return std::string_view(I_Lhs.GetNative()) >= (I_Rhs ? std::string_view(I_Rhs) : std::string_view());
    }
    [[nodiscard]] inline Bool operator>=(const char* I_Lhs, const FString& I_Rhs)
    {
        return (I_Lhs ? std::string_view(I_Lhs) : std::string_view()) >= std::string_view(I_Rhs.GetNative());
    }
    [[nodiscard]] inline Bool operator>=(const FString& I_Lhs, FStringView I_Rhs) noexcept
    {
        return std::string_view(I_Lhs.GetNative()) >= I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator>=(FStringView I_Lhs, const FString& I_Rhs) noexcept
    {
        return I_Lhs.GetNative() >= std::string_view(I_Rhs.GetNative());
    }

    // Ordering: FStringView
    [[nodiscard]] inline Bool operator<(FStringView I_Lhs, FStringView I_Rhs) noexcept
    {
        return I_Lhs.GetNative() < I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator<(FStringView I_Lhs, const char* I_Rhs) noexcept
    {
        return I_Lhs.GetNative() < (I_Rhs ? std::string_view(I_Rhs) : std::string_view());
    }
    [[nodiscard]] inline Bool operator<(const char* I_Lhs, FStringView I_Rhs) noexcept
    {
        return (I_Lhs ? std::string_view(I_Lhs) : std::string_view()) < I_Rhs.GetNative();
    }

    [[nodiscard]] inline Bool operator<=(FStringView I_Lhs, FStringView I_Rhs) noexcept
    {
        return I_Lhs.GetNative() <= I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator<=(FStringView I_Lhs, const char* I_Rhs) noexcept
    {
        return I_Lhs.GetNative() <= (I_Rhs ? std::string_view(I_Rhs) : std::string_view());
    }
    [[nodiscard]] inline Bool operator<=(const char* I_Lhs, FStringView I_Rhs) noexcept
    {
        return (I_Lhs ? std::string_view(I_Lhs) : std::string_view()) <= I_Rhs.GetNative();
    }

    [[nodiscard]] inline Bool operator>(FStringView I_Lhs, FStringView I_Rhs) noexcept
    {
        return I_Lhs.GetNative() > I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator>(FStringView I_Lhs, const char* I_Rhs) noexcept
    {
        return I_Lhs.GetNative() > (I_Rhs ? std::string_view(I_Rhs) : std::string_view());
    }
    [[nodiscard]] inline Bool operator>(const char* I_Lhs, FStringView I_Rhs) noexcept
    {
        return (I_Lhs ? std::string_view(I_Lhs) : std::string_view()) > I_Rhs.GetNative();
    }

    [[nodiscard]] inline Bool operator>=(FStringView I_Lhs, FStringView I_Rhs) noexcept
    {
        return I_Lhs.GetNative() >= I_Rhs.GetNative();
    }
    [[nodiscard]] inline Bool operator>=(FStringView I_Lhs, const char* I_Rhs) noexcept
    {
        return I_Lhs.GetNative() >= (I_Rhs ? std::string_view(I_Rhs) : std::string_view());
    }
    [[nodiscard]] inline Bool operator>=(const char* I_Lhs, FStringView I_Rhs) noexcept
    {
        return (I_Lhs ? std::string_view(I_Lhs) : std::string_view()) >= I_Rhs.GetNative();
    }
}

VISERA_MAKE_HASH(Visera::FString, return std::hash<std::string>{}(I_Object.GetNative()););
VISERA_MAKE_FORMATTER(Visera::FString, {}, "{}", I_Formatee.GetNative());

VISERA_MAKE_HASH(Visera::FStringView, return std::hash<std::string_view>{}(I_Object.GetNative()););
VISERA_MAKE_FORMATTER(Visera::FStringView, {}, "{}", I_Formatee.GetNative());