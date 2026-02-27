module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Thread.Queue.SPSC;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.OS.Thread.Sync.Atomic;
import Visera.Core.OS.Memory;
import Visera.Core.Types.Optional;

export namespace Visera
{
    /**
     * Visera implementation of Unbounded SPSC Queue.
     *
     * Reference:
     *   https://sites.google.com/site/1024cores/home/lock-free-algorithms/queues/unbounded-spsc-queue
     *   https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/Core/Public/Containers/SpscQueue.h
     *
     * Notes:
     * - Single Producer / Single Consumer only (misuse is UB).
     * - Unbounded: may allocate via ::operator new when cache is exhausted.
     * - Nodes are NOT freed until destruction (they are recycled via internal cache).
     */
    template<typename T>
    class VISERA_CORE_API TSPSCQueue final
    {
    public:
        using ElementType = T;

        template <typename... ArgTypes> void
        Enqueue(ArgTypes&&... Args)
        {
            FNode* Node = AllocateNode();
            new (reinterpret_cast<void*>(&Node->Value)) ElementType (std::forward<ArgTypes>(Args)...);

            Head->Next.Store(Node, EMemoryOrder::Release);
            Head = Node;
        }

        TOptional<ElementType>
        Dequeue()
        {
            FNode* LocalTail = Tail.Load(EMemoryOrder::Relaxed);
            FNode* LocalTailNext = LocalTail->Next.Load(EMemoryOrder::Acquire);
            if (LocalTailNext == nullptr) {  return NullOpt; }

            ElementType* TailNextValue = reinterpret_cast<ElementType*>(&LocalTailNext->Value);
            TOptional<ElementType> Value{ std::move(*TailNextValue) };
            std::destroy_at(TailNextValue);

            Tail.Store(LocalTailNext, EMemoryOrder::Release);

            return Value;
        }

        [[nodiscard]] Bool
        IsEmpty() const { return Tail.Load(EMemoryOrder::Relaxed)->Next.Load(EMemoryOrder::Acquire) == nullptr; }

        [[nodiscard]] ElementType*
        Peek() const
        {
            FNode* LocalTail = Tail.Load(EMemoryOrder::Relaxed);
            FNode* LocalTailNext = LocalTail->Next.Load(EMemoryOrder::Acquire);

            if (LocalTailNext == nullptr)
            { return nullptr; }

            return reinterpret_cast<ElementType*>(&LocalTailNext->Value);
        }

    private:
        struct FNode
        {
            TAtomic<FNode*>                   Next { nullptr };
            TTypeCompatibleBytes<ElementType> Value;
        };

        // consumer part (accessed mainly by consumer, infrequently by producer)
        alignas(VISERA_CACHELINE_SIZE)
        TAtomic<FNode*> Tail; // tail of the queue

        // producer part (accessed only by producer)
        alignas(VISERA_CACHELINE_SIZE)
        FNode* Head; // head of the queue

        FNode* First; // last unused node (tail of node cache)
        FNode* TailCopy; // helper (points somewhere between First and Tail)

    public:
        [[nodiscard]]
        TSPSCQueue()
        {
            FNode* Node = new (Memory::Malloc(sizeof(FNode), alignof(FNode))) FNode;
            Tail.Store(Node, EMemoryOrder::Relaxed);
            Head = First = TailCopy = Node;
        }

        ~TSPSCQueue()
        {
            FNode* Node = First;
            FNode* LocalTail = Tail.Load(EMemoryOrder::Relaxed);

            // Delete all nodes which are the sentinel or unoccupied
            bool bContinue = false;
            do
            {
                FNode* Next = Node->Next.Load(EMemoryOrder::Relaxed);
                bContinue = Node != LocalTail;
                Memory::Free(Node, alignof(FNode));
                Node = Next;
            } while (bContinue);

            // Delete all nodes which are occupied, destroying the element first
            while (Node != nullptr)
            {
                FNode* Next = Node->Next.Load(EMemoryOrder::Relaxed);
                std::destroy_at(reinterpret_cast<ElementType*>(&Node->Value));
                Memory::Free(Node, alignof(FNode));
                Node = Next;
            }
        }
        TSPSCQueue(const TSPSCQueue&)            = delete;
        TSPSCQueue& operator=(const TSPSCQueue&) = delete;
        TSPSCQueue(TSPSCQueue&&)                 = delete;
        TSPSCQueue& operator=(TSPSCQueue&&)      = delete;

    private:
        FNode* AllocateNode()
        {
            // first tries to allocate node from internal node cache,
            // if attempt fails, allocates node via ::operator new()
            auto AllocateFromCache = [this]()
            {
                FNode* Node = First;
                First = First->Next.Load(EMemoryOrder::Relaxed);
                Node->Next.Store(nullptr, EMemoryOrder::Relaxed);
                return Node;
            };

            if (First != TailCopy)
            {
                return AllocateFromCache();
            }

            TailCopy = Tail.Load(EMemoryOrder::Acquire);
            if (First != TailCopy)
            {
                return AllocateFromCache();
            }

            return new (Memory::Malloc(sizeof(FNode), alignof(FNode))) FNode;
        }
    };
}