#pragma once

#define RUNTIME_API

#ifdef NDEBUG
    #define gpu_assert(expr) (void)(expr);
#else
    #include "assert.h"
    #define gpu_assert assert
#endif

#ifdef __cplusplus
    #define RUNTIME_EXTERN_C extern "C"
#else
    #define RUNTIME_EXTERN_C extern
#endif

#ifdef __cplusplus
    #define FR_NOEXCEPT noexcept
#else
    #define FR_NOEXCEPT
#endif

#if defined(_MSC_VER) && !defined(__clang__)
    #ifndef FORCEINLINE
        #define FORCEINLINE __forceinline
    #endif
#else
    #ifndef FORCEINLINE
        #define FORCEINLINE inline __attribute__((always_inline))
    #endif
#endif

#if defined(_MSC_VER)
    #define FR_ALIGNAS(x) __declspec(align(x))
#else
    #define FR_ALIGNAS(x) __attribute__((aligned(x)))
#endif