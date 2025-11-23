module;
#include <Visera-Core.hpp>
#if defined(VISERA_ON_X86_CPU)
#include <xmmintrin.h>
#elif defined(VISERA_ON_ARM_CPU)
#include <arm_acle.h>
#endif
export module Visera.Core.OS.Memory;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.Log;
import Visera.Core.Math.Bit;

export namespace Visera
{
    namespace Concepts
    {
        template<typename T> concept 
        Alignable = std::integral<T> || std::is_pointer_v<T>;
    }

    namespace Memory
    {
        inline auto
        Memset(void* I_Memory, Int32 I_Value, UInt64 I_Size) -> void;
        inline auto
        Memcpy(void* I_Destination, const void* I_Source, UInt64 I_Size) -> void { std::memcpy(I_Destination, I_Source, I_Size); }
        [[nodiscard]] inline auto
        Memcmp(const void* I_MemA, const void* I_MemB, UInt64 I_Size) { return std::memcmp(I_MemA, I_MemB, I_Size); }
        [[nodiscard]] inline void*
        Malloc(UInt64 I_Size, UInt32 I_Alignment);
        [[nodiscard]] inline void*
        MallocNow(UInt64 I_Size, UInt32 I_Alignment, Int32 I_Value = 0) { void* AllocatedMemory = Malloc(I_Size, I_Alignment); Memset(AllocatedMemory, I_Value, I_Size); return AllocatedMemory; }
        [[nodiscard]] inline void*
        Realloc(void* I_Memory, UInt64 I_OldSize, UInt32 I_OldAlignment, UInt64 I_NewSize, UInt32 I_NewAlignment);
        inline auto
        Free(void* I_Memory, UInt32 I_Alignment) -> void;

        inline auto
        Prefetch(const void* I_Memory, UInt32 I_Offset = 0) -> void;

        [[nodiscard]] inline Bool
        IsValidAllocation(UInt64 I_Size, UInt32 I_Alignment);
        [[nodiscard]] inline Bool
        IsZero(const void* I_Memory, UInt64 I_Size);

        /**Example: VE::Memory::GetDataOffset(&Foo::bar);*/
        template <class Structure, typename MemeberType> [[nodiscard]] constexpr UInt64
        GetDataOffset(MemeberType Structure::* Member) { static_assert(std::is_standard_layout_v<Structure>, "Structure MUST be a standard layout type!"); return reinterpret_cast<UInt64>(&(reinterpret_cast<Structure*>(NULL)->*Member)); }

        /**Aligns a value to the nearest higher multiple of 'Alignment', which must be a power of two.*/
        template <Concepts::Alignable T> [[nodiscard]] constexpr T
        Align(T I_Value, UInt64 I_Alignment) { VISERA_ASSERT(Math::IsPowerOfTwo(I_Alignment)); return (T)(((UInt64)I_Value + I_Alignment - 1) & ~(I_Alignment - 1)); };
    }

    // << Implementation >>
    namespace Memory
    {
        void*
        Malloc(UInt64 I_Size, UInt32 I_Alignment)
        {
            VISERA_ASSERT(IsValidAllocation(I_Size, I_Alignment));

            void* AllocatedMemory = nullptr;
            if (I_Alignment)
            {
    #if defined(VISERA_ON_WINDOWS_SYSTEM)
                AllocatedMemory = _aligned_malloc(I_Size, I_Alignment);
    #elif defined(VISERA_ON_APPLE_SYSTEM)
                posix_memalign(&AllocatedMemory, I_Alignment, I_Size);
    #else
                AllocatedMemory = std::aligned_alloc(I_Alignment, 8);
    #endif
            }
            else AllocatedMemory = std::malloc(I_Size);

            if (!AllocatedMemory)
            {
                LOG_ERROR("Failed to allocate memory!"
                            "(size: {}, alignment: {})",
                          I_Size, I_Alignment);
            }
            return AllocatedMemory;
        }

        void
        Memset(void* I_Memory, Int32 I_Value, UInt64 I_Size)
        {
            memset(I_Memory, I_Value, I_Size);
        }

        void*
        Realloc(void* I_Memory, UInt64 I_OldSize, UInt32 I_OldAlignment, UInt64 I_NewSize, UInt32 I_NewAlignment)
        {
            VISERA_ASSERT(IsValidAllocation(I_NewSize, I_NewAlignment));

            void* ReallocatedMemory = nullptr;
            if (I_NewAlignment)
            {
    #if defined(VISERA_ON_WINDOWS_SYSTEM)
                ReallocatedMemory = _aligned_realloc(I_Memory, I_NewSize, I_NewAlignment);
    #else
                ReallocatedMemory = Malloc(I_NewSize, I_NewAlignment);
                Memcpy(ReallocatedMemory, I_Memory, std::min(I_OldSize, I_NewSize));
                Free(I_Memory, I_OldAlignment);
    #endif
            }
            else ReallocatedMemory = std::realloc(I_Memory, I_NewSize);

            if (!ReallocatedMemory)
            {
                LOG_ERROR("Failed to re-allocate memory! "
                    "(memory:{}, from [{},{}] to [{}, {}]).",
                      (void*)(I_Memory),
                      I_OldSize, I_OldAlignment, I_NewSize, I_NewAlignment);
            }
            return ReallocatedMemory;
        }

        void
        Free(void* I_Memory, UInt32 I_Alignment)
        {
            if (!I_Memory) { return; }

            if (I_Alignment)
            {
                VISERA_ASSERT(Math::IsPowerOfTwo(I_Alignment));
    #if defined(VISERA_ON_WINDOWS_SYSTEM)
                _aligned_free(I_Memory);
    #else
                std::free(I_Memory); // Safe
    #endif
            }
            else std::free(I_Memory);
        }

        void
        Prefetch(const void* I_Memory, UInt32 I_Offset/* = 0*/)
        {
            //Copied from Unreal Engine!
    #if defined(VISERA_ON_X86_CPU)
            _mm_prefetch(static_cast<const char*>(I_Memory) + I_Offset, _MM_HINT_T0);
    #elif defined(VISERA_ON_ARM_CPU)
    #	if defined(_MSC_VER)
            _prefetch(static_cast<const char*>(I_Memory) + I_Offset);
    #	else
            VISERA_WIP;
            //__asm__ I__volatile__("prfm pldl1keep, [%[ptr]]\n" ::[ptr] "r"(Ptr) : );
    #	endif
    #else
        VISERA_WIP
    #endif
        }

        Bool
        IsValidAllocation(UInt64 I_Size, UInt32 I_Alignment)
        {
            return I_Size
                    && (!I_Alignment
                        ||
                        (Math::IsPowerOfTwo(I_Alignment)
                        && (I_Size % I_Alignment == 0))
    #if defined(VISERA_ON_APPLE_SYSTEM)
                        && (I_Alignment % sizeof(void*) == 0)
    #endif
            );
        }

        Bool
        IsZero(const void* I_Memory, UInt64 I_Size)
        {
            FByte* Start = (FByte*)(I_Memory);
            FByte* End = Start + I_Size;
            while (Start < End)
            {  if ((*Start++) != 0) return False; }
            return True;
        }
    }
}
