module;
#include <Visera-Core.hpp>
export module Visera.Core.Delegate.Multicast;
#define VISERA_MODULE_NAME "Core.Delegate"

namespace Visera
{
    export template<typename... T_Args>
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
            bool      bPendingRemove{False};
        };

    public:
        // 订阅：返回一个 Handle，用它来 Unsubscribe
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

            // 如果当前正在 Broadcast，则先放到 PendingAdd，等 Broadcast 结束后再真正加进去
            if (LockCount > 0)
            {
                PendingAdd.push_back(std::move(NewSlot));
            }
            else
            {
                Slots.push_back(std::move(NewSlot));
            }

            return NewHandle;
        }

        // 退订：可以在 Broadcast 过程中调用
        inline void
        Unsubscribe(FHandle I_Handle)
        {
            if (I_Handle == 0) { return; }

            if (!PendingAdd.empty())
            {
                for (auto It = PendingAdd.begin(); It != PendingAdd.end(); ++It)
                {
                    if (It->Handle == I_Handle)
                    {
                        PendingAdd.erase(It);
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
                        // 正在 Broadcast：逻辑删除，当前轮剩余时间内不再调用
                        Slot.bPendingRemove = true;
                        Slot.Callback       = nullptr;
                    }
                    else
                    {
                        // 不在 Broadcast：可以直接压缩
                        Slot.bPendingRemove = true;
                        Compact();
                    }
                    return;
                }
            }
        }

        inline void
        Broadcast(T_Args&&... I_Args)
        {
            ++LockCount;
            for (auto& Slot : Slots)
            {
                if (!Slot.bPendingRemove && Slot.Callback)
                {
                    Slot.Callback(std::forward<T_Args>(I_Args)...);
                }
            }
            --LockCount;

            if (LockCount == 0)
            {
                Compact();

                if (!PendingAdd.empty())
                {
                    for (auto& NewSlot : PendingAdd)
                    {
                        Slots.push_back(std::move(NewSlot));
                    }
                    PendingAdd.clear();
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
                PendingAdd.clear();
            }
            else
            {
                Slots.clear();
                PendingAdd.clear();
            }
        }

        inline bool
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
                    {
                        *ItWrite = std::move(*ItRead);
                    }
                    ++ItWrite;
                }
            }

            if (ItWrite != Slots.end())
            {
                Slots.erase(ItWrite, Slots.end());
            }
        }

    private:
        TArray<FSlot> Slots;
        TArray<FSlot> PendingAdd;
        FHandle       NextHandle{1};   // 0 means Invalid
        Int32         LockCount {0};   // Cascaded Broadcast
    };
}