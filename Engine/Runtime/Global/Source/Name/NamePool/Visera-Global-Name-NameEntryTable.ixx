module;
#include <Visera-Global.hpp>
export module Visera.Global.Name.NamePool:NameEntryTable;
#define VISERA_MODULE_NAME "Global.Name"
import :Common;

import Visera.Core.OS.Memory;
import Visera.Core.OS.Thread.RWLock;

export namespace Visera
{
	class VISERA_GLOBAL_API FNameEntryTable
	{
	public:
		enum
		{
			SectionByteSize		= 256 * 1024 /*KByte*/,			//Unaligned Size

			MaxSectionBits		= 13,
			MaxSections			= (1U << MaxSectionBits),		// 8192 Sections
#if defined(VISERA_ON_WINDOWS_SYSTEM)
			NameEntryAlignment	= alignof(FNameEntry),
#elif defined(VISERA_ON_APPLE_SYSTEM)
			NameEntryAlignment	= (alignof(FNameEntry) / sizeof(void*) + 1) * sizeof(void*),
#endif
			NameEntryByteStride = NameEntryAlignment,

			SectionOffsetBits	= 16, // Since NameEntryAlignment and NameEntryByteStride are same (2), we can use 16bits to encode 65536 offsets (remeber these factor!)
			MaxSectionOffset	= (1U << SectionOffsetBits), //65536 Offsets (Max)
			NameEntryHandleBits	= MaxSectionBits + SectionOffsetBits,
			NameEntryHandleMask	= (1U << NameEntryHandleBits) - 1,
		};

		FNameEntryHandle
		Insert(FStringView I_ParsedName, const FNameHash& I_NameHash);
		inline const FNameEntry&
		LookUp(FNameEntryHandle I_NameEntryHandle) const
		{
			// Reset not needed since any valid NameEntryHandle is distributed after the insertion.
			return *reinterpret_cast<FNameEntry*>(
				Sections[I_NameEntryHandle.GetSectionIndex()].Data
				+ (NameEntryByteStride * I_NameEntryHandle.GetSectionOffset()));
		}

	private:
		struct FNameEntryTableSection
		{
			FByte*	Data; // Entries
			UInt32	CurrentByteCursor = 0;  //Point at the current entry.
		};
		mutable FRWLock RWLock;
		Int32 CurrentSectionCursor = -1;
		FNameEntryTableSection Sections[MaxSections];

		void AllocateNewSection();

	public:
		FNameEntryTable();
		~FNameEntryTable();
	};

	FNameEntryHandle FNameEntryTable::
	Insert(FStringView I_ParsedName, const FNameHash& I_NameHash)
	{
		UInt32 AlignedMemorySize = Memory::Align(Memory::GetDataOffset(&FNameEntry::ANSIName) + I_ParsedName.size(), NameEntryAlignment);
		VISERA_ASSERT(AlignedMemorySize <= SectionByteSize);

		FNameEntryHandle NameEntryHandle;

		RWLock.StartWriting();
		{
			if (Sections[CurrentSectionCursor].CurrentByteCursor + AlignedMemorySize >= SectionByteSize)
			{ AllocateNewSection(); }

			VISERA_ASSERT(CurrentSectionCursor % NameEntryByteStride == 0 &&
					      CurrentSectionCursor / NameEntryByteStride < MaxSectionOffset);

			auto& CurrentSection = Sections[CurrentSectionCursor];
			NameEntryHandle = FNameEntryHandle{ static_cast<UInt32>(CurrentSectionCursor),
												CurrentSection.CurrentByteCursor / NameEntryByteStride };

			CurrentSection.CurrentByteCursor += AlignedMemorySize;
		}
		RWLock.StopWriting();

		auto& NewNameEntry  = const_cast<FNameEntry&>(LookUp(NameEntryHandle));
		NewNameEntry.Header.LowerCaseProbeHash = I_NameHash.GetLowerCaseProbeHash();
		NewNameEntry.Header.Size			   = I_ParsedName.size();
		Memory::Memcpy(NewNameEntry.ANSIName, I_ParsedName.data(), NewNameEntry.Header.Size);

		return NameEntryHandle;
	}

	void FNameEntryTable::
	AllocateNewSection()
    {
		VISERA_ASSERT(CurrentSectionCursor < MaxSections);

		if (CurrentSectionCursor >= 0)
		{
			auto& CurrentSection = Sections[CurrentSectionCursor];
			// Solve Special Case: Terminator
			UInt64 ExpectedByteSize = CurrentSection.CurrentByteCursor
				                    + Memory::GetDataOffset(&FNameEntry::ANSIName);
			if (ExpectedByteSize <= SectionByteSize)
			{
				FNameEntry* Terminator = (FNameEntry*)(
					                      Sections[CurrentSectionCursor].Data
					                   +  CurrentSection.CurrentByteCursor);
				Terminator->Header.Size = 0;
			}
		}

		auto& NewSection = Sections[++CurrentSectionCursor];
		NewSection.Data = static_cast<FByte*>(Memory::MallocNow(SectionByteSize, NameEntryAlignment));
		VISERA_ASSERT(NewSection.Data);

		NewSection.CurrentByteCursor = 0;
    }

	FNameEntryTable::
	FNameEntryTable()
	{
		AllocateNewSection();
	}

	FNameEntryTable::
	~FNameEntryTable()
	{
		for (auto& Section : Sections)
		{
			Memory::Free(Section.Data, NameEntryAlignment);
			Section.CurrentByteCursor = 0;
		}
		CurrentSectionCursor = -1;
	}
}