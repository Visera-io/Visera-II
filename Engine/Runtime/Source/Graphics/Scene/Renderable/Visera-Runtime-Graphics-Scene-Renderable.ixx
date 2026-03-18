module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Graphics.Scene.Renderable;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Core.Math.Geometry.Transform;
export import Visera.Core.Math.Algebra.Vector;
       import Visera.Runtime.RHI;
       import Visera.Runtime.Graphics.Material;
       import Visera.Core.Types.Pointer;

export namespace Visera
{
    /** Unified per-instance data for 2D sprites and 3D meshes.
     *  80 bytes, std430 compatible -- matches the Slang InstanceData struct. */
    struct VISERA_RUNTIME_API FInstanceData
    {
        FTransform3x4F Transform;   // 48 bytes -- affine model matrix
        FVector4F      Color;       // 16 bytes -- tint + alpha
        FVector4F      CustomData;  // 16 bytes -- UV rect (sprites) or user params (3D)
    };
    static_assert(sizeof(FInstanceData) == 80);
    static_assert(std::is_standard_layout_v<FInstanceData>);

    /** GPU mesh: vertex and index data stored in storage buffers.
     *  Vertex format is shader-defined (e.g. StructuredBuffer<FVertex>);
     *  PSO has no vertex input state -- all data is fetched via SSBO. */
    struct VISERA_RUNTIME_API FMesh
    {
        FRHIBufferID  VertexBuffer;
        FRHIBufferID  IndexBuffer;
        UInt32        VertexCount {0};
        UInt32        IndexCount  {0};
        ERHIIndexType IndexType   {ERHIIndexType::UInt16};
    };

    /** Extracted draw command data; used by FGraphics::Draw() and scripting bindings.
     *  Data is captured at submit time so no virtual calls cross the thread boundary. */
    struct VISERA_RUNTIME_API FRenderableMeta
    {
        FInstanceData         InstanceData;
        TSharedPtr<FMaterial>  Material;
        TSharedPtr<FMesh>      Mesh;  // nullptr = sprite quad path
    };

    class VISERA_RUNTIME_API IRenderable
    {
    public:
        [[nodiscard]] virtual FInstanceData
        GetInstanceData() const = 0;
        [[nodiscard]] virtual TSharedPtr<FMaterial>
        GetMaterial() const = 0;
        /** Override to provide mesh geometry; nullptr (default) uses the sprite quad path. */
        [[nodiscard]] virtual TSharedPtr<FMesh>
        GetMesh() const { return nullptr; }

        /** Build FRenderableMeta from this renderable; used by FGraphics::Draw(const IRenderable&). */
        [[nodiscard]] FRenderableMeta
        ToRenderableMeta() const
        {
            return FRenderableMeta{
                .InstanceData = GetInstanceData(),
                .Material     = GetMaterial(),
                .Mesh         = GetMesh(),
            };
        }

        virtual ~IRenderable() = default;
    };
}