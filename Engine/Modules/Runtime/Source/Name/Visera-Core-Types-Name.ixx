module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Name;
#define VISERA_MODULE_NAME "Runtime.Name"
import Visera.Runtime.Name.NamePool;

export namespace Visera
{
    using EName = EPreservedName;

    /* Case-Ignored FString */
    class VISERA_RUNTIME_API FName
    {
    public:
        static inline auto
        FetchNameString(const FName& I_Name) -> FStringView { return FNamePool::GetInstance().FetchNameString(I_Name.Handle); }

        auto GetName()	 const -> FStringView { return FNamePool::GetInstance().FetchNameString(Handle); }
        auto GetHandle() const -> UInt32 { return Handle; }
        auto GetNumber() const -> UInt32 { return Number; }
        auto GetIdentifier() const -> UInt64 { return (UInt64(Handle) << 32) | Number; }
        auto IsNone()	 const -> Bool	 { return !Handle && !Number; } //[FIXME]: Pre-Register EName::None in the Engine
        auto HasNumber() const -> Bool	 { return !!Number; }

        FName() = default;
        FName(FStringView I_Name)					{ auto [Handle_, Number_] = FNamePool::GetInstance().Register(FString(I_Name)); Handle = Handle_; Number = Number_;   }
        FName(EName I_PreservedName)				{ auto [Handle_, Number_] = FNamePool::GetInstance().Register(I_PreservedName); Handle = Handle_; Number = Number_; }
        FName(const FName& I_Another)			    = default;
        FName(FName&& I_Another)					= default;
        FName& operator=(FStringView I_Name)		{ auto [Handle_, Number_] = FNamePool::GetInstance().Register(FString(I_Name)); Handle = Handle_; Number = Number_;  return *this; }
        FName& operator=(const FName& I_Another)    = default;
        FName& operator=(FName&& I_Another)		    = default;
        Bool operator==(const FName& I_Another) const { return GetIdentifier() == I_Another.GetIdentifier(); }
    private:
        UInt32		Handle;		//FNameEntryHandle
        UInt32		Number{ 0 };
    };

}
VISERA_MAKE_HASH(Visera::FName,
                 return I_Object.GetIdentifier();
)
VISERA_MAKE_FORMATTER(Visera::FName, {},
                      "{}({})",
                      I_Formatee.GetName(), I_Formatee.GetNumber()
)