module;
#include <Visera-Core.hpp>
#include <cstddef>
#include <memory_resource>
export module Visera.Core.OS.Memory.Arena;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.Math.Bit;

export namespace Visera
{
    // Custom monotonic buffer resource implementation to avoid MSVC linker issues
    // with std::pmr::monotonic_buffer_resource::_Min_allocation
    class VISERA_CORE_API FMonotonicBufferResource : public std::pmr::memory_resource
    {
    public:
        /// Construct with initial buffer and upstream resource
        /// @param I_Buffer Initial buffer to use (can be nullptr)
        /// @param I_Bytes Size of initial buffer
        /// @param I_Upstream Upstream allocator for overflow blocks
        FMonotonicBufferResource(
            void* I_Buffer,
            std::size_t I_Bytes,
            std::pmr::memory_resource* I_Upstream = std::pmr::get_default_resource()) noexcept
            : CurrentBuffer{ static_cast<std::byte*>(I_Buffer) }
            , BufferSize{ I_Bytes }
            , CurrentOffset{ 0 }
            , Upstream{ I_Upstream }
        {}

        /// Release all allocated blocks (except the initial buffer)
        void release() noexcept
        {
            // Free all overflow blocks
            Block* CurrBlock = OverflowBlocks;
            while (CurrBlock)
            {
                Block* Next = CurrBlock->Next;
                
                // Calculate the actual allocated size (with padding)
                std::size_t BlockHeaderSize = sizeof(Block);
                std::size_t DataOffset = (BlockHeaderSize + CurrBlock->Alignment - 1) & ~(CurrBlock->Alignment - 1);
                std::size_t TotalSize = DataOffset + CurrBlock->Size;
                std::size_t BlockAlignment = (alignof(Block) > CurrBlock->Alignment) ? alignof(Block) : CurrBlock->Alignment;
                
                Upstream->deallocate(CurrBlock, TotalSize, BlockAlignment);
                CurrBlock = Next;
            }
            OverflowBlocks = nullptr;
            CurrentOffset = 0;
        }

        /// Get the upstream memory resource
        [[nodiscard]] std::pmr::memory_resource*
        GetUpstream() const noexcept { return Upstream; }

        /// Destructor releases all overflow blocks
        ~FMonotonicBufferResource()
        {
            release();
        }

    private:
        struct Block
        {
            Block* Next;
            std::size_t Size;
            std::size_t Alignment;
            // Data follows, but we need to handle alignment manually
        };

        void* do_allocate(std::size_t I_Bytes, std::size_t I_Alignment) override
        {
            VISERA_ASSERT(Math::IsPowerOfTwo(I_Alignment));

            // Calculate aligned offset
            std::size_t AlignedOffset = (CurrentOffset + I_Alignment - 1) & ~(I_Alignment - 1);
            std::size_t RequiredSize = AlignedOffset + I_Bytes;

            // Check if we can fit in current buffer
            if (CurrentBuffer && RequiredSize <= BufferSize)
            {
                void* Result = CurrentBuffer + AlignedOffset;
                CurrentOffset = RequiredSize;
                return Result;
            }

            // Need to allocate overflow block
            // Calculate padding needed to align data portion
            std::size_t BlockHeaderSize = sizeof(Block);
            std::size_t BlockAlignment = (alignof(Block) > I_Alignment) ? alignof(Block) : I_Alignment;
            
            // Calculate padding to align data after block header
            std::size_t DataOffset = (BlockHeaderSize + I_Alignment - 1) & ~(I_Alignment - 1);
            std::size_t TotalSize = DataOffset + I_Bytes;
            
            // Allocate with block alignment (ensures Block* is aligned)
            Block* NewBlock = static_cast<Block*>(
                Upstream->allocate(TotalSize, BlockAlignment));
            
            if (!NewBlock)
            {
                throw std::bad_alloc();
            }

            NewBlock->Next = OverflowBlocks;
            NewBlock->Size = I_Bytes;
            NewBlock->Alignment = I_Alignment;
            OverflowBlocks = NewBlock;

            // Return aligned data pointer
            std::byte* BlockBytes = reinterpret_cast<std::byte*>(NewBlock);
            return BlockBytes + DataOffset;
        }

        void do_deallocate(void* /*I_Ptr*/, std::size_t /*I_Bytes*/, std::size_t /*I_Alignment*/) override
        {
            // Monotonic allocator: deallocate is a no-op
            // Memory is freed all at once via release()
        }

        bool do_is_equal(const std::pmr::memory_resource& I_Other) const noexcept override
        {
            return this == &I_Other;
        }

        std::byte* CurrentBuffer;
        std::size_t BufferSize;
        std::size_t CurrentOffset;
        std::pmr::memory_resource* Upstream;
        Block* OverflowBlocks{ nullptr };
    };

    // A monotonic (bump) arena with custom implementation.
    //
    // Features:
    // - Fast O(1) allocations
    // - No per-allocation frees (Deallocate is effectively ignored)
    // - Reset() releases *all* allocations at once
    // - Optional inline buffer for zero-allocation small usage
    // - Automatic spill to upstream when inline buffer exhausted
    // - Full PMR compatibility for standard containers
    //
    // Thread Safety:
    // - NOT thread-safe. Use one arena per thread or external synchronization.
    //
    // Example:
    //   TMemoryArena arena;
    //   void* ptr = arena.Allocate(1024, 16);
    //   std::pmr::vector<int> vec{arena.GetAllocator<int>()};
    //   arena.Reset();  // Releases all allocations
    template<UInt64 InlineBytes>
    class VISERA_CORE_API TMonotonicArena
    {
    public:
        using FResourceType = FMonotonicBufferResource;

        // ========================================================================
        // PMR Interface (for standard library containers)
        // ========================================================================

        /// Access the underlying PMR memory_resource for PMR containers
        [[nodiscard]] std::pmr::memory_resource&
        Get() noexcept { return Resource; }

        [[nodiscard]] const std::pmr::memory_resource&
        Get() const noexcept { return Resource; }

        /// Create a polymorphic allocator bound to this arena
        template<class T>
        [[nodiscard]] std::pmr::polymorphic_allocator<T>
        GetAllocator() noexcept
        {
            return std::pmr::polymorphic_allocator<T>{ Get() };
        }

        // ========================================================================
        // Direct Allocation Interface (convenience methods)
        // ========================================================================

        /// Allocate memory with specified size and alignment
        /// @param I_Size Bytes to allocate
        /// @param I_Alignment Alignment requirement (must be power of 2)
        /// @return Pointer to allocated memory, or nullptr on failure
        [[nodiscard]] void*
        Allocate(UInt64 I_Size, UInt64 I_Alignment = alignof(std::max_align_t)) noexcept
        {
            VISERA_ASSERT(Math::IsPowerOfTwo(I_Alignment));
            try
            { return Resource.allocate(I_Size, I_Alignment); }
            catch (const std::bad_alloc&)
            { return nullptr; }
        }

        // ========================================================================
        // Memory Management
        // ========================================================================

        /// Release all allocations made from this arena (including spill blocks).
        /// After Reset(), the arena can be reused.
        /// @note All pointers previously returned by Allocate() become invalid
        void
        Reset() noexcept
        {
            Resource.release();
        }

        // ========================================================================
        // Configuration
        // ========================================================================

        /// Get the inline capacity (bytes) reserved inside this arena object
        static constexpr UInt64
        GetInlineCapacityBytes() noexcept
        {  return InlineBytes; }

        /// Get the upstream memory resource
        [[nodiscard]] std::pmr::memory_resource*
        GetUpstream() const noexcept
        {
            return Resource.GetUpstream();
        }

        // ========================================================================
        // Construction / Destruction
        // ========================================================================

        /// Construct arena with inline buffer, using default upstream allocator
        explicit
        TMonotonicArena(
            std::pmr::memory_resource* I_Upstream = std::pmr::get_default_resource()) noexcept
            : Inline{}
            , Resource{ Inline, sizeof(Inline), I_Upstream }
        {}

        /// Construct arena with external buffer
        /// @param I_Buffer External buffer to use (must outlive the arena)
        /// @param I_Bytes Size of external buffer
        /// @param I_Upstream Upstream allocator for spill blocks
        TMonotonicArena(
            void* I_Buffer,
            UInt64 I_Bytes,
            std::pmr::memory_resource* I_Upstream = std::pmr::get_default_resource()) noexcept
            : Resource{ I_Buffer, static_cast<std::size_t>(I_Bytes), I_Upstream }
        {}

        // Non-copyable, non-movable (arena owns its buffer)
        TMonotonicArena(const TMonotonicArena&) = delete;
        TMonotonicArena& operator=(const TMonotonicArena&) = delete;
        TMonotonicArena(TMonotonicArena&&) = delete;
        TMonotonicArena& operator=(TMonotonicArena&&) = delete;

        /// Destructor automatically releases all allocations
        ~TMonotonicArena()
        {
            Resource.release();
        }

    private:
        // Inline buffer with proper alignment for most allocations
        alignas(std::max_align_t)
        std::byte     Inline[InlineBytes > 0 ? InlineBytes : 1]{};
        FResourceType Resource;
    };
}
