module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Object;
#define VISERA_MODULE_NAME "Runtime.Object"
export import Visera.Runtime.Object.Component;
       import Visera.Core.Log;
       import Visera.Core.Types.Name;

namespace Visera
{
    export class VISERA_RUNTIME_API VObject
    {
    public:
        [[nodiscard]] const FName&
        GetName() const { return Name; }
        [[nodiscard]] UInt64
        GetID() const { return Name.GetIdentifier(); }

    private:
        FName                          Name;
        TArray<TSharedPtr<VComponent>> Components;

        TWeakPtr<VObject>              Parent;
        TArray<TSharedPtr<VObject>>    Children;

    public:
        explicit VObject(const FName& I_Name = FName(EName::Object))
            : Name{I_Name}
        {  }
        
        virtual ~VObject() = default;

        // Prevent copying to avoid object identity issues
        VObject(const VObject&) = delete;
        VObject& operator=(const VObject&) = delete;
        
        // Allow moving
        VObject(VObject&&) = default;
        VObject& operator=(VObject&&) = default;

        Bool operator==(const VObject& Other) const
        { return Name == Other.Name; }
    };
}