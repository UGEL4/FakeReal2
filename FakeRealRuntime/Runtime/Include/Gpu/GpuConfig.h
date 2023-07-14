#pragma once

#include "Platform/Config.h"

#define GPU_USE_VULKAN

#define GPU_API RUNTIME_API

#ifndef gpu_max
    #define gpu_max(a, b) (((a) > (b)) ? (a) : (b))
#endif // cgpu_max
#ifndef gpu_min
    #define gpu_min(a, b) (((a) < (b)) ? (a) : (b))
#endif // cgpu_max

#if defined(__cplusplus)
    #if defined(_MSC_VER) && !defined(__clang__)
        #define DECLARE_ZERO_VAL(type, var, num)              \
            type* var = (type*)_alloca(sizeof(type) * (num)); \
            memset((void*)var, 0, sizeof(type) * (num));
    #else
        #define DECLARE_ZERO_VAL(type, var, num) \
            type var[(num)];                    \
            memset((void*)var, 0, sizeof(type) * (num));
    #endif
#endif