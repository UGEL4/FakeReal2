#pragma once

#include "boost/graph/adjacency_list.hpp"
#include "DependencyGraph.h"

namespace FakeReal
{
    namespace DAG
    {
        template <
        class VertexProperty = boost::no_property,
        class EdgeProperty   = boost::no_property,
        class GraphProperty  = boost::no_property,
        class EdgeListS      = boost::listS>
        using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VertexProperty, EdgeProperty, GraphProperty, EdgeListS>;

        template <
        class VertexProperty = boost::no_property,
        class EdgeProperty   = boost::no_property,
        class GraphProperty  = boost::no_property>
        using GraphVertex = typename boost::graph_traits<Graph<VertexProperty, EdgeProperty, GraphProperty>>::vertex_descriptor;

        template <
        class VertexProperty = boost::no_property,
        class EdgeProperty   = boost::no_property,
        class GraphProperty  = boost::no_property>
        using GraphEdge = typename boost::graph_traits<Graph<VertexProperty, EdgeProperty, GraphProperty>>::edge_descriptor;

        template <typename... Ts,
                  typename bidirGraph  = boost::adjacency_list<Ts...>,
                  typename bidirVertex = typename bidirGraph::vertex_descriptor,
                  typename IndexMap    = typename boost::property_map<bidirGraph, boost::vertex_index_t>::type>
        auto VertexNumber(bidirVertex vert, bidirGraph g)
        {
            IndexMap index = get(boost::vertex_index, g);
            return index[vert];
        }

        template <typename prop_name_t, typename... Ts,
                  typename bidirGraph  = boost::adjacency_list<Ts...>,
                  typename bidirVertex = typename bidirGraph::vertex_descriptor,
                  typename PropMap     = typename boost::property_map<bidirGraph, prop_name_t>::type>
        auto GetVertexProperty(bidirVertex vert, bidirGraph g)
        {
            PropMap prop = get(prop_name_t(), g);
            return prop[vert];
        }
    } // namespace DAG

    class DependencyGraphBase : public DAG::Graph<DependencyGraph::Node*, DependencyGraph::Edge*>
    {
    public:
        using DAGVertex = DAG::GraphVertex<DependencyGraph::Node*, DependencyGraph::Edge*>;
        using DAGEdge   = DAG::GraphEdge<DependencyGraph::Node*, DependencyGraph::Edge*>;

        static DependencyGraphBase* As(DependencyGraph* graph);
    };
} // namespace FakeReal