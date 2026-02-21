module;
#include <Visera-Core.hpp>
export module Visera.Core.Delegate.Multicast;
#define VISERA_MODULE_NAME "Core.Delegate"
import Visera.Core.Containers.Array;
import Visera.Core.Types.Function;

export namespace Visera
{
    template<typename... T_Args>
    class VISERA_CORE_API TMulticastDelegate
    {
    public:
        using FHandle   = UInt64;
        using FCallback = TFunction<void(T_Args...)>;

    private:
        struct FSlot
        {
            FHandle   Handle  {0};
            FCallback Callback{};
            Bool      bPendingRemove{False};
        };

    public:
        inline FHandle
        Subscribe(FCallback I_Function)
        {
            VISERA_ASSERT(I_Function && "Subscribe: callback must be valid.");

            const FHandle NewHandle = NextHandle++;

            FSlot NewSlot
            {
                .Handle = NewHandle,
                .Callback = std::move(I_Function),
                .bPendingRemove = False,
            };

            if (LockCount > 0)
            { PendingAdd.PushBack(std::move(NewSlot)); }
            else
            { Slots.PushBack(std::move(NewSlot)); }

            return NewHandle;
        }

        inline void
        Unsubscribe(FHandle I_Handle)
        {
            if (I_Handle == 0) { return; }

            if (!PendingAdd.IsEmpty())
            {
                for (auto It = PendingAdd.begin(); It != PendingAdd.end(); ++It)
                {
                    if (It->Handle == I_Handle)
                    {
                        PendingAdd.Erase(It);
                        return;
                    }
                }
            }

            for (auto& Slot : Slots)
            {
                if (Slot.Handle == I_Handle)
                {
                    if (LockCount > 0)
                    {
                        Slot.bPendingRemove = true;
                        Slot.Callback       = nullptr;
                    }
                    else
                    {
                        Slot.bPendingRemove = true;
                        Compact();
                    }
                    return;
                }
            }
        }

        inline void
        Broadcast(T_Args... I_Args)
        {
            VISERA_ASSERT(!LockCount && "Recursive broadcast is prohibited!");
            ++LockCount;
            for (auto& Slot : Slots)
            {
                if (!Slot.bPendingRemove && Slot.Callback)
                {
                    Slot.Callback(I_Args...);
                }
            }
            --LockCount;

            if (LockCount == 0)
            {
                Compact();

                if (!PendingAdd.IsEmpty())
                {
                    for (auto& NewSlot : PendingAdd)
                    {
                        Slots.PushBack(std::move(NewSlot));
                    }
                    PendingAdd.Clear();
                }
            }
        }

        inline void
        Clear()
        {
            if (LockCount > 0)
            {
                for (auto& Slot : Slots)
                {
                    Slot.bPendingRemove = True;
                    Slot.Callback       = nullptr;
                }
                PendingAdd.Clear();
            }
            else
            {
                Slots.Clear();
                PendingAdd.Clear();
            }
        }

        [[nodiscard]] inline Bool
        IsEmpty() const
        {
            if (!PendingAdd.empty())
            { return False; }

            for (const auto& Slot : Slots)
            {
                if (!Slot.bPendingRemove && Slot.Callback)
                { return False; }
            }
            return True;
        }

    private:
        inline void
        Compact()
        {
            auto ItWrite = Slots.begin();
            auto ItRead  = Slots.begin();

            for (; ItRead != Slots.end(); ++ItRead)
            {
                if (!ItRead->bPendingRemove && ItRead->Callback)
                {
                    if (ItWrite != ItRead)
                    { *ItWrite = std::move(*ItRead); }
                    ++ItWrite;
                }
            }

            if (ItWrite != Slots.end())
            {
                Slots.Erase(ItWrite, Slots.end());
            }
        }

    private:
        TArray<FSlot> Slots;
        TArray<FSlot> PendingAdd;
        FHandle       NextHandle{1};   // 0 means Invalid
        Int32         LockCount {0};   // Cascaded Broadcast
    };
}