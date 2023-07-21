#include "render_graph/include/backend/BufferPool.hpp"
#include "hash.h"

namespace RG
{
    BufferPool::Key::Key(GPUDeviceID device, const GPUBufferDescriptor& desc)
    : m_pDevice(device), mTypes(desc.descriptors), mMemoryUsage(desc.memory_usage)
    , mFormat(desc.format), mFlags(desc.flags)
    {

    }

    BufferPool::Key::operator size_t() const
    {
        return Hash64(this, sizeof(*this), (size_t)m_pDevice);
    }

    void BufferPool::Initialize(GPUDeviceID device)
    {
        m_pDevice = device;
    }

    void BufferPool::Finalize()
    {
        for (auto& pair : mBufferPools)
        {
            while (!pair.second.empty())
            {
                GPUFreeBuffer(pair.second.front().m_pBuffer);
                pair.second.pop_front();
            }
        }
        mBufferPools.clear();
    }

    std::pair<GPUBufferID, EGPUResourceState> BufferPool::Allocate(const GPUBufferDescriptor& desc, AllocationMark mark, uint64_t min_frame_index)
    {
        std::aligned_storage_t<sizeof(BufferPool::Key)> stroage;
        std::memset(&stroage, 0, sizeof(stroage));
        BufferPool::Key key = *(new (&stroage) BufferPool::Key(m_pDevice, desc));
        auto& pool = mBufferPools[key];
        int64_t index = -1;
        for (uint64_t i = 0; i < pool.size(); i++)
        {
            if (pool[i].mMark.frame_index < min_frame_index && pool[i].m_pBuffer->size >= desc.size)
            {
                index = i;
                break;
            }
        }
        if (index > 0)
        {
            PooledBuffer pooled = pool[index];
            pool.erase(pool.begin() + index);
            pool.emplace_front(pooled.m_pBuffer, pooled.mState, pooled.mMark);
        }
        if (index < 0)
        {
            auto buffer = GPUCreateBuffer(m_pDevice, &desc);
            pool.emplace_back(buffer, desc.start_state, mark);
        }
        
        std::pair<GPUBufferID, EGPUResourceState> allocated = { pool.front().m_pBuffer, pool.front().mState };
        pool.pop_front();
        return allocated;
    }

    void BufferPool::Deallocate(const GPUBufferDescriptor& desc, GPUBufferID buffer, EGPUResourceState final_state, AllocationMark mark)
    {
        std::aligned_storage_t<sizeof(BufferPool::Key)> stroage;
        std::memset(&stroage, 0, sizeof(stroage));
        BufferPool::Key key = *(new (&stroage) BufferPool::Key(m_pDevice, desc));
        auto& pool = mBufferPools[key];
        for (auto&& iter : pool)
        {
            if (iter.m_pBuffer == buffer) return;
        }
        pool.emplace_back(buffer, final_state, mark);
    }
}