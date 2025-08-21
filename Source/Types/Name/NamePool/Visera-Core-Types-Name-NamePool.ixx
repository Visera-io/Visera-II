module;
#include <Visera-Core.hpp>
export module Visera.Core.Name.NamePool;
#define VISERA_MODULE_NAME "Core.Types"
import :Common;
import :NameTokenTable;
import :NameEntryTable;

import Visera.Core.Hash;
import Visera.Core.Log;

export namespace Visera
{
    enum class EPreservedName : UInt32
    {
        None = 0, //MUST be registered at first for assuring FNameEntryID == 0 && Number == 0
        Main,     // For Shaders' entry point
    };

    class FNamePool
    {
        enum
        {
            MaxNameDigitCount   = 10,  //Sub-Effects:...
            MaxNameNumber       = INT32_MAX,
        };
    public:
        static inline auto
        GetInstance() -> FNamePool& { static FNamePool Instance{}; return Instance; }

        auto Register(FString&    I_Name)  -> TTuple<UInt32, UInt32>; //[Handle_, Number_]
        auto Register(FStringView I_Name)  -> TTuple<UInt32, UInt32>; //[Handle_, Number_]
        auto Register(EPreservedName I_PreservedName) -> TTuple<UInt32, UInt32> { return { PreservedNameTable[I_PreservedName], 0 }; }
        auto FetchNameString(UInt32 I_NameHandle /*NameEntryHandle*/) const->FStringView;
        auto FetchNameString(EPreservedName I_PreservedName) const -> FStringView;

    private:
        FNamePool();
        ~FNamePool() = default;

    private:
        FNameEntryTable NameEntryTable;
        FNameTokenTable NameTokenTable{ &NameEntryTable };
        TMap<EPreservedName, UInt32> PreservedNameTable;

        //[Number(<0 means invalid), NameLength]
        auto ParseName(const char* _Name, UInt32 _Length) const -> TTuple<Int32, UInt32>;
    };

    FNamePool::
    FNamePool()
    {
        // Pre-Register EPreservedNames (Do NOT use FString Literal here -- Read-Only Segment Fault!)
        /*None*/ { auto [Handle_, _] = Register("none"); PreservedNameTable[EPreservedName::None] = Handle_; VISERA_ASSERT(Handle_ == 0); }
        /*Main*/ { auto [Handle_, _] = Register("main"); PreservedNameTable[EPreservedName::Main] = Handle_; }
    }

    TTuple<UInt32, UInt32> FNamePool::
    Register(FString& I_Name)
    {
        auto [Number, NameLength] = ParseName(I_Name.data(), I_Name.size());
        if (Number < 0)
        {
            LOG_ERROR("Bad Name ({})! -- Naming Convention:([#Name][_#Number]?).", I_Name);
            return {0, 0};
        }

        FStringView PureName{ I_Name.data(), NameLength};
        FNameHash  NameHash{ PureName };

        FNameEntryHandle NameEntryHandle = NameTokenTable.Insert(PureName, NameHash);

        return { NameEntryHandle.Value, Number };
    }

    TTuple<UInt32, UInt32> FNamePool::
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
    ParseName(const char* _Name, UInt32 _Length) const
    {
        constexpr auto IsDigit = [](char _Char)->bool { return '0' <= _Char && _Char <= '9'; };

        UInt32 Digits = 0;
        char* It = const_cast<char*>(_Name) + _Length - 1;
        //Count Digits
        while(It >= _Name && IsDigit(*It)) { ++Digits; --It; }
        //Convert Case
        while (It >= _Name) { *It = std::tolower(static_cast<unsigned char>(*It)); --It; }

        if (Digits)
        {
            UInt32 FirstDigitCursor = _Length - Digits;

            if (/*Valid Digit Length*/      (Digits < _Length) &&
                /*Valid Naming Convention*/ (_Name[FirstDigitCursor - 1] == '_') &&
                /*Valid Digit Count*/       (Digits <= MaxNameDigitCount))
            {
                if (/*Name_0 is Valid*/         Digits == 1 ||
                    /*Zero Prefix is InValid*/  _Name[FirstDigitCursor] != '0')
                {
                    Int32 Number = atoi(_Name + FirstDigitCursor);
                    if (Number < MaxNameNumber)
                    { return { Number + 1, _Length - Digits - 1 }; }
                }
            }
            return { -1, _Length }; //Invalid Name
        }
        else return { 0, _Length }; //No Numbers
    }

    FStringView FNamePool::
    FetchNameString(EPreservedName II_PreservedName) const
    {
        switch (II_PreservedName)
        {
        case EPreservedName::None:
            return "none";
        case EPreservedName::Main:
            return "main";
        default:
            LOG_FATAL("Unexpected EPreservedName!");
        }
        return "";
    }

} 