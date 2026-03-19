#pragma once

#include <Visera-Core.hpp>
#if defined(VISERA_CORE_API)
#undef VISERA_CORE_API
#endif

// VISERA_RUNTIME_API: from CMake VISERA_RUNTIME_BUILD_STATIC/SHARED (PRIVATE for SHARED, PUBLIC for STATIC).
#if defined(VISERA_RUNTIME_BUILD_STATIC)
#  define VISERA_RUNTIME_API
#elif defined(VISERA_ON_WINDOWS_SYSTEM)
#  if defined(VISERA_RUNTIME_BUILD_SHARED)
#    define VISERA_RUNTIME_API __declspec(dllexport)
#  else
#    define VISERA_RUNTIME_API __declspec(dllimport)
#  endif
#else
#  define VISERA_RUNTIME_API __attribute__((visibility("default")))
#endif

namespace Visera
{
    // RHI SwapChain
    inline constexpr UInt8  kMaxSwapChainCount     = 8;
    inline constexpr UInt8  kInvalidSwapChainID    = kMaxSwapChainCount;
    inline constexpr UInt32 kMaxInFlightFrames     = 3;

    // RHI Render Pass
    inline constexpr UInt32 kMaxColorAttachments   = 8;
    inline constexpr UInt8  kMaxPushConstantSize   = 128U;

    // RHI Fence Timeout
    inline constexpr UInt64 kFrameFenceTimeoutNs   = 5'000'000'000ULL;   // 5s
    inline constexpr UInt64 kUploadFenceTimeoutNs  = 10'000'000'000ULL;  // 10s
    inline constexpr UInt64 kUtilityFenceTimeoutNs = 5'000'000'000ULL;   // 5s
    inline constexpr UInt64 kAcquireTimeoutNs      = 3'000'000'000ULL;   // 3s

    // RHI Command List
    inline constexpr UInt64 kCommandListHighWaterMarkBytes = 64ULL * 1024; // 64KB

    // RHI Descriptor Pool
    inline constexpr UInt32 kMaxDescriptorPoolSets = 4096;

    // RHI Staging Ring
    inline constexpr UInt64 kRHIStagingRingSize = 64ULL * 1024 * 1024; // 64MB

    // Graphics – Instance Data Pages
    inline constexpr UInt64 kInstanceDataPageSize          = 32ULL * 1024 * 1024; // 32MB
    inline constexpr UInt32 kInstanceDataPageCount         = 1u;
    inline constexpr UInt64 kInstanceDataBufferAlignment   = 80u; // = sizeof(FInstanceData); no dynamic-offset alignment needed

    // Graphics – Per-Frame UBO
    inline constexpr UInt64 kPerFrameUBOSize = 128u;

    // Graphics
    inline constexpr UInt32 kRenderGraphCompilingInlineMemory = 4096; // 4KB
    inline constexpr UInt32 kMaxPendingDrawRenderTasks         = 1;
    inline constexpr UInt32 kMaxDirtyWaitMs                    = 5000;

    // AssetHub
    inline constexpr UInt64 kAssetHubDefaultImageMB  = 64;
    inline constexpr UInt64 kAssetHubDefaultShaderMB = 32;

    // Audio
    inline constexpr UInt64 kAudioPumpInlineArenaBytes = 32ULL * 1024; // 32KB
}
