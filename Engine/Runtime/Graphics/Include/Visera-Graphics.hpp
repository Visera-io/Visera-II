#pragma once
#include <Visera-RHI.hpp>

namespace Visera
{
    inline constexpr UInt32 kRenderGraphCompilingInlineMemory  = 4096; // 4KB
    inline constexpr UInt32 kMaxPendingDrawRenderTasks         = kMaxInFlightFrames;
    inline constexpr UInt32 kMaxDirtyWaitMs                    = 5000;
}