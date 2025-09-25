module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Window.Interface;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Core.Types.Text;

namespace Visera
{
    export class VISERA_RUNTIME_API IWindow : public IGlobalSingleton
    {
        Bool bInitialized = False;
    public:
        enum class EType
        {
            Unknown,
            GLFW
        };

        [[nodiscard]] virtual void*
        GetHandle() const = 0;
        [[nodiscard]] virtual Bool
        ShouldClose() const = 0;
        virtual void
        WaitEvents() const  = 0;
        virtual void
        PollEvents() const  = 0;
        virtual void
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) = 0;
        virtual void
        SetPosition(Int32 I_X, Int32 I_Y) const = 0;

        [[nodiscard]] inline UInt32
        GetWidth() const  { return Width; }
        [[nodiscard]] inline UInt32
        GetHeight() const { return Height; }
        [[nodiscard]] EType
        GetType() const { return Type; }
        [[nodiscard]] const FText&
        GetTitle() const { return Title; }
        virtual void inline
        SetTitle(const FText& I_Title) { Title = I_Title; }
        [[nodiscard]] inline Bool
        IsMaximized() const { return bMaximized; };

        explicit IWindow() = delete;
        explicit IWindow(EType I_Type) : IGlobalSingleton("Window"), Type(I_Type) {}

    protected:
        EType Type   = EType::Unknown;
        FText       Title  = TEXT("Visera");
        Int32       Width {900},  Height{600};
        Float       ScaleX{1.0f}, ScaleY{1.0f};
        Bool        bMaximized{False};
    };
}
