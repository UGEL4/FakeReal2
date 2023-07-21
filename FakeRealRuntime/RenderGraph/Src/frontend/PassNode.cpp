#include "Frontend/PassNode.h"
#include "Frontend/ResourceEdge.h"

namespace FakeReal
{
    namespace render_graph
    {
        const PassHandle PassNode::GetHandle() const
        {
            return PassHandle(GetId());
        }

        void PassNode::ForEachTextures(const std::function<void(TextureNode*, TextureEdge*)>& func)
        {
            for (auto e : GetTextureReadEdges())
            {
                func(e->GetTextureNode(), e);
            }
            for (auto e : GetTextureWriteEdges())
            {
                func(e->GetTextureNode(), e);
            }
        }

        const bool PassNode::Before(const PassNode* other) const
        {
            if (other == nullptr) return false;
            return (mOrder < other->mOrder);
        }
        const bool PassNode::After(const PassNode* other) const
        {
            if (other == nullptr) return true;
            return (mOrder > other->mOrder);
        }

        std::span<TextureReadEdge*> PassNode::GetTextureReadEdges()
        {
            return std::span<TextureReadEdge*>(mInTextureEdges.data(), mInTextureEdges.size());
        }

        std::span<TextureWriteEdge*> PassNode::GetTextureWriteEdges()
        {
            return std::span<TextureWriteEdge*>(mOutTextureEdges.data(), mOutTextureEdges.size());
        }

        std::span<BufferReadEdge*> PassNode::GetBufferReadEdges()
        {
            return std::span<BufferReadEdge*>(mInBufferEdges.data(), mInBufferEdges.size());
        }

        std::span<BufferReadWriteEdge*> PassNode::GetBufferReadWriteEdges()
        {
            return std::span<BufferReadWriteEdge*>(mOutBufferEdges.data(), mOutBufferEdges.size());
        }

        void PassNode::ForeachBuffer(const std::function<void(BufferNode* bufferNode, BufferEdge* bifferEdge)>& func)
        {
            for (auto e : mInBufferEdges)
            {
                func(e->GetBufferNode(), e);
            }

            for (auto e : mOutBufferEdges)
            {
                func(e->GetBufferNode(), e);
            }
        }

        PassNode::PassNode(EPassType type, uint32_t order)
            : RenderGraphNode(EObjectType::Pass)
            , mOrder(order)
            , mPassType(type)
            , mCanBeLone(false)
        {
        }

        /////////////////////////////////////RenderPassNode
        RenderPassNode::RenderPassNode(uint32_t order)
            : PassNode(EPassType::Render, order)
        {
        }

        /////////////////////////////////////PresentPassNode
        PresentPassNode::PresentPassNode(uint32_t order)
            : PassNode(EPassType::Present, order)
        {
        }

        /////////////////////////////////////CopyPassNode
        CopyPassNode::CopyPassNode(uint32_t order)
            : PassNode(EPassType::Copy, order)
        {
        }
    } // namespace render_graph
} // namespace FakeReal