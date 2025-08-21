module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Object;
#define VISERA_MODULE_NAME "Runtime.Object"
import Visera.Core.Log;

export namespace Visera
{
    class VObject
    {
    public:
        VObject() { LOG_DEBUG("{}", typeid(this).name()); }
        virtual ~VObject() {};
    };
}