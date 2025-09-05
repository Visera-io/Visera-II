module;
#include <slang.h>
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Platform.MacOS;
#define VISERA_MODULE_NAME "Runtime.Platform"
import Visera.Runtime.Platform.Interface;

namespace Visera
{
    export class VISERA_RUNTIME_API FMacOSPlatform : public IPlatform
    {
    public:
        ~FMacOSPlatform() override = default;
    };
}