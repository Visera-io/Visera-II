module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Name;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Name.NamePool;
import Visera.Core.Types.String;

export namespace Visera
{
    class VISERA_CORE_API FName
    {
    public:
        [[nodiscard]] static inline auto
        FetchNameString(const FName& I_Name) -> FStringView { return FNamePool::GetInstance().FetchNameString(I_Name.Handle); }

        [[nodiscard]] FStringView inline
        GetName()       const { return FNamePool::GetInstance().FetchNameString(Handle); }
        [[nodiscard]] UInt32 inline
        GetHandle()     const { return Handle; }
        [[nodiscard]] UInt32 inline
        GetNumber()     const { return Number; }
        [[nodiscard]] UInt64 inline
        GetIdentifier() const { return (static_cast<UInt64>(Handle) << 32) | Number; }
        [[nodiscard]] Bool inline
        IsNone()        const { return !Handle && !Number; }
        [[nodiscard]] Bool inline
        HasNumber()     const { return !!Number; }

        FName() = default;
        FName(FStringView I_Name)					{ auto [Handle_, Number_] = FNamePool::GetInstance().Register(FString(I_Name)); Handle = Handle_; Number = Number_;   }
        FName(const char* I_Name, UInt32 I_Number)  { auto [Handle_, Number_] = FNamePool::GetInstance().NativeRegister(I_Name, I_Number); Handle = Handle_; Number = Number_;     }
        FName(const FName& I_Another)			    = default;
        FName(FName&& I_Another)					= default;
        FName& operator=(FStringView I_Name)		{ auto [Handle_, Number_] = FNamePool::GetInstance().Register(FString(I_Name)); Handle = Handle_; Number = Number_;  return *this; }
        FName& operator=(const FName& I_Another)    = default;
        FName& operator=(FName&& I_Another)		    = default;
        Bool   operator==(const FName& I_Another) const { return GetIdentifier() == I_Another.GetIdentifier(); }

    private:
        UInt32		Handle{0}; //FNameEntryHandle
        UInt32		Number{0};
    };

}
VISERA_MAKE_HASH(Visera::FName, return I_Object.GetIdentifier(); )
VISERA_MAKE_FORMATTER(Visera::FName, {},
                      "{}({})",
                      I_Formatee.GetName(), I_Formatee.GetNumber());