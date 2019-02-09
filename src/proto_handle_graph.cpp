/**
 * \file proto_handle_graph.hpp: implementation of the ProtoHandleGraph
 */


#include "proto_handle_graph.hpp"


namespace vg {

using namespace std;

    ProtoHandleGraph::ProtoHandleGraph(const Graph* graph) : graph(graph) {
        // nothing to do
    }
    
    handle_t ProtoHandleGraph::get_handle_by_index(const size_t& i) const {
        return EasyHandlePacking::pack(i, false);
    }
    
    size_t ProtoHandleGraph::edge_size() const {
        return graph->edge_size();
    }
    
    edge_t ProtoHandleGraph::get_edge_by_index(const size_t& i) const {
        const Edge& edge = graph->edge(i);
        return edge_handle(get_handle(edge.from(), edge.from_start()),
                           get_handle(edge.to(), edge.to_end()));
    }
    
    bool ProtoHandleGraph::has_node(id_t node_id) const {
        bool found = false;
        for (size_t i = 0; i < graph->node_size() && !found; i++) {
            found = graph->node(i).id() == node_id;
        }
        return found;
    }
    
    handle_t ProtoHandleGraph::get_handle(const id_t& node_id, bool is_reverse) const {
        for (size_t i = 0; i < graph->node_size(); i++) {
            if (graph->node(i).id() == node_id) {
                return EasyHandlePacking::pack(i, is_reverse);
            }
        }
        // tried to find a handle for a node that doesn't exist
        cerr << "error::[ProtoHandleGraph] requested handle for a node that does not exist: " << node_id << endl;
        return as_handle(-1);
    }
    
    id_t ProtoHandleGraph::get_id(const handle_t& handle) const {
        return graph->node(EasyHandlePacking::unpack_number(handle)).id();
    }
    
    bool ProtoHandleGraph::get_is_reverse(const handle_t& handle) const {
        return EasyHandlePacking::unpack_bit(handle);
    }
    
    handle_t ProtoHandleGraph::flip(const handle_t& handle) const {
        return EasyHandlePacking::toggle_bit(handle);
    }
    
    size_t ProtoHandleGraph::get_length(const handle_t& handle) const {
        return graph->node(EasyHandlePacking::unpack_number(handle)).sequence().size();
    }
    
    string ProtoHandleGraph::get_sequence(const handle_t& handle) const {
        return graph->node(EasyHandlePacking::unpack_number(handle)).sequence();
    }
    
    bool ProtoHandleGraph::follow_edges(const handle_t& handle, bool go_left,
                                      const function<bool(const handle_t&)>& iteratee) const {
        bool keep_going = true;
        bool leftward = (go_left != get_is_reverse(handle));
        for (size_t i = 0; i < graph->edge_size() && keep_going; i++) {
            const Edge& edge = graph->edge(i);
            if (edge.from() == get_id(handle) && leftward == edge.from_start()) {
                keep_going = iteratee(get_handle(edge.to(), go_left != edge.to_end()));
            }
            else if (edge.to() == get_id(handle) && leftward != edge.to_end()) {
                keep_going = iteratee(get_handle(edge.from(), go_left == edge.from_start()));
            }
        }
        return keep_going;
    }
    
    void ProtoHandleGraph::for_each_handle(const function<bool(const handle_t&)>& iteratee, bool parallel) const {
        if (parallel) {
            size_t num_nodes = graph->node_size();
#pragma omp parallel shared(num_nodes)
            for (size_t i = 0; i < num_nodes; i++) {
                // ignore stopping early in parallel mode
                iteratee(EasyHandlePacking::pack(i, false));
            }
        }
        else {
            bool keep_going = true;
            for (size_t i = 0; i < graph->node_size() && keep_going; i++) {
                keep_going = iteratee(EasyHandlePacking::pack(i, false));
            }
        }
    }
    
    size_t ProtoHandleGraph::node_size() const {
        return graph->node_size();
    }
    
    id_t ProtoHandleGraph::min_node_id() const {
        id_t min_id = numeric_limits<id_t>::max();
        for (size_t i = 0; i < graph->node_size(); i++) {
            min_id = min(min_id, graph->node(i).id());
        }
        return min_id;
    }
    
    id_t ProtoHandleGraph::max_node_id() const {
        id_t max_id = numeric_limits<id_t>::min();
        for (size_t i = 0; i < graph->node_size(); i++) {
            max_id = max(max_id, graph->node(i).id());
        }
        return max_id;
    }

}

