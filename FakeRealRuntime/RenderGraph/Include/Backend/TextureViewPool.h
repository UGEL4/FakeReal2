#pragma once

#include <stdint.h>
#include <unordered_map>
#include <deque>
#include "Gpu/GpuApi.h"

namespace FakeReal
{
    namespace render_graph
    {
        class TextureViewPool
        {
        public:
            struct AllocationMark
            {
                uint64_t frame_index;
                uint32_t tags;
            };
            struct PooledTextureView
            {
                PooledTextureView() = delete;
                PooledTextureView(GPUTextureViewID texture, AllocationMark mark)
                    : m_pTextureView(texture)
                    , mMark(mark)
                {
                }
                GPUTextureViewID m_pTextureView;
                AllocationMark mMark;
            };
            struct Key
            {
                const GPUDeviceID m_pDevice   = nullptr;
                const GPUTextureID m_pTexture = nullptr;
                EGPUFormat format             = GPU_FORMAT_UNDEFINED;
                uint32_t usages               = 0;
                uint32_t aspectMask           = 0;
                EGPUTextureDimension dims     = GPU_TEX_DIMENSION_2D;
                uint32_t baseMipLevel         = 0;
                uint32_t mipLevelCount        = 0;
                uint32_t baseArrayLayer       = 0;
                uint32_t arrayLayerCount      = 0;
                uint32_t width                = 0;
                uint32_t heught               = 0;
                uint64_t uniqueId             = 0;

                operator size_t() const;
                friend class TextureViewPool;
                struct hasher
                {
                    inline size_t operator()(const Key& val) const
                    {
                        return (size_t)val;
                    }
                };

                Key(GPUDeviceID device, const GPUTextureViewDescriptor& desc);
            };
            friend class RenderGraphBackend;
            void Initialize(GPUDeviceID device);
            void Finalize();
            GPUTextureViewID Allocate(const GPUTextureViewDescriptor& desc, uint64_t frameIndex);

        protected:
            GPUDeviceID m_pDevice;
            std::unordered_map<Key, PooledTextureView, Key::hasher> mTextureViews;
        };
    } // namespace render_graph
} // namespace FakeReal