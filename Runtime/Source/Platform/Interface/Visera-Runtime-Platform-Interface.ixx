module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Platform.Interface;
#define VISERA_MODULE_NAME "Runtime.Platform"

namespace Visera
{
    export class VISERA_RUNTIME_API IPlatform
    {
    public:
        virtual ~IPlatform() {}
    };
}