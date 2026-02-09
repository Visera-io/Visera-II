#pragma once
#include <Visera-Global.hpp>

#define VISERA_MAX_PUSH_CONSTANT_SIZE 32
// Use this Macro inside the renderpass class;
#define VISERA_PUSH_CONSTANT(...)                                           \
struct alignas(16) FPushConstantRange { __VA_ARGS__ };                      \
static_assert(sizeof(FPushConstantRange) <= VISERA_MAX_PUSH_CONSTANT_SIZE,  \
"Push constant exceeds VISERA_MAX_PUSH_CONSTANT_SIZE");                     \
static_assert((sizeof(FPushConstantRange) % 4) == 0,                        \
"Push constant size should be multiple of 4")