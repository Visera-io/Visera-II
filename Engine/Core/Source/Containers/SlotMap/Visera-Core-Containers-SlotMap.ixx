module;
#include <Visera-Core.hpp>
export module Visera.Core.Containers.SlotMap;
#define VISERA_MODULE_NAME "Core.Containers"
import Visera.Core.Containers.Array;
import Visera.Core.Types.Handle;
import Visera.Core.Types.Pointer.Unique;
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
        template<typename... Args> [[nodiscard]] HandleType
        Insert(ValueType&& I_Value, Args&&... I_Args) requires std::move_constructible<ValueType> && std::constructible_from<HandleType, UInt32, UInt32, Args...>;
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
    template<typename... Args> [[nodiscard]] HandleType TSlotMap<ValueType, HandleType>::
    Insert(ValueType&& I_Value, Args&&... I_Args) requires std::move_constructible<ValueType> && std::constructible_from<HandleType, UInt32, UInt32, Args...>
    {
        UInt32 SlotIndex = Allocate();
        FSlot& Slot      = Slots[SlotIndex];

        Slot.Index = static_cast<UInt32>(Data.GetSize());
        Data.PushBack(MakeUnique<ValueType>(std::move(I_Value)));
        DataToSlot.PushBack(SlotIndex);
        Size += 1;

        return MakeHandle<HandleType>(Slot.Generation, SlotIndex, std::forward<Args>(I_Args)...);
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
        if (DataIndex != Data.GetSize() - 1)
        {
            Data[DataIndex] = std::move(Data.Back());
            
            // Use reverse mapping for O(1) lookup
            UInt32 LastDataIndex = static_cast<UInt32>(Data.GetSize() - 1);
            UInt32 LastSlotIndex = DataToSlot[LastDataIndex];
            DataToSlot[DataIndex] = LastSlotIndex;
            Slots[LastSlotIndex].Index = DataIndex;
        }

        Data.PopBack();
        DataToSlot.PopBack();
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
        return Data[Slot.Index].Get();
    }

    template<typename ValueType, Concepts::Handle HandleType>
    const ValueType* TSlotMap<ValueType, HandleType>::
    Get(HandleType I_Handle) const
    {
        if (!HasHandle(I_Handle)) { return nullptr; }

        UInt32 SlotIndex = I_Handle.GetIndex();
        const FSlot& Slot = Slots[SlotIndex];
        // Return raw pointer from TUniquePtr - address is stable even if Data reallocates
        return Data[Slot.Index].Get();
    }

    template<typename ValueType, Concepts::Handle HandleType>
    void TSlotMap<ValueType, HandleType>::
    Clear()
    {
        Slots.Clear();
        Data.Clear();
        DataToSlot.Clear();
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
            SlotIndex = static_cast<UInt32>(Slots.GetSize());
            FSlot NewSlot
            {
                .Generation = GenerationRange.Left,
                .Index      = InvalidIndex,
                .NextFree   = InvalidIndex,
            };
            Slots.PushBack(std::move(NewSlot));
        }
        return SlotIndex;
    }

    template<typename ValueType, Concepts::Handle HandleType>
    void TSlotMap<ValueType, HandleType>::
    Free(UInt32 I_SlotIndex)
    {
        VISERA_ASSERT(I_SlotIndex < Slots.GetSize());
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
        if (SlotIndex >= Slots.GetSize())
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
