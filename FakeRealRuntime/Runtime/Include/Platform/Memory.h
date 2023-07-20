#pragma once
#include "Platform/Config.h"
#include <memory>

template<typename T>
FORCEINLINE T* New()
{
    void* ptr = calloc(1, sizeof(T));
    T* obj = new (ptr) T();
    return obj;
}

template<typename T, typename... Args>
FORCEINLINE T* New(Args&&... args)
{
    void* ptr = calloc(1, sizeof(T));
    T* obj = new (ptr) T(std::forward<Args>(args)...);
    return obj;
}

template <typename T>
FORCEINLINE void Delete(T* obj)
{
    if (obj)
    {
        obj->~T();
        free(obj);
    }
}

#define FR_New New
#define FR_Delete Delete