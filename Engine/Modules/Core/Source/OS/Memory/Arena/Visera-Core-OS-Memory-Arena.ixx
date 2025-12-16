module;
#include <Visera-Core.hpp>
#include <memory_resource>
#include <cstddef>
export module Visera.Core.OS.Memory.Arena;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    // A monotonic (bump) arena backed by std::pmr::monotonic_buffer_resource.
    // - Fast allocations
    // - No per-allocation frees (Deallocate is effectively ignored)
    // - Reset() releases *all* allocations at once
    //
    // NOTE: Not thread-safe. Prefer "one arena per thread" or external synchronization.
    template<std::size_t InlineBytes = 64 * 1024>
    class TMemoryArena
    {
    public:
        using ResourceType = std::pmr::monotonic_buffer_resource;
        // Access the underlying PMR memory_resource (for pmr::vector, polymorphic_allocator, etc.)
        [[nodiscard]] std::pmr::memory_resource&
        Get() noexcept { return Resource; }
        [[nodiscard]] const std::pmr::memory_resource&
        Get() const noexcept { return Resource; }
        // Release all allocations made from this arena (including spill blocks).
        // After Reset(), the arena can be reused.
        void inline
        Reset() noexcept { Resource.release(); }
        // Inline capacity (bytes) reserved inside this arena object.
        static constexpr std::size_t
        GetInlineCapacityBytes() noexcept { return InlineBytes; }
        // Convenience: create a polymorphic allocator bound to this arena.
        template<class T> [[nodiscard]] std::pmr::polymorphic_allocator<T>
        GetAllocator() noexcept
        { return std::pmr::polymorphic_allocator<T>{ Get() }; }

    private:
        // Make sure inline buffer alignment is safe for most allocations.
        alignas(std::max_align_t) std::byte I_Inline[InlineBytes > 0 ? InlineBytes : 1]{};
        ResourceType Resource;

    public:
        // Use inline buffer first; spill to upstream when needed.
        explicit TMemoryArena(std::pmr::memory_resource* I_Upstream = std::pmr::new_delete_resource()) noexcept
            : Resource{ I_Inline, sizeof(I_Inline), I_Upstream } {}
        // Use external buffer first; spill to upstream when needed.
        TMemoryArena(void* I_Buffer,
                     std::size_t I_Bytes,
                     std::pmr::memory_resource* upstream = std::pmr::new_delete_resource()) noexcept
        : Resource{ I_Buffer, I_Bytes, upstream } {}
        TMemoryArena(const TMemoryArena&)               = delete;
        TMemoryArena& operator=(const TMemoryArena&)    = delete;
        TMemoryArena(TMemoryArena&&)                    = delete;
        TMemoryArena& operator=(TMemoryArena&&)         = delete;
        ~TMemoryArena()                                 = default;
    };
}
