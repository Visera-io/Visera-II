module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Window.Interface;
#define VISERA_MODULE_NAME "Runtime.Window"

namespace Visera
{
    struct VISERA_RUNTIME_API FMouse
    {
        struct
        {
            struct { Double X{0}, Y{0}; } Position;
            struct { Double X{0}, Y{0}; } Offset;
        } Cursor{};
    };
    export inline VISERA_RUNTIME_API FMouse
    GMouse{};

    export class VISERA_RUNTIME_API IWindow : public IGlobalSingleton
    {
    protected:
        Bool bInitialized = False;
    public:
        enum class EType
        {
            Unknown,
            Null,
            GLFW
        };
        [[nodiscard]] static inline FMouse&
        GetMouse() {  return GMouse; }

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
        [[nodiscard]] virtual FStringView
        GetTitle() const = 0;
        virtual void
        SetTitle(FStringView I_Title) = 0;

        [[nodiscard]] inline UInt32
        GetWidth() const  { return Width; }
        [[nodiscard]] inline UInt32
        GetHeight() const { return Height; }
        [[nodiscard]] EType
        GetType() const { return Type; }
        [[nodiscard]] inline Bool
        IsMaximized() const { return bMaximized; };

        explicit IWindow() = delete;
        explicit IWindow(EType I_Type) : IGlobalSingleton("Window"), Type(I_Type) {}

    protected:
        EType       Type { EType::Unknown };
        Int32       Width {900},  Height{600};
        Float       ScaleX{1.0f}, ScaleY{1.0f};
        Bool        bMaximized{False};
    };
}
