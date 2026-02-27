module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Thread.Queue.MPSC;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.OS.Thread.Sync.Atomic;
import Visera.Core.OS.Memory;
import Visera.Core.Types.Optional;

export namespace Visera
{
    /**
     * Fast multi-producer/single-consumer unbounded concurrent queue.
     * Based on http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
     * UE5: Engine/Source/Runtime/Core/Public/Containers/MpscQueue.h
     *
     * - Multiple Producers / Single Consumer only (misuse is UB).
     * - Nodes use TTypeCompatibleBytes<T>; value lifetime is managed with placement new and DestroyUnchecked().
     * - Allocation via Visera::Memory::Malloc/Free (alignof(FNode)).
     */
    template<typename T>
    class VISERA_CORE_API TMPSCQueue final
    {
    public:
        using ElementType = T;

        template <typename... ArgTypes> void
        Enqueue(ArgTypes&&... Args)
        {
            FNode* New = new (Memory::Malloc(sizeof(FNode), alignof(FNode))) FNode;
            new (reinterpret_cast<void*>(&New->Value)) ElementType(std::forward<ArgTypes>(Args)...);

            FNode* Prev = Head.Exchange(New, EMemoryOrder::AcqRel);
            Prev->Next.Store(New, EMemoryOrder::Release);
        }

        [[nodiscard]] TOptional<ElementType>
        Dequeue()
        {
            FNode* Next = Tail->Next.Load(EMemoryOrder::Acquire);

            if (Next == nullptr) { return {}; }

            ElementType* ValuePtr = reinterpret_cast<ElementType*>(&Next->Value);
            TOptional<ElementType> Res{ std::move(*ValuePtr) };
            Next->Value.DestroyUnchecked();

            Memory::Free(Tail, alignof(FNode)); // current sentinel
            Tail = Next; // new sentinel
            return Res;
        }

        /**
         * Consumer only. Returns a pointer to the front element if the queue is not empty, nullptr otherwise.
         */
        [[nodiscard]] ElementType*
        Peek() const
        {
            FNode* Next = Tail->Next.Load(EMemoryOrder::Acquire);
            return Next? reinterpret_cast<ElementType*>(&Next->Value) : nullptr;
        }

        [[nodiscard]] Bool
        IsEmpty() const { return Tail->Next.Load(EMemoryOrder::Acquire) == nullptr; }

    private:
        struct FNode
        {
            TAtomic<FNode*>                   Next{ nullptr };
            TTypeCompatibleBytes<ElementType> Value;
        };
        alignas(VISERA_CACHELINE_SIZE)
        TAtomic<FNode*> Head { nullptr }; // accessed only by producers
        alignas(VISERA_CACHELINE_SIZE)
        FNode*              Tail { nullptr }; // accessed only by consumer

    public:
        TMPSCQueue()
        {
            FNode* Sentinel = new (Memory::Malloc(sizeof(FNode), alignof(FNode))) FNode;
            Head.Store(Sentinel, EMemoryOrder::Relaxed);
            Tail = Sentinel;
        }

        ~TMPSCQueue()
        {
            FNode* Next = Tail->Next.Load(EMemoryOrder::Relaxed);

            // Sentinel's value storage was never constructed.
            Memory::Free(Tail, alignof(FNode));

            while (Next != nullptr)
            {
                Tail = Next;
                Next = Tail->Next.Load(EMemoryOrder::Relaxed);

                Tail->Value.DestroyUnchecked();
                Memory::Free(Tail, alignof(FNode));
            }
        }

        TMPSCQueue(const TMPSCQueue&)            = delete;
        TMPSCQueue& operator=(const TMPSCQueue&) = delete;
        TMPSCQueue(TMPSCQueue&&)                 = delete;
        TMPSCQueue& operator=(TMPSCQueue&&)      = delete;
    };
}
