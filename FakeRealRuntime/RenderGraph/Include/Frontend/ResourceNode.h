#pragma once
#include "BaseTypes.h"
#include "DependencyGraph.h"

namespace FakeReal
{
    namespace render_graph
    {
        class ResourceNode : public RenderGraphNode
        {
        public:
            ResourceNode(EObjectType type);
            virtual ~ResourceNode() = default;

            const bool InImported() const
            {
                return mImported;
            }

        protected:
            bool mImported = false;
        };

        class TextureNode : public ResourceNode
        {
        public:
            friend class RenderGraph;
            friend class RenderGraphBackend;
            TextureNode();
            TextureHandle GetHandle() const;

        private:
            GPUTextureDescriptor mDesc = {};
            mutable GPUTextureID m_pFrameTexture;
            mutable EGPUResourceState mInitState = GPU_RESOURCE_STATE_UNDEFINED;
        };

        class BufferNode : public ResourceNode
        {
        public:
            friend class RenderGraph;
            friend class RenderGraphBackend;
            BufferNode();
            BufferHandle GetHandle() const;

        private:
            GPUBufferDescriptor mDesc = {};
            mutable GPUBufferID m_pBuffer;
            mutable EGPUResourceState mInitState = GPU_RESOURCE_STATE_UNDEFINED;
        };
    } // namespace render_graph
} // namespace FakeReal