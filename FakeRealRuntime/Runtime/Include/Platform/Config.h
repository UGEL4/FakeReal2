#pragma once

#define RUNTIME_API

#ifdef NDEBUG
    #define gpu_assert(expr) (void)(expr);
#else
    #include "assert.h"
    #define gpu_assert assert
#endif