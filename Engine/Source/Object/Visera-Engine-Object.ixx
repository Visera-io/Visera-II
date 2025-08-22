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
        [[nodiscard]] virtual FStringView
        GetDebugName() const { return "unknown"; };

        VObject()
        :
        Name{ EName::Object }
        {

        }
        virtual ~VObject() {};

    private:
        FName Name;
    };
}