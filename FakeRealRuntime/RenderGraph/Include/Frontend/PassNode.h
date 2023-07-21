#pragma once
#include "BaseTypes.h"
#include "DependencyGraph.h"
#include <vector>
#include <functional>
#include <span>

namespace FakeReal
{
    namespace render_graph
    {
        class TextureEdge;
        class TextureWriteEdge;
        class TextureReadEdge;
        class BufferEdge;
        class BufferReadEdge;
        class BufferReadWriteEdge;
        class PassNode : public RenderGraphNode
        {
        public:
            friend class RenderGraph;
            friend class RenderGraphBackend;

            PassHandle const GetHandle() const;
            void ForEachTextures(const std::function<void(TextureNode*, TextureEdge*)>&);

            uint32_t GetTextureCount() const
            {
                return (int32_t)(mOutTextureEdges.size() + mInTextureEdges.size());
            }
            const bool Before(const PassNode* other) const;
            const bool After(const PassNode* other) const;
            std::span<TextureReadEdge*> GetTextureReadEdges();
            std::span<TextureWriteEdge*> GetTextureWriteEdges();

            uint32_t GetBuffersCount() const
            {
                return (uint32_t)(mInBufferEdges.size() + mOutBufferEdges.size());
            }
            std::span<BufferReadEdge*> GetBufferReadEdges();
            std::span<BufferReadWriteEdge*> GetBufferReadWriteEdges();
            void ForeachBuffer(const std::function<void(BufferNode*, BufferEdge*)>&);

        protected:
            PassNode(EPassType type, uint32_t order);

        protected:
            std::vector<TextureWriteEdge*> mOutTextureEdges;
            std::vector<TextureReadEdge*> mInTextureEdges;
            std::vector<BufferReadEdge*> mInBufferEdges;
            std::vector<BufferReadWriteEdge*> mOutBufferEdges;
            uint32_t mOrder;
            const EPassType mPassType = EPassType::None;
            bool mCanBeLone           = false;
        };

        class RenderPassNode : public PassNode
        {
        public:
            friend class RenderGraph;
            friend class RenderGraphBackend;
            RenderPassNode(uint32_t order);

        private:
            RenderPassExecuteFunction mExecuteFunc;
            GPURootSignatureID m_pRootSignature = nullptr;
            GPURenderPipelineID m_pPipeline = nullptr;
            EGPULoadAction mLoadActions[GPU_MAX_MRT_COUNT + 1];
            EGPUStoreAction mStoreActions[GPU_MAX_MRT_COUNT + 1];
            EGPULoadAction mDepthLoadAction;
            EGPUStoreAction mDepthStoreAction;
            EGPULoadAction mStencilLoadAction;
            EGPUStoreAction mStencilStoreAction;
            float mClearDepth;
        };

        class PresentPassNode : public PassNode
        {
        public:
            friend class RenderGraph;
            friend class RenderGraphBackend;

            inline bool Reimport(GPUSwapchainID swapchain, uint32_t index)
            {
                if (!swapchain) return false;
                mDesc.swapchain = swapchain;
                mDesc.index     = index;
                return true;
            }

            PresentPassNode(uint32_t order);

        private:
            GPUQueuePresentDescriptor mDesc;
        };

        class CopyPassNode : public PassNode
        {
        public:
            friend class RenderGraph;
            friend class RenderGraphBackend;

            CopyPassNode(uint32_t order);

        private:
            CopyPassExecuteFunction mExecuteFunc;
            std::vector<std::pair<TextureSubresourceHandle, TextureSubresourceHandle>> mT2Ts;
            std::vector<std::pair<BufferRangeHandle, BufferRangeHandle>> mB2Bs;
            std::vector<std::pair<BufferRangeHandle, TextureSubresourceHandle>> mB2Ts;
            std::vector<std::pair<BufferHandle, EGPUResourceState>> mBufferBarriers;
            std::vector<std::pair<TextureHandle, EGPUResourceState>> mTextureBarriers;
        };
    } // namespace render_graph
} // namespace FakeReal