module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Object;
#define VISERA_MODULE_NAME "Engine.Object"
import Visera.Core.Log;
import Visera.Core.Types.Name;

export namespace Visera
{
    class VISERA_ENGINE_API VObject
    {
    public:
        VObject() : Name{ EName::Object }
        {
            LOG_DEBUG("{}", Name);
        }
        virtual ~VObject() {};

    private:
        FName Name;
    };
}