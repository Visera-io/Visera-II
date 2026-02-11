module;
#include <Visera-Core.hpp>
#include <atomic>
export module Visera.Core.OS.Thread.Queue.SPSC;
#define VISERA_MODULE_NAME "Core.OS"
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

            Head->Next.store(Node, std::memory_order_release);
            Head = Node;
        }

        TOptional<ElementType>
        Dequeue()
        {
            FNode* LocalTail = Tail.load(std::memory_order_relaxed);
            FNode* LocalTailNext = LocalTail->Next.load(std::memory_order_acquire);
            if (LocalTailNext == nullptr) {  return NullOpt; }

            ElementType* TailNextValue = reinterpret_cast<ElementType*>(&LocalTailNext->Value);
            TOptional<ElementType> Value{ std::move(*TailNextValue) };
            std::destroy_at(TailNextValue);

            Tail.store(LocalTailNext, std::memory_order_release);

            return Value;
        }

        [[nodiscard]] Bool
        IsEmpty() const { return Tail.load(std::memory_order_relaxed)->Next.load(std::memory_order_acquire) == nullptr; }

        [[nodiscard]] ElementType*
        Peek() const
        {
            FNode* LocalTail = Tail.load(std::memory_order_relaxed);
            FNode* LocalTailNext = LocalTail->Next.load(std::memory_order_acquire);

            if (LocalTailNext == nullptr)
            { return nullptr; }

            return reinterpret_cast<ElementType*>(&LocalTailNext->Value);
        }

    private:
        struct FNode
        {
            std::atomic<FNode*>               Next { nullptr };
            TTypeCompatibleBytes<ElementType> Value;
        };

        // consumer part (accessed mainly by consumer, infrequently by producer)
        alignas(VISERA_CACHELINE_SIZE)
        std::atomic<FNode*> Tail; // tail of the queue

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
            Tail.store(Node, std::memory_order_relaxed);
            Head = First = TailCopy = Node;
        }

        ~TSPSCQueue()
        {
            FNode* Node = First;
            FNode* LocalTail = Tail.load(std::memory_order_relaxed);

            // Delete all nodes which are the sentinel or unoccupied
            bool bContinue = false;
            do
            {
                FNode* Next = Node->Next.load(std::memory_order_relaxed);
                bContinue = Node != LocalTail;
                Memory::Free(Node, alignof(FNode));
                Node = Next;
            } while (bContinue);

            // Delete all nodes which are occupied, destroying the element first
            while (Node != nullptr)
            {
                FNode* Next = Node->Next.load(std::memory_order_relaxed);
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
                First = First->Next.load(std::memory_order_relaxed);
                Node->Next.store(nullptr, std::memory_order_relaxed);
                return Node;
            };

            if (First != TailCopy)
            {
                return AllocateFromCache();
            }

            TailCopy = Tail.load(std::memory_order_acquire);
            if (First != TailCopy)
            {
                return AllocateFromCache();
            }

            return new (Memory::Malloc(sizeof(FNode), alignof(FNode))) FNode;
        }
    };
}