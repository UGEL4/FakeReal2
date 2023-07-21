#pragma once

#include <unordered_map>
#include <deque>
#include "Gpu/GpuApi.h"

namespace FakeReal
{
    namespace render_graph
    {
        class TexturePool
        {
        public:
            struct AllocationMark
            {
                uint64_t frame_index;
                uint32_t tags;
            };
            struct PooledTexture
            {
                PooledTexture() = delete;
                PooledTexture(GPUTextureID texture, EGPUResourceState state, AllocationMark mark)
                    : m_pTexture(texture)
                    , mState(state)
                    , mMark(mark)
                {
                }
                GPUTextureID m_pTexture;
                EGPUResourceState mState;
                AllocationMark mMark;
            };
            struct Key
            {
                const GPUDeviceID m_pDevice;
                const GPUTextureCreationFlags mFlags;
                uint32_t mWidth;
                uint32_t mHeight;
                uint32_t mDepth;
                uint32_t mArraySize;
                EGPUFormat mFormat;
                uint32_t mMipLevels;
                EGPUSampleCount mSampleCount;
                uint32_t mSampleQuality;
                GPUResourceTypes mDescriptors;
                bool mIsDedicated = false;
                operator size_t() const;
                friend class TexturePool;

                struct hasher
                {
                    inline size_t operator()(const Key& val) const
                    {
                        return (size_t)val;
                    }
                };

                Key(GPUDeviceID device, const GPUTextureDescriptor& desc);
            };
            friend class RenderGraphBackend;
            void Initialize(GPUDeviceID device);
            void Finalize();
            std::pair<GPUTextureID, EGPUResourceState> Allocate(const GPUTextureDescriptor& desc, AllocationMark mark);
            void Deallocate(const GPUTextureDescriptor& desc, GPUTextureID texture, EGPUResourceState final_state, AllocationMark mark);

        protected:
            GPUDeviceID m_pDevice;
            std::unordered_map<Key, std::deque<PooledTexture>, Key::hasher> mTextures;
        };
    } // namespace render_graph
} // namespace FakeReal