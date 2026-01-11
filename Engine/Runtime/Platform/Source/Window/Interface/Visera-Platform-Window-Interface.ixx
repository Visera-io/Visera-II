module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Window.Interface;
#define VISERA_MODULE_NAME "Platform.Window"

namespace Visera
{
    export class VISERA_PLATFORM_API IWindow
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
        virtual void
        SetIcon(TMutable<FByte> I_Data, Int32 I_Width, Int32 I_Height) = 0;

        [[nodiscard]] inline UInt32
        GetWidth() const  { return Width; }
        [[nodiscard]] inline UInt32
        GetHeight() const { return Height; }
        [[nodiscard]] inline Float
        GetScaleX() const  { return ScaleX; }
        [[nodiscard]] inline Float
        GetScaleY() const { return ScaleY; }
        [[nodiscard]] EType
        GetType() const { return Type; }
        [[nodiscard]] inline Bool
        IsMaximized() const { return bMaximized; };

        explicit IWindow() = delete;
        explicit IWindow(EType I_Type) : Type(I_Type) {}
        virtual ~IWindow() = default;

    protected:
        EType       Type { EType::Unknown };
        Int32       Width {1920},  Height{1080};
        Float       ScaleX{1.0f}, ScaleY{1.0f};
        Bool        bMaximized{False};
    };
}
