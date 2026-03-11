module;
#include <Visera-Core.hpp>
export module Visera.Core.Indexing.SpatialHash;
#define VISERA_MODULE_NAME "Core.Indexing"
import Visera.Core.Containers.Array;
import Visera.Core.Math.Arithmetic.Operation;
import Visera.Core.Math.Algebra.Vector;
import Visera.Core.Math.Geometry.Box;
import Visera.Core.Math.Geometry.Circle;
import Visera.Core.Math.Geometry.Intersection;
import Visera.Core.Algorithm.Ranges;

export namespace Visera
{
	/**
	 * Fixed-size hashed bucket broadphase for 2D point-like entities.
	 * DOD, zero runtime allocation, cache-friendly (e.g. Vampire Survivors-style).
	 * Call Clear() each frame, then Insert() all entities.
	 *
	 * Iteration returns bucket candidates, not exact cell contents. Multiple grid cells
	 * hash to the same bucket; GetFirstInCell / GetNext iterate the bucket. QueryRange
	 * may invoke the callback multiple times for the same entity and may include entities
	 * from colliding cells. Callers must post-filter (e.g. distance or AABB) and
	 * deduplicate (e.g. frame stamp or visited set by FEntityID) as needed.
	 */
	class VISERA_CORE_API FSpatialHash2D
	{
	public:
		using FEntityID = UInt32;
		static constexpr FEntityID InvalidID = 0xFFFFFFFFu;

		FSpatialHash2D(Float I_CellSize, UInt32 I_NumBuckets, UInt32 I_MaxEntities);

		void
        Clear();
		void
        Insert(FEntityID I_EntityID, const FVector2F& I_Position);
        [[nodiscard]] Bool
        IsValidEntityID(FEntityID I_EntityID) const;
		[[nodiscard]] FEntityID
        GetFirstInCell(const FVector2F& I_Position) const;
		[[nodiscard]] FEntityID
        GetNext(FEntityID I_Current) const;
		template<typename Callback> requires Concepts::Callable<Callback, void, FEntityID> void
		QueryRange(const FBox2F& I_Box, Callback&& I_Callback) const
		{
			const Int32 MinGridX = Math::Floor(I_Box.Min.X * InverseCellSize);
			const Int32 MaxGridX = Math::Floor(I_Box.Max.X * InverseCellSize);
			const Int32 MinGridY = Math::Floor(I_Box.Min.Y * InverseCellSize);
			const Int32 MaxGridY = Math::Floor(I_Box.Max.Y * InverseCellSize);
			for (Int32 Gy = MinGridY; Gy <= MaxGridY; ++Gy)
			{
				for (Int32 Gx = MinGridX; Gx <= MaxGridX; ++Gx)
				{
					FEntityID Id = Heads[GetHashFromGrid(Gx, Gy)];
					while (Id != InvalidID)
					{
						I_Callback(Id);
						Id = GetNext(Id);
					}
				}
			}
		}
		template<typename Callback> requires Concepts::Callable<Callback, void, FEntityID> void
		QueryRange(const FCircle2F& I_Circle, Callback&& I_Callback) const
		{
			const Int32 MinGridX = Math::Floor((I_Circle.Center.X - I_Circle.Radius) * InverseCellSize);
			const Int32 MaxGridX = Math::Floor((I_Circle.Center.X + I_Circle.Radius) * InverseCellSize);
			const Int32 MinGridY = Math::Floor((I_Circle.Center.Y - I_Circle.Radius) * InverseCellSize);
			const Int32 MaxGridY = Math::Floor((I_Circle.Center.Y + I_Circle.Radius) * InverseCellSize);
			for (Int32 Gy = MinGridY; Gy <= MaxGridY; ++Gy)
			{
				for (Int32 Gx = MinGridX; Gx <= MaxGridX; ++Gx)
				{
					const FBox2F CellBox{
						FVector2F{ Gx * CellSize, Gy * CellSize },
						FVector2F{ (Gx + 1) * CellSize, (Gy + 1) * CellSize }
					};
					if (!Overlaps(CellBox, I_Circle))
						continue;
					FEntityID Id = Heads[GetHashFromGrid(Gx, Gy)];
					while (Id != InvalidID)
					{
						I_Callback(Id);
						Id = GetNext(Id);
					}
				}
			}
		}

	private:
		static Float ValidatedCellSize(Float I_CellSize);
		static UInt32 ValidatedNumBuckets(UInt32 I_NumBuckets);

		[[nodiscard]] UInt32 GetHash(const FVector2F& I_Position) const;
		[[nodiscard]] UInt32 GetHashFromGrid(Int32 I_GridX, Int32 I_GridY) const;

        Float CellSize;
		Float InverseCellSize;
		UInt32 BucketMask;
		TArray<UInt32> Heads;
		TArray<UInt32> Nexts;
	};

	inline Float FSpatialHash2D::ValidatedCellSize(Float I_CellSize)
	{
		VISERA_ASSERT(I_CellSize > 0.0f && "CellSize must be positive");
		return I_CellSize;
	}

	inline UInt32 FSpatialHash2D::ValidatedNumBuckets(UInt32 I_NumBuckets)
	{
		VISERA_ASSERT(I_NumBuckets > 0u && (I_NumBuckets & (I_NumBuckets - 1u)) == 0u && "NumBuckets must be power of two");
		return I_NumBuckets;
	}

	inline FSpatialHash2D::FSpatialHash2D(Float I_CellSize, UInt32 I_NumBuckets, UInt32 I_MaxEntities)
		: CellSize       (ValidatedCellSize(I_CellSize))
		, InverseCellSize(1.0f / CellSize)
		, BucketMask     (ValidatedNumBuckets(I_NumBuckets) - 1u)
		, Heads          (I_NumBuckets,  InvalidID)
		, Nexts          (I_MaxEntities, InvalidID)
	{}

	VISERA_FORCEINLINE void FSpatialHash2D::Clear()
	{
		Algorithm::Fill(Heads, InvalidID);
	}

	VISERA_FORCEINLINE void FSpatialHash2D::Insert(FEntityID I_EntityID, const FVector2F& I_Position)
	{
		VISERA_ASSERT(I_EntityID < Nexts.GetSize() && "EntityID out of range");
		if (I_EntityID >= Nexts.GetSize())
			return;
		const UInt32 HashIndex = GetHash(I_Position);
		Nexts[I_EntityID] = Heads[HashIndex];
		Heads[HashIndex] = I_EntityID;
	}

    VISERA_FORCEINLINE Bool FSpatialHash2D::IsValidEntityID(FEntityID I_EntityID) const
	{
		return I_EntityID < Nexts.GetSize();
	}

	[[nodiscard]] VISERA_FORCEINLINE FSpatialHash2D::FEntityID FSpatialHash2D::GetFirstInCell(const FVector2F& I_Position) const
	{
		return Heads[GetHash(I_Position)];
	}

	[[nodiscard]] VISERA_FORCEINLINE FSpatialHash2D::FEntityID FSpatialHash2D::GetNext(FEntityID I_Current) const
	{
        return I_Current >= Nexts.GetSize() ? InvalidID : Nexts[I_Current];
	}

	[[nodiscard]] VISERA_FORCEINLINE UInt32 FSpatialHash2D::GetHash(const FVector2F& I_Position) const
	{
		const Int32 GridX = Math::Floor(I_Position.X * InverseCellSize);
		const Int32 GridY = Math::Floor(I_Position.Y * InverseCellSize);
		return GetHashFromGrid(GridX, GridY);
	}

	[[nodiscard]] VISERA_FORCEINLINE UInt32 FSpatialHash2D::GetHashFromGrid(Int32 I_GridX, Int32 I_GridY) const
	{
		const UInt32 Hash = (static_cast<UInt32>(I_GridX) * 73856093u) ^ (static_cast<UInt32>(I_GridY) * 19349663u);
		return Hash & BucketMask;
	}
}
