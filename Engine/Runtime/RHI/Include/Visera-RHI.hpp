#pragma once
#include <Visera-Global.hpp>

namespace Visera
{
    inline constexpr UInt8  kInvalidSwapChainID    = 8; // Max Swapchain count.
    inline constexpr UInt32 kMaxInFlightFrames     = 3;
    inline constexpr UInt32 kMaxColorAttachments   = 8;
    inline constexpr UInt64 kFrameFenceTimeoutNs   = 5'000'000'000ULL;   // 5s
    inline constexpr UInt64 kUploadFenceTimeoutNs  = 10'000'000'000ULL;  // 10s
    inline constexpr UInt64 kUtilityFenceTimeoutNs = 5'000'000'000ULL;   // 5s
    inline constexpr UInt64 kAcquireTimeoutNs      = 3'000'000'000ULL;   // 3s
    inline constexpr UInt64 kCommandListHighWaterMarkBytes = 64ULL * 1024; // 64KB
    /** Push constant data (offset + up to 128 bytes). Must be called inside a render pass with a pipeline that has push constants. */
    inline constexpr UInt8  kMaxPushConstantSize   = 128U;
}