module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Object;
#define VISERA_MODULE_NAME "Runtime.Object"
export import Visera.Runtime.Object.Component;
       import Visera.Core.Attribute;
       import Visera.Core.Log;
       import Visera.Core.Types.Name;
       import Visera.Core.Math.Arithmetic;

namespace Visera
{
    export class VISERA_RUNTIME_API VObject
        : public Attribute::SharedOnly<VObject>
    {
    public:
        [[nodiscard]] const FName&
        GetID() const { return ID; }

    private:
        FName                          ID;
        TArray<TSharedPtr<VComponent>> Components;

        TWeakPtr<VObject>              Parent;
        TArray<TSharedPtr<VObject>>    Children;

    protected:
        explicit VObject() : ID{EName::Object}
        { VISERA_ASSERT(ID.GetIdentifier() != Math::UpperBound<UInt32>()); }

    public:
        virtual ~VObject() = default;
    };
}