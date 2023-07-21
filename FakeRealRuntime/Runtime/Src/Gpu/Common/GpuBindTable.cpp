#include "Gpu/GpuBindTable.h"
#include "Utils/Hash/hash.h"
#include <set>

void GPUBindTableValue::Initialize(const GPUBindTableLocation& loc, const GPUDescriptorData& rhs)
{
    mData         = rhs;
    mBinded       = false;
    mData.binding = loc.binding;
    mResources.resize(mData.count);
    for (uint32_t i = 0; i < mData.count; i++)
    {
        mResources[i] = rhs.ptrs[i];
    }
    mData.ptrs = mResources.data();
}

GPUBindTableID GPUBindTable::Create(GPUDeviceID device, const GPUBindTableDescriptor* desc)
{
    GPURootSignatureID rs  = desc->pRootSignature;
    uint64_t hashsSize     = desc->namesCount * sizeof(uint64_t);
    uint64_t locationsSize = desc->namesCount * sizeof(GPUBindTableLocation);
    uint64_t setsSize      = rs->table_count * sizeof(GPUDescriptorSetID);
    uint64_t totalSize     = hashsSize + locationsSize + setsSize + sizeof(GPUBindTable);

    GPUBindTable* pBindTable = (GPUBindTable*)_aligned_malloc(totalSize, _alignof(GPUBindTable));
    memset(pBindTable, 0, totalSize);
    uint64_t* pHash                  = (uint64_t*)(pBindTable + 1);
    GPUBindTableLocation* pLocations = (GPUBindTableLocation*)(pHash + desc->namesCount);
    GPUDescriptorSetID* ppSets       = (GPUDescriptorSetID*)(pLocations + desc->namesCount);

    pBindTable->m_pRS            = rs;
    pBindTable->m_pNamesHash     = pHash;
    pBindTable->m_pNamesLocation = pLocations;
    pBindTable->mNamesCount      = desc->namesCount;
    pBindTable->mSetsCount       = rs->table_count;
    pBindTable->m_ppSets         = ppSets;

    for (uint32_t i = 0; i < pBindTable->mNamesCount; i++)
    {
        pBindTable->m_pNamesHash[i] = GPU_NAME_HASH(desc->ppNames[i], strlen(desc->ppNames[i]));
    }

    for (uint32_t setIdx = 0; setIdx < rs->table_count; setIdx++)
    {
        auto& table = rs->tables[setIdx];
        for (uint32_t bindIdx = 0; bindIdx < table.resources_count; bindIdx++)
        {
            uint64_t nameHash = GPU_NAME_HASH((const char*)table.resources[bindIdx].name, strlen((const char*)table.resources[bindIdx].name));
            for (uint32_t i = 0; i < desc->namesCount; i++)
            {
                if (nameHash == pBindTable->m_pNamesHash[i])
                {
                    new (pBindTable->m_pNamesLocation + i) GPUBindTableLocation();
                    const_cast<uint32_t&>(pBindTable->m_pNamesLocation[i].tableIndex) = setIdx;
                    const_cast<uint32_t&>(pBindTable->m_pNamesLocation[i].binding)    = bindIdx;

                    GPUDescriptorSetDescriptor set_desc {};
                    set_desc.root_signature = rs;
                    set_desc.set_index      = setIdx;
                    if (!ppSets[i])
                    {
                        ppSets[i] = GPUCreateDescriptorSet(device, &set_desc);
                    }
                    break;
                }
            }
        }
    }

    return pBindTable;
}

void GPUBindTable::Free(GPUBindTableID table)
{
    if (table)
    {
        for (uint32_t i = 0; i < table->mSetsCount; i++)
        {
            if (table->m_ppSets[i]) GPUFreeDescriptorSet(table->m_ppSets[i]);
        }

        for (uint32_t i = 0; i < table->mNamesCount; i++)
        {
            table->m_pNamesLocation[i].~GPUBindTableLocation();
        }
        ((GPUBindTable*)table)->~GPUBindTable();
        _aligned_free((void*)table);
    }
}

void GPUBindTable::Bind(GPURenderPassEncoderID encoder) const
{
    for (uint32_t i = 0; i < mSetsCount; i++)
    {
        if (m_ppSets[i]) GPURenderEncoderBindDescriptorSet(encoder, m_ppSets[i]);
    }
}

void GPUBindTable::Update(const GPUDescriptorData* pData, uint32_t count)
{
    for (uint32_t i = 0;  i < count; i++)
    {
        const auto& data = pData[i];
        if (data.name)
        {
            uint64_t hash = GPU_NAME_HASH((const char*)data.name, strlen((const char*)data.name));
            for (uint32_t j = 0; j < mNamesCount; j++)
            {
                if (hash == m_pNamesHash[j])
                {
                    auto& location = m_pNamesLocation[j];
                    if (!EqualTo<GPUDescriptorData>()(location.mValue.mData, data))
                    {
                        location.mValue.Initialize(location, data);
                    }
                    break;
                }
            }
        }
        else
        {

        }
    }
    UpdateSelfIfDirty();
}

void GPUBindTable::UpdateSelfIfDirty()
{
    std::set<uint32_t> needUpdateIdxSet;
    for (uint32_t i = 0; i < mNamesCount; i++)
    {
        if (!m_pNamesLocation[i].mValue.mBinded) needUpdateIdxSet.insert(m_pNamesLocation[i].tableIndex);
    }

    for (uint32_t setIdx : needUpdateIdxSet)
    {
        std::vector<GPUDescriptorData> datas;
        for (uint32_t i = 0; i < mNamesCount; i++)
        {
            if (!m_pNamesLocation[i].mValue.mBinded && setIdx == m_pNamesLocation[i].tableIndex)
            {
                m_pNamesLocation[i].mValue.mBinded = true;
                datas.emplace_back(m_pNamesLocation[i].mValue.mData);
            }
        }
        uint32_t count = (uint32_t)datas.size();
        if (count) GPUUpdateDescriptorSet(m_ppSets[setIdx], datas.data(), count);
    }
}

///////////////////////////////////////////////////
bool EqualTo<GPUDescriptorData>::operator()(const GPUDescriptorData& a, const GPUDescriptorData& b)
{
    if (a.binding != b.binding) return false;
    if (a.binding_type != b.binding_type) return false;
    if (a.count != b.count) return false;
    for (uint32_t i = 0; i < a.count; i++)
    {
        if (a.ptrs[i] != b.ptrs[i]) return false;
    }
    return true;
}

/////////////////////////////////////////////////////
GPUBindTableID GPUCreateBindTable(GPUDeviceID device, const GPUBindTableDescriptor* desc)
{
    return GPUBindTable::Create(device, desc);
}

void GPUFreeBindTable(GPUBindTableID table)
{
    GPUBindTable::Free(table);
}

void GPUBindTableUpdate(GPUBindTableID table, const GPUDescriptorData* datas, uint32_t count)
{
    ((GPUBindTable*)table)->Update(datas, count);
}

void GPURenderEncoderBindBindTable(GPURenderPassEncoderID encoder, GPUBindTableID table)
{
    table->Bind(encoder);
}