module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.SlotMap;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Array;
import Visera.Core.Types.Handle;
import Visera.Core.Math.Arithmetic.Interval;

export namespace Visera
{
    template<typename                   ValueType,
             Concepts::Handle           HandleType = FHandle>
    class VISERA_CORE_API TSlotMap
    {
        static constexpr TClosedInterval<UInt32>
        GenerationRange = HandleType::GetGenerationRange();

        static_assert(!GenerationRange.IsDegenerate());
        static_assert(!GenerationRange.Contains(HandleType{}.GetGeneration()));
    public:
        static constexpr UInt32 InvalidIndex = ~0U;
        struct FSlot
        {
            UInt32 Generation = GenerationRange.Left;
            UInt32 Index      = InvalidIndex; // InvalidHandle marks free slot
            UInt32 NextFree   = InvalidIndex; // Free list link
        };

        [[nodiscard]] HandleType
        Insert(const ValueType& I_Value);
        [[nodiscard]] HandleType
        Insert(ValueType&& I_Value);
        [[nodiscard]] Bool
        Erase(HandleType I_Handle);
        void
        Clear();
        [[nodiscard]] ValueType*
        Get(HandleType I_Handle);
        [[nodiscard]] const ValueType*
        Get(HandleType I_Handle) const;
        [[nodiscard]] Bool
        Contains(HandleType I_Handle) const { return HasHandle(I_Handle); };
        [[nodiscard]] UInt32
        GetSize() const { return Size; }
        [[nodiscard]] Bool
        IsEmpty() const { return Size == 0; }

    private:
        TArray<FSlot>                 Slots;
        TArray<TUniquePtr<ValueType>> Data; // Store as TUniquePtr for pointer stability
        TArray<UInt32>                DataToSlot; // Reverse mapping: dense index -> slot index
        UInt32 FreeHead = InvalidIndex;
        UInt32 Size     = 0;

    private:
        [[nodiscard]] UInt32
        Allocate();
        void
        Free(UInt32 I_SlotIndex);
        [[nodiscard]] Bool
        HasHandle(HandleType I_Handle) const;

    public:
        TSlotMap();
        ~TSlotMap() = default;
        TSlotMap(const TSlotMap&) = delete;
        TSlotMap& operator=(const TSlotMap&) = delete;
        TSlotMap(TSlotMap&&) noexcept = default;
        TSlotMap& operator=(TSlotMap&&) noexcept = default;
    };

    template<typename ValueType, Concepts::Handle HandleType>
    TSlotMap<ValueType, HandleType>::
    TSlotMap()
        : FreeHead(InvalidIndex)
        , Size(0)
    {

    }

    template<typename ValueType, Concepts::Handle HandleType>
    HandleType TSlotMap<ValueType, HandleType>::
    Insert(const ValueType& I_Value)
    {
        UInt32 SlotIndex = Allocate();
        FSlot& Slot      = Slots[SlotIndex];

        // Always append (reused slots have InvalidIndex, so always append)
        // Wrap in TUniquePtr to ensure pointer stability even when Data reallocates
        Slot.Index = static_cast<UInt32>(Data.size());
        Data.push_back(MakeUnique<ValueType>(I_Value));
        DataToSlot.push_back(SlotIndex);
        Size += 1;
        return HandleType(Slot.Generation, SlotIndex);
    }

    template<typename ValueType, Concepts::Handle HandleType>
    HandleType TSlotMap<ValueType, HandleType>::
    Insert(ValueType&& I_Value)
    {
        UInt32 SlotIndex = Allocate();
        FSlot& Slot      = Slots[SlotIndex];

        // Always append (reused slots have InvalidIndex, so always append)
        // Wrap in TUniquePtr to ensure pointer stability even when Data reallocates
        Slot.Index = static_cast<UInt32>(Data.size());
        Data.push_back(MakeUnique<ValueType>(std::move(I_Value)));
        DataToSlot.push_back(SlotIndex);
        Size += 1;
        return HandleType(Slot.Generation, SlotIndex);
    }

    template<typename ValueType, Concepts::Handle HandleType>
    Bool TSlotMap<ValueType, HandleType>::
    Erase(HandleType I_Handle)
    {
        if (!HasHandle(I_Handle)) { return False; }

        UInt32 SlotIndex = I_Handle.GetIndex();
        FSlot& Slot      = Slots[SlotIndex];
        UInt32 DataIndex = Slot.Index;

        // Swap with last element to maintain density
        if (DataIndex != Data.size() - 1)
        {
            Data[DataIndex] = std::move(Data.back());
            
            // Use reverse mapping for O(1) lookup
            UInt32 LastDataIndex = static_cast<UInt32>(Data.size() - 1);
            UInt32 LastSlotIndex = DataToSlot[LastDataIndex];
            DataToSlot[DataIndex] = LastSlotIndex;
            Slots[LastSlotIndex].Index = DataIndex;
        }

        Data.pop_back();
        DataToSlot.pop_back();
        Free(SlotIndex);
        Size -= 1;
        return True;
    }

    template<typename ValueType, Concepts::Handle HandleType>
    ValueType* TSlotMap<ValueType, HandleType>::
    Get(HandleType I_Handle)
    {
        if (!HasHandle(I_Handle)) { return nullptr; }

        UInt32 SlotIndex = I_Handle.GetIndex();
        const FSlot& Slot = Slots[SlotIndex];
        // Return raw pointer from TUniquePtr - address is stable even if Data reallocates
        return Data[Slot.Index].get();
    }

    template<typename ValueType, Concepts::Handle HandleType>
    const ValueType* TSlotMap<ValueType, HandleType>::
    Get(HandleType I_Handle) const
    {
        if (!HasHandle(I_Handle)) { return nullptr; }

        UInt32 SlotIndex = I_Handle.GetIndex();
        const FSlot& Slot = Slots[SlotIndex];
        // Return raw pointer from TUniquePtr - address is stable even if Data reallocates
        return Data[Slot.Index].get();
    }

    template<typename ValueType, Concepts::Handle HandleType>
    void TSlotMap<ValueType, HandleType>::
    Clear()
    {
        Slots.clear();
        Data.clear();
        DataToSlot.clear();
        FreeHead = InvalidIndex;
        Size = 0;
    }

    template<typename ValueType, Concepts::Handle HandleType>
    [[nodiscard]] UInt32 TSlotMap<ValueType, HandleType>::
    Allocate()
    {
        UInt32 SlotIndex {InvalidIndex};
        if (FreeHead != InvalidIndex)
        {
            // Reuse a free slot
            SlotIndex       = FreeHead;
            FSlot& Slot     = Slots[SlotIndex];
            FreeHead        = Slot.NextFree;
            Slot.NextFree   = InvalidIndex;
            // Generation is already incremented in Free(), don't increment again
            // Index is already set to InvalidHandle in Free(), will be set in Insert()
        }
        else
        {
            // Allocate a new slot
            SlotIndex = static_cast<UInt32>(Slots.size());
            FSlot NewSlot
            {
                .Generation = GenerationRange.Left,
                .Index      = InvalidIndex,
                .NextFree   = InvalidIndex,
            };
            Slots.push_back(std::move(NewSlot));
        }
        return SlotIndex;
    }

    template<typename ValueType, Concepts::Handle HandleType>
    void TSlotMap<ValueType, HandleType>::
    Free(UInt32 I_SlotIndex)
    {
        VISERA_ASSERT(I_SlotIndex < Slots.size());
        FSlot& Slot = Slots[I_SlotIndex];
        VISERA_ASSERT(Slot.Index != InvalidIndex && "Double free detected");

        // Mark slot as free by setting Index to InvalidIndex
        Slot.Index = InvalidIndex;

        if (Slot.Generation == GenerationRange.Right)
        { Slot.Generation = GenerationRange.Left; }
        else
        { Slot.Generation += 1; }
        
        // Add to free list
        Slot.NextFree = FreeHead;
        FreeHead = I_SlotIndex;
    }

    template<typename ValueType, Concepts::Handle HandleType>
    [[nodiscard]] Bool TSlotMap<ValueType, HandleType>::
    HasHandle(HandleType I_Handle) const
    {
        if (I_Handle.IsNull()) { return False; }

        UInt32 SlotIndex = I_Handle.GetIndex();
        if (SlotIndex >= Slots.size())
        { return False; }

        const FSlot& Slot = Slots[SlotIndex];
        
        // Check if slot is free (Index == InvalidIndex marks free slots)
        if (Slot.Index == InvalidIndex)
        { return False; } // Slot is free

        // Check generation match
        if (Slot.Generation != I_Handle.GetGeneration())
        { return False; } // Generation mismatch

        return True;
    }
}
