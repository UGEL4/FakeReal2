#pragma once
#include "GpuApi.h"
#include <string>
#include <vector>

DEFINE_GPU_OBJECT(GPUBindTable)

typedef struct GPUBindTableDescriptor
{
    GPURootSignatureID pRootSignature;
    const char** ppNames;
    uint32_t namesCount;
} GPUBindTableDescriptor;

struct GPUBindTableLocation;
typedef struct GPUBindTableValue
{
    friend struct GPUBindTable;
public:
    GPUBindTableValue() = default;
    GPUBindTableValue(const GPUBindTableValue&) = delete;
    GPUBindTableValue& operator=(const GPUBindTableValue&) = delete;

    void Initialize(const GPUBindTableLocation& loc, const GPUDescriptorData& rhs);

protected:
    bool mBinded            = false;
    GPUDescriptorData mData = {};
    // arena
    std::vector<const void*> mResources;
} GPUBindTableValue;

typedef struct GPUBindTableLocation
{
    const uint32_t tableIndex = 0; // set index
    const uint32_t binding    = 0;
    GPUBindTableValue mValue;
} GPUBindTableLocation;

typedef struct GPUBindTable
{
    GPU_API static GPUBindTableID Create(GPUDeviceID device, const GPUBindTableDescriptor* desc);
    GPU_API static void Free(GPUBindTableID table);

    GPU_API void Bind(GPURenderPassEncoderID encoder) const;
    GPU_API void Update(const GPUDescriptorData* pData, uint32_t count);

    GPURootSignatureID m_pRS               = nullptr;
    uint64_t* m_pNamesHash                 = nullptr;
    GPUBindTableLocation* m_pNamesLocation = nullptr;
    uint32_t mNamesCount                   = 0;
    uint32_t mSetsCount                    = 0;
    GPUDescriptorSetID* m_ppSets           = nullptr;

private:
    void UpdateSelfIfDirty();

} GPUBindTable;


template <typename T> struct EqualTo;

template <>
struct EqualTo<GPUDescriptorData>
{
    GPU_API bool operator()(const GPUDescriptorData& a, const GPUDescriptorData& b);
};

/////////////////////////
RUNTIME_EXTERN_C GPU_API GPUBindTableID GPUCreateBindTable(GPUDeviceID device, const GPUBindTableDescriptor* desc);
RUNTIME_EXTERN_C GPU_API void GPUFreeBindTable(GPUBindTableID table);
RUNTIME_EXTERN_C GPU_API void GPUBindTableUpdate(GPUBindTableID table, const GPUDescriptorData* datas, uint32_t count);
RUNTIME_EXTERN_C GPU_API void GPURenderEncoderBindBindTable(GPURenderPassEncoderID encoder, GPUBindTableID table);
