module;
#include <Visera-Platform.hpp>
#include <windows.h>
export module Visera.Platform.Windows.Path;
#define VISERA_MODULE_NAME "Platform.Windows"
import Visera.Platform.Interface.Path;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;
import Visera.Core.Types.Pointer.Unique;

namespace Visera
{
    export class VISERA_PLATFORM_API FWindowsPath : public IPlatformPath
    {
    public:
        using FPathChar = wchar_t;
        explicit FWindowsPath(const FPath& I_Path);
        explicit FWindowsPath(std::wstring_view I_Native) : Native(I_Native) {}
        [[nodiscard]] operator std::wstring_view() const noexcept { return Native; }
        [[nodiscard]] const FPathChar* GetPathString() const noexcept { return Native.c_str(); }
        [[nodiscard]] Bool IsEmpty() const override { return Native.empty(); }
        [[nodiscard]] TUniquePtr<IPlatformPath> GetParent() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath> GetFileName() const override;
        [[nodiscard]] FPath ToPath() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath> Clone() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath> Concat(const IPlatformPath& I_Other) const override;
        [[nodiscard]] TUniquePtr<IPlatformPath> Append(FStringView I_Suffix) const override;

    private:
        std::wstring Native;
    };

    FWindowsPath::FWindowsPath(const FPath& I_Path)
    {
        VISERA_ASSERT(I_Path.IsNormalized());
        const FStringView Utf8(I_Path.GetString().GetNative());
        if (Utf8.IsEmpty()) return;
        const int WideLength = MultiByteToWideChar(CP_UTF8, 0, Utf8.Data(), static_cast<int>(Utf8.GetSize()), nullptr, 0);
        if (WideLength <= 0) return;
        Native.resize(static_cast<size_t>(WideLength));
        MultiByteToWideChar(CP_UTF8, 0, Utf8.Data(), static_cast<int>(Utf8.GetSize()), Native.data(), WideLength);
        for (wchar_t& Ch : Native)
            if (Ch == L'/') Ch = L'\\';
    }

    FPath FWindowsPath::ToPath() const
    {
        const std::wstring_view Wide = *this;
        if (Wide.empty()) return FPath(FString());
        const int Utf8Length = WideCharToMultiByte(CP_UTF8, 0, Wide.data(), static_cast<int>(Wide.size()), nullptr, 0, nullptr, nullptr);
        if (Utf8Length <= 0) return FPath(FString());
        FString Utf8(static_cast<FString::SizeType>(Utf8Length), '\0');
        WideCharToMultiByte(CP_UTF8, 0, Wide.data(), static_cast<int>(Wide.size()), Utf8.Data(), Utf8Length, nullptr, nullptr);
        for (char& Ch : Utf8)
            if (Ch == '\\') Ch = '/';
        FPath Path(std::move(Utf8));
        Path.Normalize();
        return Path;
    }

    TUniquePtr<IPlatformPath> FWindowsPath::GetParent() const
    {
        const size_t Pos = Native.find_last_of(L"\\/");
        if (Pos == std::wstring::npos || Pos == 0) return nullptr;
        return MakeUnique<FWindowsPath>(Native.substr(0, Pos));
    }

    TUniquePtr<IPlatformPath> FWindowsPath::GetFileName() const
    {
        const size_t Pos = Native.find_last_of(L"\\/");
        if (Pos != std::wstring::npos && Pos + 1 < Native.size())
            return MakeUnique<FWindowsPath>(Native.substr(Pos + 1));
        return Clone();
    }

    TUniquePtr<IPlatformPath> FWindowsPath::Clone() const
    {
        return MakeUnique<FWindowsPath>(static_cast<std::wstring_view>(*this));
    }

    TUniquePtr<IPlatformPath> FWindowsPath::Concat(const IPlatformPath& I_Other) const
    {
        const FPath Merged = ToPath() / I_Other.ToPath();
        return MakeUnique<FWindowsPath>(Merged);
    }

    TUniquePtr<IPlatformPath>     FWindowsPath::Append(FStringView I_Suffix) const
    {
        if (I_Suffix.IsEmpty()) return Clone();
        const int WideLen = MultiByteToWideChar(CP_UTF8, 0, I_Suffix.Data(), static_cast<int>(I_Suffix.GetSize()), nullptr, 0);
        if (WideLen <= 0) return Clone();
        std::wstring SuffixW(static_cast<size_t>(WideLen), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, I_Suffix.Data(), static_cast<int>(I_Suffix.GetSize()), SuffixW.data(), WideLen);
        std::wstring Result = Native;
        if (!Result.empty() && Result.back() != L'\\' && Result.back() != L'/')
            ; // no separator for direct append
        Result.append(SuffixW);
        return MakeUnique<FWindowsPath>(Result);
    }
}
