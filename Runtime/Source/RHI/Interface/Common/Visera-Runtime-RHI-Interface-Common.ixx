module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.Common;
#define VISERA_MODULE_NAME "Runtime.RHI"

export namespace Visera::RHI
{
    enum class EImageLayout
    {
        Undefined,

        Color,
        Depth,
        DepthStencil,

        TransferSource,
        TransferDestination,

        Present,
    };

    enum class EPipelineStage
    {
        Top,
        ColorOutput,
    };

    enum class EAccess
    {
        None,
        W_Color,
    };

    struct FExtent2D
    {
        UInt32 Width {0};
        UInt32 Height{0};
    };
}

using namespace Visera;

template <>
struct fmt::formatter<RHI::EImageLayout>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(RHI::EImageLayout I_ImageLayout, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        const char* Name = "Undefined";
        switch (I_ImageLayout)
        {
        case RHI::EImageLayout::Color:               Name = "Color"; break;
        case RHI::EImageLayout::Depth:               Name = "Depth"; break;
        case RHI::EImageLayout::DepthStencil:        Name = "DepthStencil"; break;
        case RHI::EImageLayout::TransferSource:      Name = "TransferSource"; break;
        case RHI::EImageLayout::TransferDestination: Name = "TransferDestination"; break;
        case RHI::EImageLayout::Present:             Name = "Present"; break;
        default: break;
        }
        return fmt::format_to(I_Context.out(),"{}", Name);
    }
};