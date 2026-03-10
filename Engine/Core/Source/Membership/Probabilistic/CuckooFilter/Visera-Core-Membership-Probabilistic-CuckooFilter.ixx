module;
#include <Visera-Core.hpp>
export module Visera.Core.Membership.Probabilistic.CuckooFilter;
#define VISERA_MODULE_NAME "Core.Membership"
import Visera.Core.Containers.Array;
import Visera.Core.Math.Hash;
import Visera.Core.Limits.Numeric;
import Visera.Core.Math.Bit;
import Visera.Core.Math.Random;
import Visera.Core.Types.String;
import Visera.Core.OS.Memory;

#ifndef VISERA_ON_LITTLE_ENDIAN_PLATFORM
#error "CuckooFilter SWAR and packed-tag paths require little-endian."
#endif

/**
 * Cuckoo Filter implementation.
 * Original algorithm and reference implementation:
 *   https://github.com/efficient/cuckoofilter
 * Paper: "Cuckoo Filter: Practically Better Than Bloom" (ACM CoNEXT 2014)
 *   Bin Fan, Dave Andersen, Michael Kaminsky
 */

namespace Visera
{
	/** SWAR-style "any 4/8/12/16-bit lane equals I_Needle" in a 64-bit word (4 lanes). Little-endian.
	 *  Reference: cuckoofilter-master src/bitsutil.h hasvalue4/8/12/16 (ZeroInWord bithack). */
	[[nodiscard]] inline Bool HasValue4(UInt64 I_Word, UInt32 I_Needle) noexcept
	{
		constexpr UInt64 kPattern4 = 0x1111ULL;
		UInt64 XorValue = (I_Word ^ (kPattern4 * static_cast<UInt64>(I_Needle)));
		return ((XorValue - kPattern4) & (~XorValue) & (kPattern4 << 3)) != 0;
	}
	[[nodiscard]] inline Bool HasValue8(UInt64 I_Word, UInt32 I_Needle) noexcept
	{
		constexpr UInt64 kPattern8 = 0x01010101ULL;
		UInt64 XorValue = (I_Word ^ (kPattern8 * static_cast<UInt64>(I_Needle)));
		return ((XorValue - kPattern8) & (~XorValue) & (kPattern8 << 7)) != 0;
	}
	[[nodiscard]] inline Bool HasValue12(UInt64 I_Word, UInt32 I_Needle) noexcept
	{
		constexpr UInt64 kPattern12 = 0x001001001001ULL;
		UInt64 XorValue = (I_Word ^ (kPattern12 * static_cast<UInt64>(I_Needle)));
		return ((XorValue - kPattern12) & (~XorValue) & (kPattern12 << 11)) != 0;
	}
	[[nodiscard]] inline Bool HasValue16(UInt64 I_Word, UInt32 I_Needle) noexcept
	{
		constexpr UInt64 kPattern16 = 0x0001000100010001ULL;
		UInt64 XorValue = (I_Word ^ (kPattern16 * static_cast<UInt64>(I_Needle)));
		return ((XorValue - kPattern16) & (~XorValue) & (kPattern16 << 15)) != 0;
	}

	/** Internal table: 4 tags per bucket, packed bits. Tail padding sizeof(UInt64) for safe 8-byte read (ARM/alignment).
	 *  Reference: cuckoofilter-master src/singletable.h SingleTable. Modifications: TArray<UInt8> storage,
	 *  Memory::Memcpy for unaligned-safe read/write, explicit 12-bit mask to preserve adjacent tag. */
	template<size_t BitsPerTag>
	class SingleTable
	{
	public:
		[[nodiscard]] UInt64 GetNumBuckets() const { return BucketCount; }
		[[nodiscard]] UInt64 SizeInBytes() const { return BucketCount * kBytesPerBucket; }
		[[nodiscard]] UInt64 SizeInTags() const { return kTagsPerBucket * BucketCount; }

		[[nodiscard]] UInt32 ReadTag(UInt64 I_BucketIndex, UInt64 I_SlotIndex) const;
		void WriteTag(UInt64 I_BucketIndex, UInt64 I_SlotIndex, UInt32 I_Tag);
		[[nodiscard]] Bool FindTagInBuckets(UInt64 I_BucketIndex1, UInt64 I_BucketIndex2, UInt32 I_Tag) const;
		Bool DeleteTagFromBucket(UInt64 I_BucketIndex, UInt32 I_Tag);
		Bool InsertTagToBucket(UInt64 I_BucketIndex, UInt32 I_Tag, Bool I_Kickout, UInt32& I_OutOldTag, UInt32 I_KickoutSlot);

		explicit SingleTable(UInt64 I_NumBuckets);

	private:
		static_assert(BitsPerTag >= 2 && BitsPerTag <= 32, "BitsPerTag must be in [2, 32]");
		static constexpr size_t kTagsPerBucket = 4;
		static constexpr size_t kBytesPerBucket = (BitsPerTag * kTagsPerBucket + 7) >> 3;
		static constexpr UInt32 kTagMask = (BitsPerTag >= 32) ? 0xFFFFFFFFu : static_cast<UInt32>((1ULL << BitsPerTag) - 1);
		TArray<UInt8> Buffer;
		UInt64 BucketCount = 0;

		[[nodiscard]] const UInt8* BucketPointer(UInt64 I_BucketIndex) const { return Buffer.Data() + I_BucketIndex * kBytesPerBucket; }
		[[nodiscard]] UInt8* BucketPointer(UInt64 I_BucketIndex) { return Buffer.Data() + I_BucketIndex * kBytesPerBucket; }
	};

	template<size_t BitsPerTag>
	SingleTable<BitsPerTag>::SingleTable(UInt64 I_NumBuckets) : BucketCount(I_NumBuckets)
	{
		VISERA_ASSERT(I_NumBuckets <= (Limits::UpperBound<UInt64>() - sizeof(UInt64)) / kBytesPerBucket);
		UInt64 TotalBytes = I_NumBuckets * kBytesPerBucket + sizeof(UInt64);
		Buffer.Resize(static_cast<typename TArray<UInt8>::SizeType>(TotalBytes));
		Memory::Memset(Buffer.Data(), 0, TotalBytes);
	}

	template<size_t BitsPerTag>
	UInt32 SingleTable<BitsPerTag>::ReadTag(UInt64 I_BucketIndex, UInt64 I_SlotIndex) const
	{
		const UInt8* Pointer = BucketPointer(I_BucketIndex);
		UInt32 TagValue;
		if constexpr (BitsPerTag == 2) {
			TagValue = Pointer[0] >> (I_SlotIndex * 2);
		} else if constexpr (BitsPerTag == 4) {
			Pointer += (I_SlotIndex >> 1);
			TagValue = Pointer[0] >> ((I_SlotIndex & 1) << 2);
		} else if constexpr (BitsPerTag == 8) {
			TagValue = Pointer[I_SlotIndex];
		} else if constexpr (BitsPerTag == 12) {
			Pointer += I_SlotIndex + (I_SlotIndex >> 1);
			UInt16 Word;
			Memory::Memcpy(&Word, Pointer, sizeof(UInt16));
			TagValue = (Word >> ((I_SlotIndex & 1) << 2)) & kTagMask;
		} else if constexpr (BitsPerTag == 16) {
			UInt16 Word;
			Memory::Memcpy(&Word, Pointer + (I_SlotIndex << 1), sizeof(UInt16));
			TagValue = Word & kTagMask;
		} else if constexpr (BitsPerTag == 32) {
			Memory::Memcpy(&TagValue, Pointer + (I_SlotIndex * 4), sizeof(UInt32));
			TagValue &= kTagMask;
		} else {
			size_t BitOffset = I_SlotIndex * BitsPerTag;
			size_t ByteOffset = BitOffset / 8;
			BitOffset %= 8;
			UInt64 Value = 0;
			for (size_t Bit = 0; Bit < BitsPerTag; ++Bit) {
				if (Pointer[ByteOffset] & (1u << BitOffset)) Value |= (1ULL << Bit);
				if (++BitOffset == 8) { BitOffset = 0; ++ByteOffset; }
			}
			TagValue = static_cast<UInt32>(Value);
		}
		return TagValue & kTagMask;
	}

	template<size_t BitsPerTag>
	void SingleTable<BitsPerTag>::WriteTag(UInt64 I_BucketIndex, UInt64 I_SlotIndex, UInt32 I_Tag)
	{
		UInt8* Pointer = BucketPointer(I_BucketIndex);
		UInt32 TagValue = I_Tag & kTagMask;
		if constexpr (BitsPerTag == 2) {
			Pointer[0] = static_cast<UInt8>((Pointer[0] & ~(3u << (I_SlotIndex * 2))) | (TagValue << (I_SlotIndex * 2)));
		} else if constexpr (BitsPerTag == 4) {
			Pointer += (I_SlotIndex >> 1);
			if ((I_SlotIndex & 1) == 0) {
				Pointer[0] = static_cast<UInt8>((Pointer[0] & 0xf0u) | TagValue);
			} else {
				Pointer[0] = static_cast<UInt8>((Pointer[0] & 0x0fu) | (TagValue << 4));
			}
		} else if constexpr (BitsPerTag == 8) {
			Pointer[I_SlotIndex] = static_cast<UInt8>(TagValue);
		} else if constexpr (BitsPerTag == 12) {
			Pointer += I_SlotIndex + (I_SlotIndex >> 1);
			UInt16 Word;
			Memory::Memcpy(&Word, Pointer, sizeof(UInt16));
			if ((I_SlotIndex & 1) == 0)
				Word = static_cast<UInt16>((Word & 0xf000u) | TagValue);
			else
				Word = static_cast<UInt16>((Word & 0x000fu) | (TagValue << 4));
			Memory::Memcpy(Pointer, &Word, sizeof(UInt16));
		} else if constexpr (BitsPerTag == 16) {
			UInt16 Word = static_cast<UInt16>(TagValue);
			Memory::Memcpy(Pointer + (I_SlotIndex << 1), &Word, sizeof(UInt16));
		} else if constexpr (BitsPerTag == 32) {
			Memory::Memcpy(Pointer + (I_SlotIndex * 4), &TagValue, sizeof(UInt32));
		} else {
			size_t BitOffset = I_SlotIndex * BitsPerTag;
			size_t ByteOffset = BitOffset / 8;
			BitOffset %= 8;
			for (size_t Bit = 0; Bit < BitsPerTag; ++Bit) {
				if (TagValue & (1u << Bit)) Pointer[ByteOffset] |= static_cast<UInt8>(1u << BitOffset);
				else Pointer[ByteOffset] &= static_cast<UInt8>(~(1u << BitOffset));
				if (++BitOffset == 8) { BitOffset = 0; ++ByteOffset; }
			}
		}
	}

	template<size_t BitsPerTag>
	Bool SingleTable<BitsPerTag>::FindTagInBuckets(UInt64 I_BucketIndex1, UInt64 I_BucketIndex2, UInt32 I_Tag) const
	{
		if constexpr (kBytesPerBucket <= 8) {
			const UInt8* Pointer1 = BucketPointer(I_BucketIndex1);
			const UInt8* Pointer2 = BucketPointer(I_BucketIndex2);
			UInt64 Value1 = 0, Value2 = 0;
			Memory::Memcpy(&Value1, Pointer1, sizeof(UInt64));
			Memory::Memcpy(&Value2, Pointer2, sizeof(UInt64));
			if constexpr (kBytesPerBucket < 8) {
				constexpr UInt64 kValidMask = (1ULL << (kBytesPerBucket * 8)) - 1;
				Value1 &= kValidMask;
				Value2 &= kValidMask;
			}
			if constexpr (BitsPerTag == 4 && kTagsPerBucket == 4) {
				return HasValue4(Value1, I_Tag) || HasValue4(Value2, I_Tag);
			} else if constexpr (BitsPerTag == 8 && kTagsPerBucket == 4) {
				return HasValue8(Value1, I_Tag) || HasValue8(Value2, I_Tag);
			} else if constexpr (BitsPerTag == 12 && kTagsPerBucket == 4) {
				return HasValue12(Value1, I_Tag) || HasValue12(Value2, I_Tag);
			} else if constexpr (BitsPerTag == 16 && kTagsPerBucket == 4) {
				return HasValue16(Value1, I_Tag) || HasValue16(Value2, I_Tag);
			}
		}
		for (UInt64 SlotIndex = 0; SlotIndex < kTagsPerBucket; ++SlotIndex) {
			if (ReadTag(I_BucketIndex1, SlotIndex) == I_Tag || ReadTag(I_BucketIndex2, SlotIndex) == I_Tag) return True;
		}
		return False;
	}

	template<size_t BitsPerTag>
	Bool SingleTable<BitsPerTag>::DeleteTagFromBucket(UInt64 I_BucketIndex, UInt32 I_Tag)
	{
		for (UInt64 SlotIndex = 0; SlotIndex < kTagsPerBucket; ++SlotIndex) {
			if (ReadTag(I_BucketIndex, SlotIndex) == I_Tag) {
				WriteTag(I_BucketIndex, SlotIndex, 0);
				return True;
			}
		}
		return False;
	}

	template<size_t BitsPerTag>
	Bool SingleTable<BitsPerTag>::InsertTagToBucket(UInt64 I_BucketIndex, UInt32 I_Tag, Bool I_Kickout, UInt32& I_OutOldTag, UInt32 I_KickoutSlot)
	{
		for (UInt64 SlotIndex = 0; SlotIndex < kTagsPerBucket; ++SlotIndex) {
			if (ReadTag(I_BucketIndex, SlotIndex) == 0) {
				WriteTag(I_BucketIndex, SlotIndex, I_Tag);
				return True;
			}
		}
		if (I_Kickout) {
			I_OutOldTag = ReadTag(I_BucketIndex, I_KickoutSlot % kTagsPerBucket);
			WriteTag(I_BucketIndex, I_KickoutSlot % kTagsPerBucket, I_Tag);
		}
		return False;
	}

	/** Bucket count for given max keys. Reference: cuckoofilter.h ctor (upperpower2, load > 0.96 then double). Argument to BitCeil is at least 1. */
	[[nodiscard]] inline UInt64 ComputeNumBuckets(UInt64 I_MaxNumKeys) noexcept
	{
		constexpr UInt64 Associativity = 4;
		UInt64 NumBuckets = Math::BitCeil(I_MaxNumKeys <= Associativity ? 1ull : I_MaxNumKeys / Associativity);
		if (I_MaxNumKeys > 0 && static_cast<double>(I_MaxNumKeys) / (NumBuckets * Associativity) > 0.96)
			NumBuckets *= 2;
		return NumBuckets;
	}

	/** Default 64-bit hasher: GoldenRatioHash(seed, item). FStringView uses CityHash64 (unseeded by design). Replaces reference TwoIndependentMultiplyShift. */
	template<typename ItemType>
	class DefaultCuckooHasher
	{
		UInt64 Seed;
	public:
		DefaultCuckooHasher() : Seed(FRandomSeed{}.Get<UInt64>()) {}
		explicit DefaultCuckooHasher(UInt64 I_Seed) : Seed(I_Seed) {}
		[[nodiscard]] UInt64 operator()(const ItemType& I_Item) const noexcept
		{
			if constexpr (std::is_same_v<std::decay_t<ItemType>, FStringView>) {
				return Math::CityHash64(I_Item);  // NOTE: FStringView path is unseeded by design
			} else {
				return Math::GoldenRatioHash(Seed, I_Item);
			}
		}
	};
}

export namespace Visera
{
	/** Probabilistic set with Insert/MayContain/Erase.
	 *  Duplicate inserts are allowed (multiple fingerprints). Erase removes one matching fingerprint.
	 *  Insert: returns False only when victim cache is already full; otherwise always succeeds (item stored in table or in victim).
	 *  MayContain may return false positives. Erase should only be used for items known to have been inserted,
	 *  otherwise a false-positive match may remove another item's fingerprint.
	 *  Reference: cuckoofilter-master src/cuckoofilter.h. Modifications: Bool return instead of Status enum; FPCG32 for kickout slot. */
	template<typename ItemType, size_t BitsPerItem, typename HashFamily = DefaultCuckooHasher<ItemType>>
	class VISERA_CORE_API FCuckooFilter
	{
	public:
		[[nodiscard]] Bool Insert(const ItemType& I_Item);
		[[nodiscard]] Bool MayContain(const ItemType& I_Item) const noexcept;
		[[nodiscard]] Bool Erase(const ItemType& I_Item);
		[[nodiscard]] UInt64 GetSize() const { return NumItems; }
		[[nodiscard]] UInt64 GetSizeInBytes() const { return Table.SizeInBytes(); }
		[[nodiscard]] FString GetInfo() const;

		explicit FCuckooFilter(UInt64 I_MaxNumKeys);

	private:
		static_assert(BitsPerItem >= 4 && BitsPerItem <= 32, "BitsPerItem must be in [4, 32]");
		static constexpr UInt32 kTagsPerBucket = 4;
		static constexpr UInt32 kMaxCuckooCount = 500;
		SingleTable<BitsPerItem> Table;
		UInt64 NumItems = 0;
		UInt64 VictimIndex = 0;
		UInt32 VictimTag = 0;
		Bool VictimUsed = False;
		HashFamily Hasher;
		FPCG32 Rng;

		[[nodiscard]] UInt64 IndexHash(UInt32 I_HashValue) const { return I_HashValue & (Table.GetNumBuckets() - 1); }
		[[nodiscard]] UInt32 TagHash(UInt32 I_HashValue) const;
		void GenerateIndexTagHash(const ItemType& I_Item, UInt64& I_OutIndex, UInt32& I_OutTag) const;
		[[nodiscard]] UInt64 AltIndex(UInt64 I_Index, UInt32 I_Tag) const { return IndexHash(static_cast<UInt32>(I_Index) ^ (I_Tag * 0x5bd1e995u)); }
		void InsertImplNoCount(UInt64 I_Index, UInt32 I_Tag);
		void InsertImplAndIncrCount(UInt64 I_Index, UInt32 I_Tag);
	};

	template<typename ItemType, size_t BitsPerItem, typename HashFamily>
	UInt32 FCuckooFilter<ItemType, BitsPerItem, HashFamily>::TagHash(UInt32 I_HashValue) const
	{
		constexpr UInt32 kTagMask = (BitsPerItem >= 32) ? 0xFFFFFFFFu : static_cast<UInt32>((1ULL << BitsPerItem) - 1);
		UInt32 Tag = I_HashValue & kTagMask;
		return Tag == 0 ? 1u : Tag;
	}

	template<typename ItemType, size_t BitsPerItem, typename HashFamily>
	void FCuckooFilter<ItemType, BitsPerItem, HashFamily>::GenerateIndexTagHash(const ItemType& I_Item, UInt64& I_OutIndex, UInt32& I_OutTag) const
	{
		UInt64 Hash = Hasher(I_Item);
		I_OutIndex = IndexHash(static_cast<UInt32>(Hash >> 32));
		I_OutTag = TagHash(static_cast<UInt32>(Hash));
	}

	template<typename ItemType, size_t BitsPerItem, typename HashFamily>
	void FCuckooFilter<ItemType, BitsPerItem, HashFamily>::InsertImplNoCount(UInt64 I_Index, UInt32 I_Tag)
	{
		UInt64 CurrentIndex = I_Index;
		UInt32 CurrentTag = I_Tag;
		UInt32 OldTag = 0;
		for (UInt32 Count = 0; Count < kMaxCuckooCount; ++Count) {
			Bool Kickout = (Count > 0);
			UInt32 KickoutSlot = Kickout ? (Rng.Uniform<UInt32>() % kTagsPerBucket) : 0;
			if (Table.InsertTagToBucket(CurrentIndex, CurrentTag, Kickout, OldTag, KickoutSlot))
				return;
			if (Kickout) CurrentTag = OldTag;
			CurrentIndex = AltIndex(CurrentIndex, CurrentTag);
		}
		VictimIndex = CurrentIndex;
		VictimTag = CurrentTag;
		VictimUsed = True;
	}

	template<typename ItemType, size_t BitsPerItem, typename HashFamily>
	void FCuckooFilter<ItemType, BitsPerItem, HashFamily>::InsertImplAndIncrCount(UInt64 I_Index, UInt32 I_Tag)
	{
		InsertImplNoCount(I_Index, I_Tag);
		++NumItems;
	}

	template<typename ItemType, size_t BitsPerItem, typename HashFamily>
	FCuckooFilter<ItemType, BitsPerItem, HashFamily>::FCuckooFilter(UInt64 I_MaxNumKeys)
		: Table(ComputeNumBuckets(I_MaxNumKeys)), Hasher()
	{
		const UInt64 BucketCount = Table.GetNumBuckets();
		VISERA_ASSERT(BucketCount > 0);
		VISERA_ASSERT(Math::IsPowerOfTwo(BucketCount));
		VISERA_ASSERT(BucketCount <= (1ull << 32) && "Index hash is 32-bit; bucket count must be at most 2^32.");
	}

	template<typename ItemType, size_t BitsPerItem, typename HashFamily>
	Bool FCuckooFilter<ItemType, BitsPerItem, HashFamily>::
	Insert(const ItemType& I_Item)
	{
		if (VictimUsed) { return False; }

		UInt64 Index;
		UInt32 Tag;
		GenerateIndexTagHash(I_Item, Index, Tag);
		InsertImplAndIncrCount(Index, Tag);
		return True;
	}

	template<typename ItemType, size_t BitsPerItem, typename HashFamily>
	Bool FCuckooFilter<ItemType, BitsPerItem, HashFamily>::
	MayContain(const ItemType& I_Item) const noexcept
	{
		UInt64 IndexPrimary;
		UInt64 IndexAlternate;
		UInt32 Tag;
		GenerateIndexTagHash(I_Item, IndexPrimary, Tag);
		IndexAlternate = AltIndex(IndexPrimary, Tag);
		if (VictimUsed && Tag == VictimTag && (IndexPrimary == VictimIndex || IndexAlternate == VictimIndex))
			return True;
		return Table.FindTagInBuckets(IndexPrimary, IndexAlternate, Tag);
	}

	template<typename ItemType, size_t BitsPerItem, typename HashFamily>
	Bool FCuckooFilter<ItemType, BitsPerItem, HashFamily>::
	Erase(const ItemType& I_Item)
	{
		UInt64 IndexPrimary;
		UInt64 IndexAlternate;
		UInt32 Tag;
		GenerateIndexTagHash(I_Item, IndexPrimary, Tag);
		IndexAlternate = AltIndex(IndexPrimary, Tag);
		auto TryReinsertVictim = [this]() {
			if (VictimUsed) {
				UInt64 SavedIndex = VictimIndex;
				UInt32 SavedTag = VictimTag;
				VictimUsed = False;
				InsertImplNoCount(SavedIndex, SavedTag);
			}
		};
		if (Table.DeleteTagFromBucket(IndexPrimary, Tag)) {
			--NumItems;
			TryReinsertVictim();
			return True;
		}
		if (Table.DeleteTagFromBucket(IndexAlternate, Tag)) {
			--NumItems;
			TryReinsertVictim();
			return True;
		}
		if (VictimUsed && Tag == VictimTag && (IndexPrimary == VictimIndex || IndexAlternate == VictimIndex)) {
			VictimUsed = False;
			--NumItems;
			return True;
		}
		return False; // item not found is normal for probabilistic filter.
	}

	template<typename ItemType, size_t BitsPerItem, typename HashFamily>
	FString FCuckooFilter<ItemType, BitsPerItem, HashFamily>::GetInfo() const
	{
		UInt64 Capacity = Table.SizeInTags();
		double LoadFactor = (Capacity > 0) ? (static_cast<double>(NumItems) / static_cast<double>(Capacity)) : 0.0;
		return FString::Format(
			"CuckooFilter: items={}, capacity={}, load_factor={:.4f}, victim_used={}, table_bytes={}",
			NumItems, Capacity, LoadFactor, VictimUsed ? "true" : "false", Table.SizeInBytes());
	}

}
