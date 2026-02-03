module;
#include <Visera-Platform.hpp>
#define VISERA_MODULE_NAME "Platform.Window"
export module Visera.Platform.Window.Interface;
export import Visera.Core.Types.String;

export namespace Visera
{
    class VISERA_PLATFORM_API IWindow
    {
    public:
        struct FIconSet
        {
            const FByte* Icon16x16    = nullptr;
            const FByte* Icon32x32    = nullptr;
            const FByte* Icon48x48    = nullptr;
            const FByte* Icon64x64    = nullptr;
            const FByte* Icon128x128  = nullptr;
            const FByte* Icon256x256  = nullptr;
        };

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
        SetIcon(const FIconSet& I_IconSet) = 0;

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
        Float       ScaleX{1.0f},  ScaleY{1.0f};
        Bool        bMaximized{False};
    };
}
