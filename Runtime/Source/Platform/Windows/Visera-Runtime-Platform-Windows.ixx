module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Platform.Windows;
#define VISERA_MODULE_NAME "Runtime.Platform"
import Visera.Runtime.Platform.Interface;

namespace Visera
{
    export class VISERA_RUNTIME_API FWindowsPlatform : public IPlatform
    {
    public:
        ~FWindowsPlatform() override = default;
    };
}