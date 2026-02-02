module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Memory;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.Math.Bit;
import :Arena;

export namespace Visera
{
    namespace Concepts
    {
        template<typename T> concept 
        Alignable = std::integral<T> || std::is_pointer_v<T>;
    }

    [[nodiscard]] constexpr UInt64
    operator""_KB(unsigned long long I_Value) noexcept
    { return static_cast<UInt64>(I_Value * 1024); }

    [[nodiscard]] constexpr UInt64
    operator""_MB(unsigned long long I_Value) noexcept
    { return static_cast<UInt64>(I_Value * 1024 * 1024); }

    [[nodiscard]] constexpr UInt64
    operator""_GB(unsigned long long I_Value) noexcept
    { return static_cast<UInt64>(I_Value * 1024ULL * 1024 * 1024); }

    namespace Memory
    {
        template<UInt64 InlineBytes>
        using TMonotonicArena = TMonotonicArena<InlineBytes>;

        VISERA_FORCEINLINE auto
        Memset(void* I_Memory, Int32 I_Value, UInt64 I_Size) -> void;
        VISERA_FORCEINLINE auto
        Memcpy(void* I_Destination, const void* I_Source, UInt64 I_Size) -> void { std::memcpy(I_Destination, I_Source, I_Size); }
        [[nodiscard]] VISERA_FORCEINLINE auto
        Memcmp(const void* I_MemA, const void* I_MemB, UInt64 I_Size) { return std::memcmp(I_MemA, I_MemB, I_Size); }
        [[nodiscard]] VISERA_FORCEINLINE void*
        Malloc(UInt64 I_Size, UInt32 I_Alignment);
        [[nodiscard]] VISERA_FORCEINLINE void*
        MallocNow(UInt64 I_Size, UInt32 I_Alignment, Int32 I_Value = 0) { void* AllocatedMemory = Malloc(I_Size, I_Alignment); Memset(AllocatedMemory, I_Value, I_Size); return AllocatedMemory; }
        [[nodiscard]] VISERA_FORCEINLINE void*
        Realloc(void* I_Memory, UInt64 I_OldSize, UInt32 I_OldAlignment, UInt64 I_NewSize, UInt32 I_NewAlignment);
        VISERA_FORCEINLINE auto
        Free(void* I_Memory, UInt32 I_Alignment) -> void;

        [[nodiscard]] VISERA_FORCEINLINE Bool
        IsValidAllocation(UInt64 I_Size, UInt32 I_Alignment);
        [[nodiscard]] VISERA_FORCEINLINE Bool
        IsZero(const void* I_Memory, UInt64 I_Size);
        template<typename T> [[nodiscard]] Bool
        IsZero(const T& I_Object) { return IsZero(&I_Object, sizeof(T)); };

        /**Example: VE::Memory::GetDataOffset(&Foo::bar);*/
        template <class Structure, typename MemeberType> [[nodiscard]] constexpr UInt64
        GetDataOffset(MemeberType Structure::* Member) { static_assert(std::is_standard_layout_v<Structure>, "Structure MUST be a standard layout type!"); return reinterpret_cast<UInt64>(&(reinterpret_cast<Structure*>(NULL)->*Member)); }

        /**Aligns a value to the nearest higher multiple of 'Alignment', which must be a power of two.*/
        template <Concepts::Alignable T> [[nodiscard]] constexpr T
        Align(T I_Value, UInt64 I_Alignment) { VISERA_ASSERT(Math::IsPowerOfTwo(I_Alignment)); return (T)(((UInt64)I_Value + I_Alignment - 1) & ~(I_Alignment - 1)); };
    }


    /** An untyped array of data with compile-time alignment and size derived from another type. */
	template<typename ElementType>
	struct TTypeCompatibleBytes
	{
		using ElementTypeAlias_NatVisHelper = ElementType;

		// Trivially constructible and destuctible - users are responsible for managing the lifetime of the inner element.
		TTypeCompatibleBytes()  = default;
		~TTypeCompatibleBytes() = default;

		// Noncopyable
		TTypeCompatibleBytes(TTypeCompatibleBytes&&)				 = delete;
		TTypeCompatibleBytes(const TTypeCompatibleBytes&)			 = delete;
		TTypeCompatibleBytes& operator=(TTypeCompatibleBytes&&)		 = delete;
		TTypeCompatibleBytes& operator=(const TTypeCompatibleBytes&) = delete;

		// GetTypedPtr only exists for backwards compatibility - these functions do not exist and cannot be implemented for the reference and void specializations.
		ElementType* GetTypedPtr()
		{
			return (ElementType*)this;
		}
		const ElementType* GetTypedPtr() const
		{
			return (const ElementType*)this;
		}

		using MutableGetType = ElementType&;       // The type returned by Bytes.Get() where Bytes is a non-const lvalue
		using ConstGetType   = const ElementType&; // The type returned by Bytes.Get() where Bytes is a const lvalue
		using RvalueGetType  = ElementType&&;      // The type returned by Bytes.Get() where Bytes is an rvalue (non-const)

		// Gets the inner element - no checks are performed to ensure an element is present.
		ElementType& GetUnchecked() &
		{
			return *(ElementType*)this;
		}
		const ElementType& GetUnchecked() const&
		{
			return *(const ElementType*)this;
		}
		ElementType&& GetUnchecked() &&
		{
			return (ElementType&&)*(ElementType*)this;
		}

		// Emplaces an inner element.
		// Note: no checks are possible to ensure that an element isn't already present.  DestroyUnchecked() must be called to end the element's lifetime.
		template <typename... ArgTypes>
		void EmplaceUnchecked(ArgTypes&&... Args)
		{
			new ((void*)GetTypedPtr()) ElementType((ArgTypes&&)Args...);
		}

		// Destroys the inner element.
		// Note: no checks are possible to ensure that there is an element already present.
		void DestroyUnchecked()
		{
			ElementTypeAlias_NatVisHelper* Ptr = (ElementTypeAlias_NatVisHelper*)this;
			Ptr->ElementTypeAlias_NatVisHelper::~ElementTypeAlias_NatVisHelper();
		}

		alignas(ElementType) FByte Pad[sizeof(ElementType)];
	};

	template <typename T>
	struct TTypeCompatibleBytes<T&>
	{
		using ElementTypeAlias_NatVisHelper = T&;

		// Trivially constructible and destuctible - users are responsible for managing the lifetime of the inner element.
		TTypeCompatibleBytes()  = default;
		~TTypeCompatibleBytes() = default;

		// Noncopyable
		TTypeCompatibleBytes(TTypeCompatibleBytes&&)				 = delete;
		TTypeCompatibleBytes(const TTypeCompatibleBytes&)			 = delete;
		TTypeCompatibleBytes& operator=(TTypeCompatibleBytes&&)		 = delete;
		TTypeCompatibleBytes& operator=(const TTypeCompatibleBytes&) = delete;

		using MutableGetType = T&; // The type returned by Bytes.Get() where Bytes is a non-const lvalue
		using ConstGetType   = T&; // The type returned by Bytes.Get() where Bytes is a const lvalue
		using RvalueGetType  = T&; // The type returned by Bytes.Get() where Bytes is an rvalue (non-const)

		// Gets the inner element - no checks are performed to ensure an element is present.
		T& GetUnchecked() const
		{
			return *Ptr;
		}

		// Emplaces an inner element.
		// Note: no checks are possible to ensure that an element isn't already present.  DestroyUnchecked() must be called to end the element's lifetime.
		void EmplaceUnchecked(T& Ref)
		{
			Ptr = &Ref;
		}

		// Destroys the inner element.
		// Note: no checks are possible to ensure that there is an element already present.
		void DestroyUnchecked()
		{
		}

		T* Ptr;
	};

	template <>
	struct TTypeCompatibleBytes<void>
	{
		using ElementTypeAlias_NatVisHelper = void;

		// Trivially constructible and destuctible - users are responsible for managing the lifetime of the inner element.
		TTypeCompatibleBytes()  = default;
		~TTypeCompatibleBytes() = default;

		// Noncopyable
		TTypeCompatibleBytes(TTypeCompatibleBytes&&)				 = delete;
		TTypeCompatibleBytes(const TTypeCompatibleBytes&)			 = delete;
		TTypeCompatibleBytes& operator=(TTypeCompatibleBytes&&)		 = delete;
		TTypeCompatibleBytes& operator=(const TTypeCompatibleBytes&) = delete;

		using MutableGetType = void; // The type returned by Bytes.Get() where Bytes is a non-const lvalue
		using ConstGetType   = void; // The type returned by Bytes.Get() where Bytes is a const lvalue
		using RvalueGetType  = void; // The type returned by Bytes.Get() where Bytes is an rvalue (non-const)

		// Gets the inner element - no checks are performed to ensure an element is present.
		void GetUnchecked() const
		{
		}

		// Emplaces an inner element.
		// Note: no checks are possible to ensure that an element isn't already present.  DestroyUnchecked() must be called to end the element's lifetime.
		void EmplaceUnchecked()
		{
		}

		// Destroys the inner element.
		// Note: no checks are possible to ensure that there is an element already present.
		void DestroyUnchecked()
		{
		}
	};

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

            VISERA_ASSERT(AllocatedMemory);
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

            VISERA_ASSERT(ReallocatedMemory);
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
            auto Start = static_cast<const FByte*>(I_Memory);
            auto End   = Start + I_Size;
            while (Start < End)
            {  if ((*Start++) != 0) return False; }
            return True;
        }
    }
}
