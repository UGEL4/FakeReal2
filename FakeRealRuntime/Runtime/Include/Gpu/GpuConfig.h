#pragma once

#include "Platform/Config.h"
#include "Utils/Macro.h"

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

#define GPU_NAME_HASH_SEED 8053064571610612741
#define GPU_HASH(buffer, size, seed) Hash64((buffer), (size), (seed))
#define GPU_NAME_HASH(buffer, size) GPU_HASH((buffer), (size), GPU_NAME_HASH_SEED)

#define GPU_SAFE_FREE(ptr) if (ptr) free(ptr);

#define GPU_LOG_INFO LOG_INFO
#define GPU_LOG_DEBUG LOG_DEBUG
#define GPU_LOG_WARNING LOG_WARNING
#define GPU_LOG_ERROR LOG_ERROR