module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.Texture2D;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API ITexture
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

    public:
        ITexture()                          = default;
        virtual ~ITexture()                 = default;
        ITexture(const ITexture&)            = delete;
        ITexture& operator=(const ITexture&) = delete;
    };
}