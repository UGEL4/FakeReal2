#include "Frontend/ResourceNode.h"

namespace FakeReal
{
    namespace render_graph
    {
        ResourceNode::ResourceNode(EObjectType type)
            : RenderGraphNode(type)
        {
        }

        ////////////////texture node//////////////////////////
        TextureNode::TextureNode()
            : ResourceNode(EObjectType::Texture)
        {
        }

        TextureHandle TextureNode::GetHandle() const
        {
            return TextureHandle(GetId());
        }
        ////////////////texture node//////////////////////////
        ////////////////buffer node//////////////////////////
        BufferNode::BufferNode()
            : ResourceNode(EObjectType::Buffer)
        {
        }
        BufferHandle BufferNode::GetHandle() const
        {
            return BufferHandle(GetId());
        }
        ////////////////buffer node//////////////////////////
    } // namespace render_graph
} // namespace FakeReal