#include "Backend/TexturePool.h"
#include "Utils/Hash/hash.h"

namespace FakeReal
{
    namespace render_graph
    {
        TexturePool::Key::Key(GPUDeviceID device, const GPUTextureDescriptor& desc)
            : m_pDevice(device)
            , mFlags(desc.flags)
            , mWidth(desc.width)
            , mHeight(desc.height)
            , mDepth(desc.depth ? desc.depth : 1)
            , mArraySize(desc.array_size ? desc.array_size : 1)
            , mFormat(desc.format)
            , mMipLevels(desc.mip_levels ? desc.mip_levels : 1)
            , mSampleCount(desc.sample_count)
            , mSampleQuality(desc.sample_quality)
            , mDescriptors(desc.descriptors)
            , mIsDedicated(desc.is_dedicated)
        {
        }

        TexturePool::Key::operator size_t() const
        {
            return Hash64(this, sizeof(*this), (size_t)m_pDevice);
        }

        void TexturePool::Initialize(GPUDeviceID device)
        {
            m_pDevice = device;
        }

        void TexturePool::Finalize()
        {
            for (auto&& pair : mTextures)
            {
                while (!pair.second.empty())
                {
                    GPUFreeTexture(pair.second.front().m_pTexture);
                    pair.second.pop_front();
                }
            }
            mTextures.clear();
        }

        std::pair<GPUTextureID, EGPUResourceState> TexturePool::Allocate(const GPUTextureDescriptor& desc, AllocationMark mark)
        {
            std::aligned_storage_t<sizeof(TexturePool::Key)> stroage;
            std::memset(&stroage, 0, sizeof(stroage));
            TexturePool::Key key = *(new (&stroage) TexturePool::Key(m_pDevice, desc));
            auto& pool           = mTextures[key];
            if (pool.empty())
            {
                auto tex = GPUCreateTexture(m_pDevice, &desc);
                pool.emplace_back(tex, desc.start_state, mark);
            }
            pool.front().mMark                                   = mark;
            std::pair<GPUTextureID, EGPUResourceState> allocated = { pool.front().m_pTexture, pool.front().mState };
            pool.pop_front();
            return allocated;
        }

        void TexturePool::Deallocate(const GPUTextureDescriptor& desc, GPUTextureID texture, EGPUResourceState final_state, AllocationMark mark)
        {
            std::aligned_storage_t<sizeof(TexturePool::Key)> stroage;
            std::memset(&stroage, 0, sizeof(stroage));
            TexturePool::Key key = *(new (&stroage) TexturePool::Key(m_pDevice, desc));
            auto& pool           = mTextures[key];
            for (auto&& iter : pool)
            {
                if (iter.m_pTexture == texture) return;
            }
            pool.emplace_back(texture, final_state, mark);
        }
    } // namespace render_graph
} // namespace FakeReal