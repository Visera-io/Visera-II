module;
#include <Visera-Global.hpp>
export module Visera.Global.Name.NamePool;
#define VISERA_MODULE_NAME "Global.Name"
import :Common;
import :NameTokenTable;
import :NameEntryTable;

import Visera.Core.Math.Hash.CityHash;
import Visera.Core.Types.Map;
import Visera.Global.Log;

export namespace Visera
{
    class VISERA_GLOBAL_API FNamePool
    {
        enum
        {
            MaxNameDigitCount   = 10,  //!!! Has Sub-Effects !!!
            MaxNameNumber       = INT32_MAX,
        };
    public:
        static inline auto
        GetInstance() -> FNamePool& { static FNamePool Instance{}; return Instance; }

        [[nodiscard]] TPair<UInt32, UInt32> inline //[Handle, Number]
        Register(FString&    I_Name);
        [[nodiscard]] TPair<UInt32, UInt32> inline //[Handle, Number]
        Register(FStringView I_Name);
        [[nodiscard]] TPair<UInt32, UInt32> inline //[Handle, Number]
        NativeRegister(const char* I_PureLowerCaseName, UInt32 I_Number = 0);
        [[nodiscard]] FStringView inline
        FetchNameString(UInt32 I_NameHandle /*NameEntryHandle*/) const;

    private:
        FNamePool();
        ~FNamePool() = default;

    private:
        FNameEntryTable NameEntryTable;
        FNameTokenTable NameTokenTable{ &NameEntryTable };

        //[Number(<0 means invalid), NameLength]
        auto ParseName(const char* I_Name, UInt32 I_Length) const -> TTuple<Int32, UInt32>;
    };

    FNamePool::
    FNamePool()
    {
        LOG_DEBUG("Creating a new NamePool on current thread.");
        // Pre-Register EPreservedNames (Do NOT use FString Literal here -- Read-Only Segment Fault!)
        { auto [Handle,_] = NativeRegister("none"); VISERA_ASSERT(Handle == 0); }
    }


    TPair<UInt32, UInt32> FNamePool::
    NativeRegister(const char* I_PureLowerCaseName, UInt32 I_Number)
    {
        VISERA_ASSERT(I_PureLowerCaseName);
        FNameHash        NameHash{ I_PureLowerCaseName };
        FNameEntryHandle NameEntryHandle = NameTokenTable.Insert(I_PureLowerCaseName, NameHash);
        return { NameEntryHandle.Value, I_Number };
    }

    TPair<UInt32, UInt32> FNamePool::
    Register(FString& I_Name)
    {
        auto [Number, NameLength] = ParseName(I_Name.data(), I_Name.size());
        VISERA_ASSERT(Number >= 0);
        FStringView PureName{ I_Name.data(), NameLength};
        return NativeRegister(PureName.data(), Number);
    }

    TPair<UInt32, UInt32> FNamePool::
    Register(FStringView I_Name)
    {
        FString CopiedName{I_Name};
        return Register(CopiedName);
    }

    FStringView FNamePool::
    FetchNameString(UInt32 I_NameHandle /*NameEntryHandle*/) const
    {
        FNameEntryHandle NameEntryHandle;
        NameEntryHandle.Value = I_NameHandle;
        VISERA_ASSERT(NameEntryHandle.GetSectionIndex()  < FNameEntryTable::MaxSections &&
                      NameEntryHandle.GetSectionOffset() < FNameEntryTable::MaxSectionOffset);
        auto& R = NameEntryTable.LookUp(NameEntryHandle);

        return NameEntryTable.LookUp(NameEntryHandle).GetANSIName();
    }

    TTuple<Int32, UInt32> FNamePool::
    ParseName(const char* I_Name, UInt32 I_Length) const
    {
        constexpr auto IsDigit = [](char _Char)->bool { return '0' <= _Char && _Char <= '9'; };

        UInt32 Digits = 0;
        char* It = const_cast<char*>(I_Name) + I_Length - 1;
        //Count Digits
        while(It >= I_Name && IsDigit(*It)) { ++Digits; --It; }
        //Convert Case
        while (It >= I_Name) { *It = std::tolower(static_cast<unsigned char>(*It)); --It; }

        if (Digits)
        {
            UInt32 FirstDigitCursor = I_Length - Digits;

            if (/*Valid Digit Length*/      (Digits < I_Length) &&
                /*Valid Naming Convention*/ (I_Name[FirstDigitCursor - 1] == '_') &&
                /*Valid Digit Count*/       (Digits <= MaxNameDigitCount))
            {
                if (/*Name_0 is Valid*/         Digits == 1 ||
                    /*Zero Prefix is InValid*/  I_Name[FirstDigitCursor] != '0')
                {
                    Int32 Number = atoi(I_Name + FirstDigitCursor);
                    if (Number < MaxNameNumber)
                    { return { Number + 1, I_Length - Digits - 1 }; }
                }
            }
            return { -1, I_Length }; //Invalid Name
        }
        else return { 0, I_Length }; //No Numbers
    }

} 