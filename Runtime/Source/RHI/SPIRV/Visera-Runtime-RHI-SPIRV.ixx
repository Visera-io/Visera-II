module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.SPIRV;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FSPIRVShader
    {
    public:
        enum class EType { Unknown, Vertex, Fragment };

        [[nodiscard]] inline UInt64
        GetSize()       const { return ShaderCode.size(); }
        [[nodiscard]] inline FStringView
        GetEntryPoint() const { return EntryPoint; }
        [[nodiscard]] inline const TArray<FByte>&
        GetShaderCode() const { return ShaderCode; }

        [[nodiscard]] inline Bool
        IsEmpty()           const { return ShaderCode.empty(); }
        [[nodiscard]] inline Bool
        IsVertexShader()    const { return Type == EType::Vertex; }
        [[nodiscard]] inline Bool
        IsFragmentShader()  const { return Type == EType::Fragment; }

    private:
        EType         Type        {EType::Unknown};
        FString       EntryPoint;
        TArray<FByte> ShaderCode;

    public:
        FSPIRVShader() = delete;
        FSPIRVShader(EType                I_Type,
                     FStringView          I_EntryPoint,
                     const TArray<FByte>& I_ShaderCode);
        ~FSPIRVShader() = default;
    };

    FSPIRVShader::
    FSPIRVShader(EType                I_Type,
                 FStringView          I_EntryPoint,
                 const TArray<FByte>& I_ShaderCode)
    : Type       (I_Type),
      EntryPoint (I_EntryPoint),
      ShaderCode (I_ShaderCode)
    {
        VISERA_ASSERT(Type != EType::Unknown && !IsEmpty());
    }
}