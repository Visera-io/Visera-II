module;
#include <Visera-Core.hpp>
#include <atomic>
export module Visera.Core.OS.Thread.Queue.SPSC;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    /**
     * Visera implementation of Unbounded SPSC Queue.
     *
     * Reference:
     *   https://sites.google.com/site/1024cores/home/lock-free-algorithms/queues/unbounded-spsc-queue
     *
     * Notes:
     * - Single Producer / Single Consumer only (misuse is UB).
     * - Unbounded: may allocate via ::operator new when cache is exhausted.
     * - Nodes are NOT freed until destruction (they are recycled via internal cache).
     */
    template<typename T>
    class VISERA_CORE_API TSPSCQueue
    {
    public:
        /**
         * Producer thread only.
         */
        template <typename U> inline void
        Produce(U&& I_Value)
        {
            FNode* Node = AllocateNode();
            Node->Value = std::forward<U>(I_Value);
            Node->Next.store(nullptr, std::memory_order_relaxed);

            // Publish node: consumer sees it via Tail->Next (acquire).
            StoreRelease(&Head->Next, Node);
            Head = Node;
        }

        /**
         * Consumer thread only.
         * Returns False if queue is empty.
         */
        [[nodiscard]] inline Bool
        Consume(TMutable<T> O_Value)
        {
            FNode* LocalTail = Tail.load(std::memory_order_relaxed);

            // Acquire pairs with producer's release on Head->Next.
            FNode* Next = LocalTail->Next.load(std::memory_order_acquire);
            if (!Next) { return False; }

            *O_Value = std::move(Next->Value);

            // Publish new tail for producer-side recycling snapshot.
            StoreRelease(&Tail, Next);
            return True;
        }

        /**
         * Consumer thread only (best-effort).
         */
        [[nodiscard]] inline Bool
        IsEmpty() const noexcept
        {
            FNode* LocalTail = Tail.load(std::memory_order_acquire);
            return LocalTail ->Next.load(std::memory_order_acquire) == nullptr;
        }


    private:
        struct FNode
        {
            std::atomic<FNode*> Next  {nullptr};
            T                   Value {};
        };
        static_assert(std::is_default_constructible_v<T>, "TSPSCQueue<T> requires T to be default-constructible in this implementation.");

        // Consumer Part (the Producer uses in AllocateNode so use atomic)
        alignas(VISERA_CACHELINE_SIZE)
        std::atomic<FNode*> Tail {nullptr};

        // Delimiter between Consumer Part and Producer Part
        // So that they situate on different cache lines
        alignas(VISERA_CACHELINE_SIZE)
        FByte CacheLinePadding[VISERA_CACHELINE_SIZE]{};

        // Producer Part
        FNode* Head     {nullptr};
        FNode* LastUsed {nullptr}; // Tail of node cache
        FNode* TailCopy {nullptr}; // helper (points somewhere between first_ and

    public:
        TSPSCQueue()
        {
            FNode* Dummy = new FNode();
            Dummy->Next.store(nullptr, std::memory_order_relaxed);

            Head     = Dummy;
            LastUsed = Dummy;
            TailCopy = Dummy;
            Tail.store(Dummy, std::memory_order_relaxed);
        }

        ~TSPSCQueue()
        {
            // Delete all nodes reachable from cache chain (LastUsed -> ...)
            // Since nodes are recycled and never freed during runtime, this reclaims everything.
            FNode* Node = LastUsed;
            while (Node)
            {
                FNode* Next = Node->Next.load(std::memory_order_relaxed);
                delete Node;
                Node = Next;
            }
        }

        TSPSCQueue(const TSPSCQueue&)            = delete;
        TSPSCQueue& operator=(const TSPSCQueue&) = delete;
        TSPSCQueue(TSPSCQueue&&)                 = delete;
        TSPSCQueue& operator=(TSPSCQueue&&)      = delete;

    private:
        // ---- Atomic helpers (explicit semantics, discourage misuse) ----
        static inline FNode* LoadConsume(const std::atomic<FNode*>* I_Address) noexcept
        {
            // Use acquire as a portable substitute for "consume".
            return I_Address->load(std::memory_order_acquire);
        }

        static inline void StoreRelease(std::atomic<FNode*>* I_Address, FNode* I_Value) noexcept
        {
            I_Address->store(I_Value, std::memory_order_release);
        }

        FNode* AllocateNode()
        {
            // Fast path: cache not exhausted (LastUsed hasn't caught up to TailCopy).
            if (LastUsed != TailCopy)
            {
                FNode* Node = LastUsed;
                LastUsed    = LastUsed->Next.load(std::memory_order_relaxed);
                return Node;
            }

            // Refresh snapshot of consumer tail only when cache seems exhausted.
            TailCopy = LoadConsume(&Tail);

            if (LastUsed != TailCopy)
            {
                FNode* Node = LastUsed;
                LastUsed = LastUsed->Next.load(std::memory_order_relaxed);
                return Node;
            }

            // Truly exhausted -> allocate new node.
            return new FNode();
        } 
    };
}