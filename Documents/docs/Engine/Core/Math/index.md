# Core.Math (Visera.Core.Math)

**Core.Math** provides math and geometry for the engine: linear algebra (vector, matrix, quaternion), arithmetic and intervals, geometry (area, AABB, point, rotation), color (common and linear), constants, hash (CityHash, GoldenRatio), random (RNG, PCG, Seed), trigonometry (degree and radian), bit ops, interpolation, and convolution kernels (e.g. Gaussian). All types and functions are under `Visera`; used with [Types](../Types/index.md), [Image](../Image/index.md), etc.

## Responsibilities
- **Algebra**: Vectors (2/3/4D etc.), matrix, quaternion for transform, camera, physics.
- **Geometry**: Area, AABB, point, rotation representation and conversion.
- **Color**: sRGB and linear color, common formats for rendering and [Image.Pixel](../Image/Pixel.md).
- **Random and hash**: PCG RNG, seed type, CityHash/GoldenRatio hash for randomization and hash tables.
- **Other**: Math constants, trigonometry (degree/radian types), bit ops, interpolation (lerp, smoothstep, etc.), Gaussian kernel.

## Submodules
| Module | Description |
|------|------|
| [Algebra](Algebra/index.md) | [Vector](Algebra/Vector.md), [Matrix](Algebra/Matrix.md), [Quaternion](Algebra/Quaternion.md). |
| [Arithmetic](Arithmetic/index.md) | Arithmetic, [Interval](Arithmetic/Interval.md), [Operation](Arithmetic/Operation.md). |
| [Geometry](Geometry/index.md) | [Area](Geometry/Area.md), [Box](Geometry/Box.md), [Point](Geometry/Point.md), [Rotation](Geometry/Rotation.md). |
| [Color](Color/index.md) | [Common](Color/Common.md), [Linear](Color/Linear.md). |
| [Hash](Hash/index.md) | [CityHash](Hash/CityHash.md), [GoldenRatio](Hash/GoldenRatio.md). |
| [Random](Random/index.md) | [RNG](Random/RNG.md), [Seed](Random/Seed.md), [PCG](Random/RNG/PCG.md). |
| [Trigonometry](Trigonometry/index.md) | [Degree](Trigonometry/Degree.md), [Radian](Trigonometry/Radian.md). |
| [Bit](Bit.md), [Constants](Constants.md), [Interpolation](Interpolation.md), [Kernel](Kernal/index.md) | Bit ops, constants, interpolation, convolution kernel. |

## See also
- [Core](../index.md) — Parent module
- [Image](../Image/index.md) — Image and pixel use color and vector
- [Runtime.Graphics](../../Runtime/Graphics/index.md) — Rendering uses matrix and quaternion
