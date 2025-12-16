module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Name.NamePool:NameTokenTable;
#define VISERA_MODULE_NAME "Runtime.Name"
import :Common;
import :NameEntryTable;

#if defined(VISERA_DEBUG_MODE)
import Visera.Core.Math.Arithmetic;
#endif
import Visera.Core.OS.Memory;
import Visera.Core.OS.Thread.RWLock;
import Visera.Core.Math.Bit;

export namespace Visera
{
	class VISERA_RUNTIME_API FNameTokenTable
	{
	public:
        enum
        {
            MaxNameTokenTableSectionsBits    = 8,
            MaxNameTokenTableSections        = (1U << MaxNameTokenTableSectionsBits),
            MaxNameTokenTableSectionsMask    = MaxNameTokenTableSections - 1,
        };
		auto Insert(FStringView I_ParsedName, FNameHash I_NameHash) -> FNameEntryHandle;

        FNameTokenTable() = delete;
        FNameTokenTable(FNameEntryTable* _NameEntryTable) : LinkedNameEntryTable{ _NameEntryTable } {}

	private: 
		struct VISERA_RUNTIME_API FNameTokenTableSection
		{
            enum
            {
                InitNameTokens    = 1 << 8,
                InitSectionSize   = InitNameTokens * sizeof(FNameToken), // Unaligned
            };
            static constexpr float GrowingThresh = 0.9;

			auto ProbeToken(FNameHash I_NameHash, std::function<Bool(FNameToken)> I_Prediction) const -> const FNameToken&;
			auto ClaimToken(const FNameToken* I_Token, FNameToken I_Value) { 
                VISERA_ASSERT(!RWLock.TryToWrite() && !I_Token->IsClaimed());
                const_cast<FNameToken*>(I_Token)->HashAndID = I_Value.HashAndID; 
                ++UseCount; 
            };
			Bool ShouldGrow() const { return UseCount > Capacity * GrowingThresh; }

            FNameTokenTableSection();
            ~FNameTokenTableSection();

            mutable FRWLock RWLock;
			FNameToken*     Tokens;
			UInt32          Capacity = 0;
			UInt32          UseCount = 0;
		};

        FNameEntryTable* const LinkedNameEntryTable;
	    FNameTokenTableSection Sections[MaxNameTokenTableSections];
        void GrowAndRehash(FNameTokenTableSection& I_Section);
	};

    FNameTokenTable::FNameTokenTableSection::
    FNameTokenTableSection()
    {
#if defined(VISERA_ON_WINDOWS_SYSTEM)
        constexpr UInt32 TokenAlignment = alignof(FNameToken);
#elif defined(VISERA_ON_APPLE_SYSTEM)
        constexpr UInt32 TokenAlignment = ((alignof(FNameToken) / sizeof(void*)) + 1) * sizeof(void*);
#else
        VISERA_WIP;
        static_assert(false);
#endif

        Tokens   = (FNameToken*)Memory::MallocNow(InitSectionSize, TokenAlignment);
        VISERA_ASSERT(Tokens);

        Capacity = InitNameTokens;
        UseCount = 0;
    }

    FNameTokenTable::FNameTokenTableSection::
    ~FNameTokenTableSection()
    {
        Memory::Free(Tokens, alignof(FNameToken));
        Capacity = 0;
        UseCount = 0;
    }

    FNameEntryHandle FNameTokenTable::
    Insert(FStringView I_ParsedName, FNameHash I_NameHash)
    {
        auto& Section = Sections[I_NameHash.GetTokenTableSectionIndex()];
        FNameEntryHandle NewNameEntryHandle{};

        Section.RWLock.StartWriting();
        {
            if (Section.ShouldGrow()) { GrowAndRehash(Section); }

            auto& Token = Section.ProbeToken(I_NameHash,
                /*Further Comparsion*/
                [&](FNameToken I_Token)->Bool
                { 
                    VISERA_ASSERT(I_Token.IsClaimed());
                    FNameEntryHandle NameEntryHandle{I_Token};
                    auto& NameEntry = LinkedNameEntryTable->LookUp(NameEntryHandle);
                    if (NameEntry.GetHeader().LowerCaseProbeHash
                        ==
                        I_NameHash.GetLowerCaseProbeHash())
                    {
                        if (NameEntry.GetANSIName() == I_ParsedName)
                        {
                            return True; // [O]
                        }
                        return False; // [X]: Different String Literal
                    }
                    return False; // [X]: Different LowerCase Probe Hash
                });
            
            if (!Token.IsClaimed())
            {
                NewNameEntryHandle = LinkedNameEntryTable->Insert(I_ParsedName, I_NameHash);
                Section.ClaimToken(&Token, FNameToken{ NewNameEntryHandle, I_NameHash });
            }
            else
            {
                NewNameEntryHandle = FNameEntryHandle{ Token };
            }
        }
        Section.RWLock.StopWriting();

        return NewNameEntryHandle;
    }

	void FNameTokenTable::
    GrowAndRehash(FNameTokenTableSection& I_Section)
    {
        VISERA_ASSERT(!I_Section.RWLock.TryToWrite());

        auto* OldTokens     = I_Section.Tokens;
        auto  OldCapacity   = I_Section.Capacity;

        // Reallocating
        I_Section.Capacity  <<= 1; // Double
        I_Section.UseCount  = 0;
        I_Section.Tokens    = (FNameToken*)Memory::MallocNow(I_Section.Capacity * sizeof(FNameToken), alignof(FNameToken));
        VISERA_ASSERT(I_Section.Tokens);

        // Rehashing
        for (UInt32 It = 0; It < OldCapacity; ++It)
        {   
            FNameToken& CurrentOldToken = *(OldTokens + It);
            if (CurrentOldToken.IsClaimed())
            {
                auto& NameEntry = LinkedNameEntryTable->LookUp(FNameEntryHandle{ CurrentOldToken });
                FNameHash NameHash{ NameEntry.GetANSIName() };

                auto& Token = I_Section.ProbeToken(NameHash, [](FNameToken I_Token) -> Bool { return False; });
                VISERA_ASSERT(!Token.IsClaimed() && "Any Token in the old section is UNIQUE!");
                I_Section.ClaimToken(&Token, CurrentOldToken);
            }
        }
        Memory::Free(OldTokens, alignof(FNameToken));
    }

    const FNameToken& FNameTokenTable::FNameTokenTableSection::
    ProbeToken(FNameHash I_NameHash, std::function<Bool(FNameToken)> I_Prediction) const
    {
        VISERA_ASSERT(UseCount <= Capacity && "Curreny FNamePoolShard is full! Try to call FNamePool::GrowAndRehash(...)");
        VISERA_ASSERT(Math::IsPowerOfTwo(Capacity));
         
        UInt32 TokenIndexMask = Capacity - 1;
        for(UInt32 Offset = 0; True; Offset++)
        {
            auto& Token = Tokens[(I_NameHash.GetUnmaskedTokenIndex() + Offset) & TokenIndexMask];
            if (!Token.IsClaimed() || I_Prediction(Token))
            {
                return Token; // Call FNameTokenTable::ClaimToken(...) if it should be used.
            }
        }
        VISERA_ASSERT(False && "Absolutely return a Token!");
    }

} 