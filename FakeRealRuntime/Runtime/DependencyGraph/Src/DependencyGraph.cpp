#include "DependencyGraph.h"
#include "Graph_Boost.h"
#include "boost/graph/named_graph.hpp"
#include "Platform/Memory.h"

namespace FakeReal
{
    class DependencyGraphImp : public DependencyGraph, public DependencyGraphBase
    {
        using DAGVertex = DependencyGraphBase::DAGVertex;

    public:
        virtual dep_graph_handle_t Insert(DependencyGraphNode* pNode) override;
        virtual bool Link(DependencyGraphNode* pFrom, DependencyGraphNode* pTo, DependencyGraphEdge* pEdge) override;
        virtual Node* AccessNode(dep_graph_handle_t handle) override;
        virtual Node* NodeAt(dep_graph_handle_t id) final;
        virtual bool Clear() final;
        virtual uint32_t InComingEdges(const Node* node) const final;
        virtual uint32_t OutGoingEdges(const Node* node) const final;
        virtual uint32_t InComingEdges(dep_graph_handle_t handle) const final;
        virtual uint32_t OutGoingEdges(dep_graph_handle_t handle) const final;
        virtual uint32_t ForeachIncomingEdges(Node* node, const std::function<void(Node* from, Node* to, Edge* edge)>& func) final;
        virtual uint32_t ForeachIncomingEdges(dep_graph_handle_t handle, const std::function<void(Node* from, Node* to, Edge* edge)>& func) final;
        virtual uint32_t ForeachOutgoingEdges(Node* node, const std::function<void(Node* from, Node* to, Edge* edge)>& func) final;
        virtual uint32_t ForeachOutgoingEdges(dep_graph_handle_t handle, const std::function<void(Node* from, Node* to, Edge* edge)>& func) final;
        virtual uint32_t ForeachNeighbors(Node* node, const std::function<void(DependencyGraphNode* neighbor)>&) final;
        virtual uint32_t ForeachNeighbors(dep_graph_handle_t handle, const std::function<void(DependencyGraphNode* neighbor)>&) final;
        virtual uint32_t ForeachNeighbors(const Node* node, const std::function<void(const DependencyGraphNode* neighbor)>&) const final;
        virtual uint32_t ForeachNeighbors(const dep_graph_handle_t handle, const std::function<void(const DependencyGraphNode* neighbor)>&) const final;

    protected:
        vertex_descriptor GetDescriptor(Node* node)
        {
            return (vertex_descriptor)node->mId;
        }
    };

    dep_graph_handle_t DependencyGraphImp::Insert(DependencyGraphNode* pNode)
    {
        // boos::add_vertex
        pNode->mId      = (dep_graph_handle_t)boost::add_vertex(pNode, *this);
        pNode->m_pGraph = this;
        return pNode->mId;
    }

    bool DependencyGraphImp::Link(DependencyGraphNode* pFrom, DependencyGraphNode* pTo, DependencyGraphEdge* pEdge)
    {
        // boost::add_edge
        auto&& result = boost::add_edge(GetDescriptor(pFrom), GetDescriptor(pTo), pEdge, *this);
        if (pEdge)
        {
            pEdge->m_pGraph  = this;
            pEdge->mFromNode = pFrom->GetId();
            pEdge->mToNode   = pTo->GetId();
        }
        return result.second;
    }

    DependencyGraph::Node* DependencyGraphImp::AccessNode(dep_graph_handle_t handle)
    {
        return (*this)[DAGVertex(handle)];
    }

    DependencyGraph::Node* DependencyGraphImp::NodeAt(dep_graph_handle_t id)
    {
        return (*this)[DAGVertex(id)];
    }

    bool DependencyGraphImp::Clear()
    {
        DAG::Graph<DependencyGraph::Node*, DependencyGraph::Edge*>::clear();
        return true;
    }

    uint32_t DependencyGraphImp::InComingEdges(const Node* node) const
    {
        return InComingEdges(node->mId);
    }

    uint32_t DependencyGraphImp::OutGoingEdges(const Node* node) const
    {
        return OutGoingEdges(node->mId);
    }

    uint32_t DependencyGraphImp::InComingEdges(dep_graph_handle_t handle) const
    {
        uint32_t count = 0;
        auto iedges    = boost::in_edges((vertex_descriptor)handle, *this);
        for (auto iter = iedges.first; iter != iedges.second; ++iter)
        {
            count++;
        }
        return count;
    }

    uint32_t DependencyGraphImp::OutGoingEdges(dep_graph_handle_t handle) const
    {
        uint32_t count = 0;
        auto oedges    = boost::out_edges((vertex_descriptor)handle, *this);
        for (auto iter = oedges.first; iter != oedges.second; ++iter)
        {
            count++;
        }
        return count;
    }

    uint32_t DependencyGraphImp::ForeachIncomingEdges(Node* node, const std::function<void(Node* from, Node* to, Edge* edge)>& func)
    {
        return ForeachIncomingEdges(node->mId, func);
    }

    uint32_t DependencyGraphImp::ForeachIncomingEdges(dep_graph_handle_t handle, const std::function<void(Node* from, Node* to, Edge* edge)>& func)
    {
        uint32_t count = 0;
        auto iedges    = boost::in_edges((vertex_descriptor)handle, *this);
        for (auto iter = iedges.first; iter != iedges.second; ++iter)
        {
            func(NodeAt(iter->m_source), NodeAt(iter->m_target), (*this)[*iter]);
            count++;
        }
        return count;
    }

    uint32_t DependencyGraphImp::ForeachOutgoingEdges(Node* node, const std::function<void(Node* from, Node* to, Edge* edge)>& func)
    {
        return ForeachOutgoingEdges(node->mId, func);
    }

    uint32_t DependencyGraphImp::ForeachOutgoingEdges(dep_graph_handle_t handle, const std::function<void(Node* from, Node* to, Edge* edge)>& func)
    {
        uint32_t count = 0;
        auto oedges    = boost::out_edges((vertex_descriptor)handle, *this);
        for (auto iter = oedges.first; iter != oedges.second; ++iter)
        {
            func(NodeAt(iter->m_source), NodeAt(iter->m_target), (*this)[*iter]);
            count++;
        }
        return count;
    }

    uint32_t DependencyGraphImp::ForeachNeighbors(Node* node, const std::function<void(DependencyGraphNode* neighbor)>& func)
    {
        return ForeachNeighbors(node->mId, func);
    }

    uint32_t DependencyGraphImp::ForeachNeighbors(dep_graph_handle_t handle, const std::function<void(DependencyGraphNode* neighbor)>& func)
    {
        DAGVertex vert(handle);
        auto neigs     = boost::adjacent_vertices(vert, *this);
        uint32_t count = 0;
        for (auto iter = neigs.first; iter != neigs.second; iter++)
        {
            func((*this)[*iter]);
            count++;
        }
        return count;
    }

    uint32_t DependencyGraphImp::ForeachNeighbors(const Node* node, const std::function<void(const DependencyGraphNode* neighbor)>& func) const
    {
        return ForeachNeighbors(node->mId, func);
    }

    uint32_t DependencyGraphImp::ForeachNeighbors(const dep_graph_handle_t handle, const std::function<void(const DependencyGraphNode* neighbor)>& func) const
    {
        DAGVertex vert(handle);
        auto neigs     = boost::adjacent_vertices(vert, *this);
        uint32_t count = 0;
        for (auto iter = neigs.first; iter != neigs.second; iter++)
        {
            func((*this)[*iter]);
            count++;
        }
        return count;
    }

    //////////////////////////////////////////////////////////////////////////////
    DependencyGraph* DependencyGraph::Create()
    {
        return New<DependencyGraphImp>();
    }

    void DependencyGraph::Destroy(DependencyGraph* graph)
    {
        Delete(graph);
    }

    DependencyGraphBase* DependencyGraphBase::As(DependencyGraph* graph)
    {
        return (DependencyGraphImp*)graph;
    }

    /////////////////////////////////////DependencyGraphNode///////////////////////////////////////////
    uint32_t DependencyGraphNode::ForeachNeighbors(const std::function<void(DependencyGraphNode* neighbor)>& func)
    {
        return m_pGraph->ForeachNeighbors(this, func);
    }
    uint32_t DependencyGraphNode::ForeachNeighbors(const std::function<void(const DependencyGraphNode* neighbor)>& func) const
    {
        return m_pGraph->ForeachNeighbors(this, func);
    }

    uint32_t DependencyGraphNode::InComingEdges() const
    {
        return m_pGraph->InComingEdges(this);
    }

    uint32_t DependencyGraphNode::OutGoingEdges() const
    {
        return m_pGraph->OutGoingEdges(this);
    }
    /////////////////////////////////////DependencyGraphNode///////////////////////////////////////////
} // namespace FakeReal