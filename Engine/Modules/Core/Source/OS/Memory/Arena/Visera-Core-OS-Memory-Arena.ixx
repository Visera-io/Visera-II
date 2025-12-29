module;
#include <Visera-Core.hpp>
#include <memory_resource>
#include <cstddef>
export module Visera.Core.OS.Memory.Arena;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.Math.Bit;

export namespace Visera
{
    // A monotonic (bump) arena backed by std::pmr::monotonic_buffer_resource.
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
    class TMonotonicArena
    {
    public:
        using FResourceType = std::pmr::monotonic_buffer_resource;

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
            // Note: monotonic_buffer_resource doesn't expose upstream directly
            // This would require custom wrapper or tracking
            return std::pmr::get_default_resource();
        }

        // ========================================================================
        // Construction / Destruction
        // ========================================================================

        /// Construct arena with inline buffer, using default upstream allocator
        explicit TMonotonicArena(
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
            : Resource{ I_Buffer, I_Bytes, I_Upstream }
        {}

        // Non-copyable, non-movable (arena owns its buffer)
        TMonotonicArena(const TMonotonicArena&) = delete;
        TMonotonicArena& operator=(const TMonotonicArena&) = delete;
        TMonotonicArena(TMonotonicArena&&) = delete;
        TMonotonicArena& operator=(TMonotonicArena&&) = delete;

        /// Destructor automatically releases all allocations
        ~TMonotonicArena() = default;

    private:
        // Inline buffer with proper alignment for most allocations
        alignas(std::max_align_t)
        std::byte     Inline[InlineBytes > 0 ? InlineBytes : 1]{};
        FResourceType Resource;
    };
}
