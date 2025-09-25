module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.CommandBuffer;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API ICommandBuffer
    {
    public:
        enum class EType : UInt8
        {
            Unknown,
            Graphics,
            Compute,
        };

        enum class EStatus : UInt8
        {
            Idle,
            Recording,
            InsideRenderPass,
            ReadyToSubmit,
        };

        virtual void
        Begin() const = 0;
        virtual void
        End() const = 0;
        virtual void
        Submit(const void* I_Queue) const = 0;

        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

        [[nodiscard]] inline Bool
        IsIdle() const { return Status == EStatus::Idle; }
        [[nodiscard]] inline Bool
        IsRecording() const { return Status == EStatus::Recording; }
        [[nodiscard]] inline Bool
        IsInsideRenderPass() const { return Status == EStatus::InsideRenderPass; }
        [[nodiscard]] inline Bool
        IsReadyToSubmit() const { return Status == EStatus::ReadyToSubmit; }

    protected:
        const   EType   Type   {EType::Unknown};
        mutable EStatus Status {EStatus::Idle};

    public:
        ICommandBuffer()                                 = delete;
        ICommandBuffer(EType I_Type) : Type(I_Type) {}
        virtual ~ICommandBuffer()                        = default;
        ICommandBuffer(const ICommandBuffer&)            = delete;
        ICommandBuffer& operator=(const ICommandBuffer&) = delete;
    };
}


template <>
struct fmt::formatter<Visera::RHI::ICommandBuffer::EType>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(const Visera::RHI::ICommandBuffer::EType& I_Type, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        const char* TypeName = "Unknown";
        switch (I_Type)
        {
        case Visera::RHI::ICommandBuffer::EType::Graphics:
            TypeName = "Graphics"; break;
        case Visera::RHI::ICommandBuffer::EType::Compute:
            TypeName = "Compute"; break;
        default: break;
        }
        return fmt::format_to(I_Context.out(),"{}", TypeName);
    }
};
