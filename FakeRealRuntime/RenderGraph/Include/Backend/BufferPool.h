#pragma once

#include <stdint.h>
#include <unordered_map>
#include <deque>
#include "Gpu/GpuApi.h"

namespace FakeReal
{
    namespace render_graph
    {
        class BufferPool
        {
        public:
            struct AllocationMark
            {
                uint64_t frame_index;
                uint32_t tags;
            };
            struct PooledBuffer
            {
                PooledBuffer() = delete;
                PooledBuffer(GPUBufferID buffer, EGPUResourceState state, AllocationMark mark)
                    : m_pBuffer(buffer)
                    , mState(state)
                    , mMark(mark)
                {
                }
                GPUBufferID m_pBuffer;
                EGPUResourceState mState;
                AllocationMark mMark;
            };
            struct Key
            {
                const GPUDeviceID m_pDevice   = nullptr;
                GPUResourceTypes mTypes       = 0;
                EGPUMemoryUsage mMemoryUsage  = GPU_MEM_USAGE_GPU_ONLY;
                EGPUFormat mFormat            = GPU_FORMAT_UNDEFINED;
                GPUBufferCreationFlags mFlags = 0;

                operator size_t() const;
                friend class BufferPool;
                struct hasher
                {
                    inline size_t operator()(const Key& val) const
                    {
                        return (size_t)val;
                    }
                };

                Key(GPUDeviceID device, const GPUBufferDescriptor& desc);
            };
            friend class RenderGraphBackend;
            void Initialize(GPUDeviceID device);
            void Finalize();
            std::pair<GPUBufferID, EGPUResourceState> Allocate(const GPUBufferDescriptor& desc, AllocationMark mark, uint64_t min_frame_index);
            void Deallocate(const GPUBufferDescriptor& desc, GPUBufferID buffer, EGPUResourceState final_state, AllocationMark mark);

        protected:
            GPUDeviceID m_pDevice;
            std::unordered_map<Key, std::deque<PooledBuffer>, Key::hasher> mBufferPools;
        };
    } // namespace render_graph
} // namespace FakeReal