module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Types.Shader;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FRHIShader
    {
    public:
        enum class EStage { Unknown, Vertex, Fragment };

        [[nodiscard]] inline EStage
        GetStage()      const { return Stage; }
        [[nodiscard]] inline UInt64
        GetSize()       const { return ShaderCode.size(); }
        [[nodiscard]] inline FStringView
        GetEntryPoint() const { return EntryPoint; }
        [[nodiscard]] inline const TArray<FByte>&
        GetShaderCode() const { return ShaderCode; }

        [[nodiscard]] inline Bool
        IsEmpty()           const { return ShaderCode.empty(); }
        [[nodiscard]] inline Bool
        IsVertexShader()    const { return Stage == EStage::Vertex; }
        [[nodiscard]] inline Bool
        IsFragmentShader()  const { return Stage == EStage::Fragment; }

    private:
        EStage        Stage        {EStage::Unknown};
        FString       EntryPoint;
        TArray<FByte> ShaderCode;

    public:
        FRHIShader() = delete;
        FRHIShader(EStage               I_Stage,
                     FStringView          I_EntryPoint,
                     const TArray<FByte>& I_ShaderCode);
        ~FRHIShader() = default;
    };

    FRHIShader::
    FRHIShader(EStage               I_Stage,
                 FStringView          I_EntryPoint,
                 const TArray<FByte>& I_ShaderCode)
    : Stage       (I_Stage),
      EntryPoint (I_EntryPoint),
      ShaderCode (I_ShaderCode)
    {
        VISERA_ASSERT(Stage != EStage::Unknown && !IsEmpty());
    }
}