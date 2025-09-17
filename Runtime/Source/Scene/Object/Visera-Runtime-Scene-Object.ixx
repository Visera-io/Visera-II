module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scene.Object;
#define VISERA_MODULE_NAME "Runtime.Scene"
import Visera.Runtime.Scene.ECS.Entity;
import Visera.Core.Attribute;
import Visera.Core.Log;
import Visera.Core.Types.Name;

namespace Visera
{
    export class VISERA_RUNTIME_API VObject
        : public Attribute::SharedOnly<VObject>
    {
    public:
        virtual void
        Awake()  {};
        virtual void
        Update(Float I_FrameTime) {};

        [[nodiscard]] UInt64
        GetID() const { return ID; }

        void inline
        SetName(const FName& I_Name)  { Name = I_Name; }
        [[nodiscard]] inline const FName&
        GetName() const { return Name; }

    private:
        static inline UInt64           UUID {0};
        const UInt64                   ID   {0};
        FName                          Name {FName{EName::Object}};

        TWeakPtr<VObject>              Parent;
        TArray<TSharedPtr<VObject>>    Children;

    public:
        explicit VObject(): ID { ++UUID }
        {

        }
        virtual ~VObject()
        {

        }
    };

    export namespace Concepts
    {
        template<typename T> concept
        Object = std::is_base_of_v<VObject, T>;
    }
}