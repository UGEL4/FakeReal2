#pragma once
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

void InitRSParamTables(struct GPURootSignature* RS, const struct GPURootSignatureDescriptor* desc);
void FreeRSParamTables(struct GPURootSignature* RS);
bool ShaderResourceIsRootConst(const struct GPUShaderResource* resource, const struct GPURootSignatureDescriptor* desc);
bool ShaderResourceIsStaticSampler(const struct GPUShaderResource* resource, const struct GPURootSignatureDescriptor* desc);

inline static char8_t* DuplicateString(const char8_t* src_string)
{
    if (src_string != NULL)
    {
        const size_t source_len = strlen((const char*)src_string);
        char8_t* result         = (char8_t*)malloc(sizeof(char8_t) * (1 + source_len));
#ifdef _WIN32
        strcpy_s((char*)result, source_len + 1, (const char*)src_string);
#else
        strcpy((char*)result, (const char*)src_string);
#endif
        return result;
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif

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
    return new (ptr) T(std::forward<Args>(args)...);
}

template <typename T>
void GPUDelete(T* obj)
{
    if (obj)
    {
        obj->~T();
        free(obj);
    }
}

#endif