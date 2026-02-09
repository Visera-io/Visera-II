module;
#include <Visera-Audio.hpp>
export module Visera.Runtime.Audio.Interface;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API IAudioEngine
    {
    public:
        enum class EType
        {
            Unknown,
            Null,
            Wwise,
        };
        [[nodiscard]] inline EType
        GetType() const { return Type; }
        virtual void
        Tick(Float I_Seconds) = 0;

    private:
        EType Type {EType::Unknown};

    public:
        explicit IAudioEngine() = delete;
        explicit IAudioEngine(EType I_Type) : Type{I_Type} {}
        virtual ~IAudioEngine() = default;
    };
}
