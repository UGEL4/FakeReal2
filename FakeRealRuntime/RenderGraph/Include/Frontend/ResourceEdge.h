#pragma once
#include "BaseTypes.h"
#include "DependencyGraph.h"
#include <string_view>

namespace FakeReal
{
    namespace render_graph
    {
        class PassNode;
        class TextureNode;
        class BufferNode;
        class TextureEdge : public RenderGraphEdge
        {
            friend class RenderGraph;
            friend class RenderGraphBackend;

        public:
            TextureEdge(ERelationshipType type, EGPUResourceState requestedState);
            virtual ~TextureEdge() = default;

            virtual PassNode* GetPassNode()       = 0;
            virtual TextureNode* GetTextureNode() = 0;

        protected:
            EGPUResourceState mRequestedState;
        };

        class TextureWriteEdge : public TextureEdge
        {
            friend class RenderGraph;
            friend class RenderGraphBackend;

        public:
            TextureWriteEdge(uint32_t mrtIndex, TextureRTVHandle handle, EGPUResourceState requestedState = EGPUResourceState::GPU_RESOURCE_STATE_RENDER_TARGET);

            virtual PassNode* GetPassNode() final;
            virtual TextureNode* GetTextureNode() final;
            const uint32_t GetMipBase() const
            {
                return mTextureHandle.mMipBase;
            }
            const uint32_t GetArrayBase() const
            {
                return mTextureHandle.mArrayBase;
            }
            const uint32_t GetArrayCount() const
            {
                return mTextureHandle.mArrayCount;
            }

        private:
            TextureRTVHandle mTextureHandle;
            uint32_t mMRTIndex;
        };

        class TextureReadEdge : public TextureEdge
        {
        public:
            friend class RenderGraph;
            friend class RenderGraphBackend;

            TextureReadEdge(const std::string_view& name, TextureSRVHandle handle, EGPUResourceState requestedState = EGPUResourceState::GPU_RESOURCE_STATE_SHADER_RESOURCE);
            virtual PassNode* GetPassNode() final;
            virtual TextureNode* GetTextureNode() final;

            const uint32_t GetMipBase() const
            {
                return mTextureHandle.mMipBase;
            }
            const uint32_t GetMipCount() const
            {
                return mTextureHandle.mMipCount;
            }
            const uint32_t GetArrayBase() const
            {
                return mTextureHandle.mArrayBase;
            }
            const uint32_t GetArrayCount() const
            {
                return mTextureHandle.mArrayCount;
            }
            const EGPUTextureDimension GetDimension() const
            {
                return mTextureHandle.mDim;
            }
            const char* GetName() const
            {
                return mName.c_str();
            }

        private:
            uint64_t mNameHash;
            std::string mName; // shader resource name
            TextureSRVHandle mTextureHandle;
        };

        ////////////////////////////////////////////BufferEdge///////////////////////////////
        class BufferEdge : public RenderGraphEdge
        {
        public:
            friend class RenderGraph;
            friend class RenderGraphBackend;
            BufferEdge(ERelationshipType type, EGPUResourceState requestedState);
            virtual ~BufferEdge() = default;

            virtual PassNode* GetPassNode()     = 0;
            virtual BufferNode* GetBufferNode() = 0;

        protected:
            EGPUResourceState mRequestedState;
        };

        class BufferReadEdge : public BufferEdge
        {
        public:
            friend class RenderGraph;
            friend class RenderGraphBackend;
            BufferReadEdge(const std::string_view& name, BufferRangeHandle handle, EGPUResourceState requestedState = EGPUResourceState::GPU_RESOURCE_STATE_UNDEFINED);
            virtual PassNode* GetPassNode() final;
            virtual BufferNode* GetBufferNode() final;
            const char* GetName() const
            {
                return mName.c_str();
            }

        private:
            uint64_t mNameHash;
            std::string mName; // shader resource name
            BufferRangeHandle mHandle;
        };

        class BufferReadWriteEdge : public BufferEdge
        {
        public:
            friend class RenderGraph;
            friend class RenderGraphBackend;
            BufferReadWriteEdge(BufferRangeHandle handle, EGPUResourceState requestedState = EGPUResourceState::GPU_RESOURCE_STATE_UNDEFINED);
            virtual PassNode* GetPassNode() final;
            virtual BufferNode* GetBufferNode() final;

        private:
            BufferRangeHandle mHandle;
        };
    } // namespace render_graph
} // namespace FakeReal