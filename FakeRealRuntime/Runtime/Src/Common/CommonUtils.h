#pragma once

#ifdef __cplusplus
#include <type_traits>

template <typename T, typename... Args>
T* GPUNewPlaced(void* memory, Args&&... args)
{
    return new (memory) T(std::forward<Args>(args)...);
}

template <typename T>
void GPUDeletePlaced(T* obj)
{
    if (obj)
    {
        obj->~T();
    }
}

template <typename T, typename... Args>
T* GPUNew(Args&&... args)
{
    void* ptr = calloc(1, sizeof(T));
    return GPUNewPlaced(ptr, std::forward<Args>(args)...);
}

template <typename T>
void GPUDelete(T* obj)
{
    if (obj)
    {
        GPUDeletePlaced(obj);
        free(obj);
    }
}

#endif