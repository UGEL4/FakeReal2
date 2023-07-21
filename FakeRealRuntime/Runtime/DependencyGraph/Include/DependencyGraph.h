#pragma once
#include <functional>

namespace FakeReal
{
    typedef uint64_t dep_graph_handle_t;

    class DependencyGraph;
    class DependencyGraphNode
    {
    public:
        friend class DependencyGraphImp;
        DependencyGraphNode() = default;
        const dep_graph_handle_t GetId() const
        {
            return mId;
        }
        uint32_t ForeachNeighbors(const std::function<void(DependencyGraphNode* neighbor)>&);
        uint32_t ForeachNeighbors(const std::function<void(const DependencyGraphNode* neighbor)>&) const;
        uint32_t InComingEdges() const;
        uint32_t OutGoingEdges() const;

    protected:
        DependencyGraph* m_pGraph;
        dep_graph_handle_t mId;
    };

    class DependencyGraphEdge
    {
    public:
        friend class DependencyGraphImp;
        DependencyGraphEdge() = default;

        DependencyGraphNode* From();
        DependencyGraphNode* To();

    protected:
        DependencyGraph* m_pGraph;
        dep_graph_handle_t mFromNode;
        dep_graph_handle_t mToNode;
    };

    // insert : boost::add_vertex
    // link : boost::add_edge
    class DependencyGraph
    {
    public:
        using Node = DependencyGraphNode;
        using Edge = DependencyGraphEdge;
        static DependencyGraph* Create();
        static void Destroy(DependencyGraph* graph);
        virtual ~DependencyGraph() = default;

        virtual dep_graph_handle_t Insert(DependencyGraphNode* pNode) = 0;
        virtual bool Link(DependencyGraphNode* pFrom, DependencyGraphNode* pTo, DependencyGraphEdge* pEdge) = 0;
        virtual Node* AccessNode(dep_graph_handle_t handle) = 0;
        virtual Node* NodeAt(dep_graph_handle_t id) = 0;
        virtual bool Clear() = 0;
        virtual uint32_t InComingEdges(const Node* node) const = 0;
        virtual uint32_t InComingEdges(dep_graph_handle_t handle) const = 0;
        virtual uint32_t OutGoingEdges(const Node* node) const = 0;
        virtual uint32_t OutGoingEdges(dep_graph_handle_t handle) const = 0;
        virtual uint32_t ForeachIncomingEdges(Node* node, const std::function<void(Node* from, Node* to, Edge* edge)>&) = 0;
        virtual uint32_t ForeachIncomingEdges(dep_graph_handle_t handle, const std::function<void(Node* from, Node* to, Edge* edge)>&) = 0;
        virtual uint32_t ForeachOutgoingEdges(Node* node, const std::function<void(Node* from, Node* to, Edge* edge)>&) = 0;
        virtual uint32_t ForeachOutgoingEdges(dep_graph_handle_t handle, const std::function<void(Node* from, Node* to, Edge* edge)>&) = 0;
        virtual uint32_t ForeachNeighbors(Node* node, const std::function<void(DependencyGraphNode* neighbor)>&) = 0;
        virtual uint32_t ForeachNeighbors(dep_graph_handle_t handle, const std::function<void(DependencyGraphNode* neighbor)>&) = 0;
        virtual uint32_t ForeachNeighbors(const Node* node, const std::function<void(const DependencyGraphNode* neighbor)>&) const = 0;
        virtual uint32_t ForeachNeighbors(const dep_graph_handle_t handle, const std::function<void(const DependencyGraphNode* neighbor)>&) const = 0;
    };

    inline DependencyGraphNode* DependencyGraphEdge::From()
    {
        return m_pGraph->NodeAt(mFromNode);
    }

    inline DependencyGraphNode* DependencyGraphEdge::To()
    {
        return m_pGraph->NodeAt(mToNode);
    }
} // namespace FakeReal