#include "Frontend/ResourceEdge.h"
#include "Frontend/PassNode.h"
#include "Frontend/ResourceNode.h"
#include "Utils/Hash/hash.h"

namespace FakeReal
{
    namespace render_graph
    {
        ///////////TextureEdge////////////////
        TextureEdge::TextureEdge(ERelationshipType type, EGPUResourceState requestedState)
            : RenderGraphEdge(type)
            , mRequestedState(requestedState)
        {
        }
        ///////////TextureEdge////////////////

        ///////////TextureWriteEdge////////////////
        TextureWriteEdge::TextureWriteEdge(uint32_t mrtIndex, TextureRTVHandle handle, EGPUResourceState requestedState)
            : TextureEdge(ERelationshipType::TextureWrite, requestedState)
            , mTextureHandle(handle)
            , mMRTIndex(mrtIndex)
        {
        }
        PassNode* TextureWriteEdge::GetPassNode()
        {
            return (PassNode*)From();
        }
        TextureNode* TextureWriteEdge::GetTextureNode()
        {
            return (TextureNode*)To();
        }
        ///////////TextureWriteEdge////////////////

        ///////////TextureReadEdge////////////////
        TextureReadEdge::TextureReadEdge(const std::string_view& name, TextureSRVHandle handle, EGPUResourceState requestedState)
            : TextureEdge(ERelationshipType::TextureRead, requestedState)
            , mNameHash(GPU_NAME_HASH(name.data(), strlen(name.data())))
            , mName(name)
            , mTextureHandle(handle)
        {
        }
        PassNode* TextureReadEdge::GetPassNode()
        {
            return (PassNode*)To();
        }

        TextureNode* TextureReadEdge::GetTextureNode()
        {
            return (TextureNode*)From();
        }
        ///////////TextureReadEdge////////////////
        ///////////BufferEdge////////////////
        BufferEdge::BufferEdge(ERelationshipType type, EGPUResourceState requestedState)
            : RenderGraphEdge(type)
            , mRequestedState(requestedState)
        {
        }

        BufferReadEdge::BufferReadEdge(const std::string_view& name, BufferRangeHandle handle, EGPUResourceState requestedState)
            : BufferEdge(ERelationshipType::BufferRead, requestedState)
            , mNameHash(GPU_NAME_HASH(name.data(), strlen(name.data())))
            , mName(name)
            , mHandle(handle)
        {
        }
        PassNode* BufferReadEdge::GetPassNode()
        {
            return (PassNode*)To();
        }

        BufferNode* BufferReadEdge::GetBufferNode()
        {
            return (BufferNode*)From();
        }
        ///////////BufferEdge////////////////
        ///////////BufferReadWriteEdge////////////////
        BufferReadWriteEdge::BufferReadWriteEdge(BufferRangeHandle handle, EGPUResourceState requestedState)
            : BufferEdge(ERelationshipType::BufferReadWrite, requestedState)
            , mHandle(handle)
        {
        }
        PassNode* BufferReadWriteEdge::GetPassNode()
        {
            return (PassNode*)To();
        }

        BufferNode* BufferReadWriteEdge::GetBufferNode()
        {
            return (BufferNode*)From();
        }
        ///////////BufferReadWriteEdge////////////////
    } // namespace render_graph
} // namespace FakeReal