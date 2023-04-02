#include "precomp.h"

namespace DataVis
{
    SugiyamaLayout::SugiyamaLayout()
    {
        m_oscm_heuristics.emplace_back("Barycenter", SugiyamaLayout::OSCMBarycenterHeuristic);
        m_oscm_heuristics.emplace_back("Median", SugiyamaLayout::OSCMMedianHeuristic);
        m_node_offset = {100, -200};
    }

    //--------------------------------------------------------------
    bool SugiyamaLayout::Gui(IStructure& _structure)
    {
        bool active = false;
        if (ImGui::TreeNode("Sugiyama Layout"))
        {
            ImGui::InputInt("OSCM Iterations", &m_oscm_iterations);
            ImGui::InputFloat("Delta X", &m_node_offset.x);
            ImGui::InputFloat("Delta Y", &m_node_offset.y);

            const char* select_oscm_heuristic_preview = m_oscm_heuristics[m_oscm_heuristic_idx].first.c_str();
            if (ImGui::BeginCombo("Select OSCM Heuristic", select_oscm_heuristic_preview))
            {
                for (int n = 0; n < m_oscm_heuristics.size(); n++)
                {
                    const bool is_selected = (m_oscm_heuristic_idx == n);
                    if (ImGui::Selectable(m_oscm_heuristics[n].first.c_str(), is_selected))
                        m_oscm_heuristic_idx = n;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Checkbox("Curved Edges", &m_curved_edges);

            if (ImGui::Button("Apply"))
            {
                try
                {
                    Graph& graph = dynamic_cast<Graph&>(_structure);
                    Apply(graph, m_oscm_heuristics[m_oscm_heuristic_idx].second, m_node_offset, m_oscm_iterations, m_curved_edges);
                    active = true;
                }
                catch (std::exception& e)
                {
                    std::cout << e.what() << std::endl;
                }
            }
            ImGui::TreePop();
        }
        return active;
    }

    //--------------------------------------------------------------
    void SugiyamaLayout::Apply(Graph& _graph, const OSCMHeuristic& _heuristic, const glm::vec2& _node_offset,
                         const int& _oscm_iterations, bool _curved_edges)
    {
        Dataset copy = *_graph.dataset;

        // Step 01: Break cycles
        std::vector<int> reversed_edges;
        Dataset new_dataset = BreakCycles(copy, reversed_edges);

        // Step 02: Layer assignment
        std::vector<Layer> vertices_per_layer;
        Layer layer_per_vertex;
        LayerAssignment(new_dataset, vertices_per_layer, layer_per_vertex);
        AddDummyVertices(new_dataset, vertices_per_layer, layer_per_vertex);

        // Step 03: Crossing minimization
        const int crossings = CrossingMinimization(new_dataset, vertices_per_layer, _heuristic, _oscm_iterations);
        new_dataset.AddInfo("# crossings", std::to_string(crossings));

        // Step 04: Vertex positioning
        const auto x_per_vertex = VertexPositioning(new_dataset, vertices_per_layer, layer_per_vertex, _node_offset.x);

        // Step 05: Reverse back reversed edges
        ReverseEdges(new_dataset, reversed_edges);
        
        _graph.Load(std::make_shared<Dataset>(std::move(new_dataset)));
        std::vector<glm::vec3> new_positions(new_dataset.vertices.size());
        for (size_t y = 0; y < vertices_per_layer.size(); y++)
        {
            for (const int idx : vertices_per_layer[y])
            {
                const auto new_pos = glm::vec3(x_per_vertex[idx], _node_offset.y * y, 0);
                new_positions[idx] = new_pos;
                const auto& node = _graph.nodes[idx];
                node->SetNewPosition(new_pos);
            }
        }

        _graph.edges.resize(copy.edges.size());
        CreateEdges(_graph, new_positions, _node_offset, _curved_edges);
            
        _graph.UpdateAABB();
    }

    void SugiyamaLayout::ReverseEdges(Dataset& _dataset, const std::vector<int>& _reversed_edges)
    {
        const auto& vertices = _dataset.vertices;
        for(const int edge_idx : _reversed_edges)
        {
            auto& edge = _dataset.edges[edge_idx];
            int from_idx = edge.from_idx;
            int to_idx = edge.to_idx;
            const int old_idx = edge.idx;
            
            // Reverse first edge
            RemoveNeighbors(_dataset, edge);
            vertices[from_idx]->incoming_neighbors.emplace_back(to_idx, edge.idx);
            vertices[to_idx]->outgoing_neighbors.emplace_back(from_idx, edge.idx);
            edge.from_idx = to_idx;
            edge.to_idx = from_idx;
            
            // v0 -> dum -> v1
            while(vertices[to_idx]->id == DUMMY_ID)
            {
                const auto& outgoing_neigbors = vertices[to_idx]->outgoing_neighbors;
                auto& edge = _dataset.edges[outgoing_neigbors[0].edge_idx];
                from_idx = edge.from_idx;
                to_idx = edge.to_idx;
                
                RemoveNeighbors(_dataset, edge);
                vertices[from_idx]->incoming_neighbors.emplace_back(to_idx, edge.idx);
                vertices[to_idx]->outgoing_neighbors.emplace_back(from_idx, edge.idx);
                edge.from_idx = to_idx;
                edge.to_idx = from_idx;
                edge.idx = old_idx;
            }            
        }
    }

    void SugiyamaLayout::CreateEdges(Graph& _graph, const std::vector<glm::vec3>& _new_positions, const glm::vec2& _node_offset, bool _curved_edges)
    {
        const auto& nodes = _graph.nodes;
        const auto& dataset = *_graph.dataset;
        // Create curves
        const auto& vertices = dataset.vertices;
        for(const auto& edge : dataset.edges)
        {
            if(vertices[edge.from_idx]->id == DUMMY_ID)
                continue;

            _graph.edges[edge.idx] = std::make_shared<EdgePath>(edge.idx, _graph.dataset->GetKind());
            const auto& edge_path = _graph.edges[edge.idx];
            edge_path->Clear();
            // Set line type
            if(_curved_edges)
            {
                edge_path->SetStyle(EdgePath::Style::Curve);
                // Add first control point
            
                const auto& incoming_neighbors = vertices[edge.from_idx]->incoming_neighbors;
                if(not incoming_neighbors.empty())
                    edge_path->SetStartCtrlPoint(_new_positions[incoming_neighbors[0].idx]);
                else
                    edge_path->SetStartCtrlPoint(_new_positions[edge.from_idx] - glm::vec3(0, _node_offset.y, 0));
            }
            else
            {
                edge_path->SetStyle(EdgePath::Style::Line);
            }
            
            // Add start vertex
            edge_path->AddPoint(_new_positions[edge.from_idx]);

            auto current_edge = edge;
            while (vertices[current_edge.to_idx]->id == DUMMY_ID)
            {
                edge_path->AddPoint(_new_positions[current_edge.to_idx]);
                auto& next_node = vertices[current_edge.to_idx];
                // Only 1 outgoing edge
                current_edge = dataset.edges[next_node->outgoing_neighbors[0].edge_idx];
            } 
            // Add end vertex
            edge_path->AddPoint(_new_positions[current_edge.to_idx]);
            edge_path->SetArrowOffset(nodes[current_edge.to_idx]->GetRadius() * 2);

            if(_curved_edges)
            {
                // Add last control point
                const auto& neighbors = vertices[current_edge.to_idx]->outgoing_neighbors;
                if(not neighbors.empty()) 
                    edge_path->SetEndCtrlPoint(_new_positions[neighbors[0].idx]);
                else
                    edge_path->SetEndCtrlPoint(_new_positions[current_edge.to_idx] + glm::vec3(0, _node_offset.y, 0));
            }
        }
    }

    //--------------------------------------------------------------
    // Break Cycles
    //--------------------------------------------------------------
    Dataset SugiyamaLayout::BreakCycles(Dataset& _dataset, std::vector<int>& _reversed_edges )
    {
        // Make a copy
        auto& vertices = _dataset.vertices;

        // Create a new dataset without any edges
        Dataset new_dataset;
        new_dataset = _dataset;
        new_dataset.vertices.clear();        
        auto& new_vertices = new_dataset.vertices;

        for (auto& vertex : vertices)
        {
            new_vertices.emplace_back(std::make_shared<Vertex>());
            auto& v = *new_vertices.back();
            v.id = vertex->id;
            v.idx = vertex->idx;
            v.attributes = vertex->attributes;
        }

        // While there are unvisited vertices
        Vertex dummy;
        while (HasUnvisited([](Vertex&) { return true; }, vertices, dummy))
        {
            // Remove sinks
            Vertex sink;
            while (HasUnvisited(SugiyamaLayout::IsSink, vertices, sink))
            {
                for (auto& neighbor : sink.incoming_neighbors)
                {
                    // Add edge to new dataset
                    new_vertices[sink.idx]->incoming_neighbors.push_back(neighbor);
                    Neighbor rev_neighbor = neighbor;
                    rev_neighbor.idx = sink.idx;
                    new_vertices[neighbor.idx]->outgoing_neighbors.push_back(rev_neighbor);

                    // Remove edge from old dataset
                    auto& outgoing = vertices[neighbor.idx]->outgoing_neighbors;
                    for (int i = 0; i < outgoing.size(); i++)
                    {
                        if (outgoing[i].edge_idx == neighbor.edge_idx)
                        {
                            outgoing[i] = outgoing[outgoing.size() - 1];
                            outgoing.pop_back();
                        }
                    }
                }
                vertices[sink.idx]->incoming_neighbors.clear();
                vertices[sink.idx]->idx = VISITED_IDX;
            }

            // Delete all isolated nodes
            for (auto& v : vertices)
                if (v->incoming_neighbors.empty() && v->outgoing_neighbors.empty())
                    v->idx = VISITED_IDX;

            // Remove sources
            Vertex source;
            while (HasUnvisited(SugiyamaLayout::IsSource, vertices, source))
            {
                for (auto& neighbor : source.outgoing_neighbors)
                {
                    // Add edge to new dataset
                    new_vertices[source.idx]->outgoing_neighbors.push_back(neighbor);
                    Neighbor rev_neighbor = neighbor;
                    rev_neighbor.idx = source.idx;
                    new_vertices[neighbor.idx]->incoming_neighbors.push_back(rev_neighbor);

                    // Remove edge from old dataset
                    auto& incoming = vertices[neighbor.idx]->incoming_neighbors;
                    for (int i = 0; i < incoming.size(); i++)
                    {
                        if (incoming[i].edge_idx == neighbor.edge_idx)
                        {
                            incoming[i] = incoming[incoming.size() - 1];
                            incoming.pop_back();
                        }
                    }
                }
                vertices[source.idx]->outgoing_neighbors.clear();
                vertices[source.idx]->idx = VISITED_IDX;
            }

            // Check if vertices is empty
            size_t max_vertex_idx = VISITED_IDX;
            int max_difference = INT_MIN;
            for (int i = 0; i < vertices.size(); i++)
            {
                if (vertices[i]->idx != VISITED_IDX)
                {
                    int difference = vertices[i]->outgoing_neighbors.size() - vertices[i]->incoming_neighbors.size();
                    if (difference > max_difference)
                    {
                        max_vertex_idx = i;
                        max_difference = difference;
                    }
                }
            }

            // Remove max vertex with outgoing edges
            if (max_difference > INT_MIN && max_vertex_idx != VISITED_IDX)
            {
                Vertex& max_vertex = *vertices[max_vertex_idx];
                // Add outgoing to new dataset and remove from neighbors
                for (auto& neighbor : max_vertex.outgoing_neighbors)
                {
                    // Add edge to new dataset
                    new_vertices[max_vertex.idx]->outgoing_neighbors.push_back(neighbor);
                    Neighbor rev_neighbor = neighbor;
                    rev_neighbor.idx = max_vertex.idx;
                    new_vertices[neighbor.idx]->incoming_neighbors.push_back(rev_neighbor);

                    // Remove edge from old dataset
                    auto& incoming = vertices[neighbor.idx]->incoming_neighbors;
                    for (int i = 0; i < incoming.size(); i++)
                    {
                        if (incoming[i].edge_idx == neighbor.edge_idx)
                        {
                            incoming[i] = incoming[incoming.size() - 1];
                            incoming.pop_back();
                        }
                    }
                }

                // Remove incoming from neighbors
                for (auto& neighbor : max_vertex.incoming_neighbors)
                {
                    // Add flipped edge to new dataset
                    Neighbor rev_neighbor = neighbor;
                    rev_neighbor.idx = max_vertex.idx;
                    new_vertices[max_vertex.idx]->outgoing_neighbors.push_back(neighbor);
                    new_vertices[neighbor.idx]->incoming_neighbors.push_back(rev_neighbor);
                    
                    auto& edge = new_dataset.edges[neighbor.edge_idx];
                    VertexIdx temp = edge.to_idx;
                    edge.to_idx = edge.from_idx;
                    edge.from_idx = temp;
                    _reversed_edges.push_back(edge.idx);

                    // Remove edge from old dataset
                    auto& outgoing = vertices[neighbor.idx]->outgoing_neighbors;
                    for (int i = 0; i < outgoing.size(); i++)
                    {
                        if (outgoing[i].edge_idx == neighbor.edge_idx)
                        {
                            outgoing[i] = outgoing[outgoing.size() - 1];
                            outgoing.pop_back();
                        }
                    }
                }
                max_vertex.outgoing_neighbors.clear();
                max_vertex.incoming_neighbors.clear();
                max_vertex.idx = VISITED_IDX;
            }
        }
        return new_dataset;
    }

    bool SugiyamaLayout::HasUnvisited(const std::function<bool(Vertex&)>& _f, std::vector<std::shared_ptr<Vertex>> _vs, Vertex& _out)
    {
        for (auto& v : _vs)
        {
            if (v->idx != VISITED_IDX && _f(*v))
            {
                _out = *v;
                return true;
            }
        }
        return false;
    }

    //--------------------------------------------------------------
    // Layer Assignment
    //--------------------------------------------------------------
    void SugiyamaLayout::LayerAssignment(const Dataset& _dataset, std::vector<Layer>& _vertices_per_layer, Layer& _layer_per_vertex)
    {
        int layer = 0;
        Dataset copy_dataset = _dataset;
        const int size = _dataset.vertices.size();
        _layer_per_vertex.resize(size);

        bool hasSources = false;
        do
        {
            hasSources = false;
            _vertices_per_layer.emplace_back();
            for (auto& v : copy_dataset.vertices)
            {
                if (v->idx == VISITED_IDX) continue;

                if (IsSource(*v))
                {
                    hasSources = true;
                    // Assign layer to sink
                    _layer_per_vertex[v->idx] = layer;
                    _vertices_per_layer[layer].push_back(v->idx);
                    v->idx = REMOVE_IDX;
                }
            }
            for (auto& v : copy_dataset.vertices)
            {
                if (v->idx == REMOVE_IDX)
                {
                    // Remove sink from copy_dataset (including incoming edges)
                    RemoveOutgoingNeighbors(copy_dataset, *v);
                    v->idx = VISITED_IDX;
                }
            }
            layer++;
        }
        while (hasSources);
        _vertices_per_layer.resize(_vertices_per_layer.size() - 1);
    }

    //--------------------------------------------------------------
    void SugiyamaLayout::AddDummyVertices(Dataset& _dataset, std::vector<Layer>& _vertices_per_layer,
                                    Layer& _layer_per_vertex)
    {
        auto& vertices = _dataset.vertices;
        const int size = _dataset.edges.size();
        for (int j = 0; j < size; j++)
        {
            int current_edge_idx = j;
            const auto& edge = _dataset.edges[current_edge_idx];
            const int start_layer = _layer_per_vertex[edge.from_idx];
            const int end_layer = _layer_per_vertex[edge.to_idx];
            if (end_layer - start_layer <= 1) continue;

            for (int i = start_layer; i < end_layer - 1; i++)
            {
                auto& e = _dataset.edges[current_edge_idx];
                RemoveNeighbors(_dataset, e);

                vertices.emplace_back(std::make_shared<Vertex>());
                Vertex& dummy_vertex = *vertices.back();
                dummy_vertex.id = DUMMY_ID;
                dummy_vertex.idx = _dataset.vertices.size() - 1;
                
                _layer_per_vertex.emplace_back(i + 1);
                _vertices_per_layer[i + 1].emplace_back(dummy_vertex.idx);

                Edge dummy_edge;
                dummy_edge.from_idx = dummy_vertex.idx;
                dummy_edge.to_idx = e.to_idx;
                dummy_edge.idx = _dataset.edges.size();
                e.to_idx = dummy_vertex.idx;

                AddNeighbors(_dataset, e);
                AddNeighbors(_dataset, dummy_edge);

                current_edge_idx = dummy_edge.idx;
                _dataset.edges.emplace_back(dummy_edge);
            }
        }
    }

    bool SugiyamaLayout::IsSink(const Vertex& _v)
    {
        return _v.outgoing_neighbors.empty();
    }

    bool SugiyamaLayout::IsSource(const Vertex& _v)
    {
        return _v.incoming_neighbors.empty();
    }

    void SugiyamaLayout::RemoveOutgoingNeighbors(Dataset& _dataset, Vertex& _v)
    {
        // Add outgoing to new dataset and remove from neighbors
        for (const auto& neighbor : _v.outgoing_neighbors)
        {
            // Remove edge from dataset
            auto& incoming = _dataset.vertices[neighbor.idx]->incoming_neighbors;
            for (size_t i = 0; i < incoming.size(); i++)
            {
                if (incoming[i].edge_idx == neighbor.edge_idx)
                {
                    incoming[i] = incoming[incoming.size() - 1];
                    incoming.pop_back();
                }
            }
        }
        _v.outgoing_neighbors.clear();
    }

    void SugiyamaLayout::RemoveNeighbors(Dataset& _dataset, const Edge& _e)
    {
        auto& outgoing = _dataset.vertices[_e.from_idx]->outgoing_neighbors;
        for (size_t i = 0; i < outgoing.size(); i++)
        {
            if (outgoing[i].idx == _e.to_idx)
            {
                outgoing[i] = outgoing[outgoing.size() - 1];
                outgoing.pop_back();
                break;
            }
        }

        auto& incoming = _dataset.vertices[_e.to_idx]->incoming_neighbors;
        for (size_t i = 0; i < incoming.size(); i++)
        {
            if (incoming[i].idx == _e.from_idx)
            {
                incoming[i] = incoming[incoming.size() - 1];
                incoming.pop_back();
                break;
            }
        }
    }

    void SugiyamaLayout::AddNeighbors(const Dataset& _dataset, Edge& _e)
    {
        auto& outgoing = _dataset.vertices[_e.from_idx]->outgoing_neighbors;
        outgoing.emplace_back(_e.to_idx, _e.idx);
        auto& incoming = _dataset.vertices[_e.to_idx]->incoming_neighbors;
        incoming.emplace_back(_e.from_idx, _e.idx);
    }

    void SugiyamaLayout::RemoveIncomingNeighbors(const Dataset& _dataset, Vertex& _v)
    {
        // Remove incoming from neighbors
        for (const auto& neighbor : _v.incoming_neighbors)
        {
            // Remove edge from old dataset
            auto& outgoing = _dataset.vertices[neighbor.idx]->outgoing_neighbors;
            for (int i = 0; i < outgoing.size(); i++)
            {
                if (outgoing[i].edge_idx == neighbor.edge_idx)
                {
                    outgoing[i] = outgoing[outgoing.size() - 1];
                    outgoing.pop_back();
                }
            }
        }
        _v.incoming_neighbors.clear();
    }

    //--------------------------------------------------------------
    // Crossing Minimization
    //--------------------------------------------------------------
    int SugiyamaLayout::CrossingMinimization(Dataset& _dataset, std::vector<Layer>& _vertices_per_layer,
                                       const OSCMHeuristic& _heuristic, int _iterations)
    {
        auto& vertices = _dataset.vertices;
        const GetNeighbors get_neighbors = [](Vertex& v) { return v.outgoing_neighbors; };
        const GetNeighbors get_reverse_neighbors = [](Vertex& v) { return v.incoming_neighbors; };

        int best_crossings = INT_MAX;
        std::vector<Layer> best;
        for (int i = 0; i < _iterations; i++)
        {
            // Copy and shuffle first layer
            std::vector<Layer> vertices_per_layer = _vertices_per_layer;
            std::shuffle(vertices_per_layer[0].begin(), vertices_per_layer[0].end(), Random::random_device);
            int crossings = 0;
            int new_crossings = INT_MAX;

            do
            {
                crossings = new_crossings;
                new_crossings = 0;

                // Go up
                for (size_t j = 1; j < vertices_per_layer.size(); j++)
                {
                    Layer new_layer;
                    _heuristic(_dataset, vertices_per_layer[j - 1], vertices_per_layer[j], new_layer,
                               get_reverse_neighbors);
                    vertices_per_layer[j] = new_layer;
                }

                // Go down
                for (int j = vertices_per_layer.size() - 2; j >= 0; j--)
                {
                    Layer new_layer;
                    _heuristic(_dataset, vertices_per_layer[j + 1], vertices_per_layer[j], new_layer, get_neighbors);
                    vertices_per_layer[j] = new_layer;
                }

                // Count crossings
                for (int j = 1; j < vertices_per_layer.size(); j++)
                {
                    new_crossings += Crossings(_dataset, vertices_per_layer[j - 1], vertices_per_layer[j]);
                }
            }
            while (new_crossings < crossings);

            if (new_crossings < best_crossings)
            {
                // Found a better permutation for each layer
                best_crossings = new_crossings;
                best = vertices_per_layer;
            }
        }

        _vertices_per_layer = best;
        return best_crossings;
    }

    bool SugiyamaLayout::OSCMBarycenterHeuristic(const Dataset& _dataset, const Layer& _layer_fixed, Layer& _layer, Layer& _new_layer,
                                                 const GetNeighbors& _get_neighbors)
    {
        auto& vertices = _dataset.vertices;

        std::unordered_map<int, int> pos_per_vertex;
        for (size_t i = 0; i < _layer_fixed.size(); i++)
        {
            pos_per_vertex[_layer_fixed[i]] = i;
        }

        // barycenter, vertex idx
        std::vector<std::pair<float, int>> coords(_layer.size());
        for (int i = 0; i < _layer.size(); i++)
        {
            float coor = 0;
            auto& neighbors = _get_neighbors(*vertices[_layer[i]]);
            for (auto& neighbor : neighbors)
                coor += pos_per_vertex[neighbor.idx];
            coor /= static_cast<float>(neighbors.size());
            coords[i] = {coor, _layer[i]};
        }

        bool changed = false;
        std::stable_sort(coords.begin(), coords.end(), [&](std::pair<float, int> a, std::pair<float, int> b)
        {
            changed |= a.first < b.first;
            return a.first < b.first;
        });

        _new_layer.resize(_layer.size());
        for (size_t i = 0; i < _layer.size(); i++) _new_layer[i] = coords[i].second;
        return changed;
    }

    bool SugiyamaLayout::OSCMMedianHeuristic(
        const Dataset& _dataset,
        const Layer& _layer_fixed,
        Layer& _layer,
        Layer& _new_layer,
        const GetNeighbors& _get_neighbors )
    {
        auto& vertices = _dataset.vertices;

        std::unordered_map<int, int> pos_per_vertex;
        for (int i = 0; i < _layer_fixed.size(); i++)
        {
            pos_per_vertex[_layer_fixed[i]] = i;
        }

        // tuple: position, degree, parity, idx
        std::vector<std::tuple<int, int, int, int>> medians( _layer.size() );
        for (int i = 0; i < _layer.size(); i++)
        {
            auto& neighbors = _get_neighbors( *vertices[_layer[i]] );
            int degree = neighbors.size();
            if (degree == 0)
            {
                medians[i] = { 0, 0, 0, i };
                continue;
            }

            // sort neighbors in position in _layer_fixed
            std::vector<int> neighbor_positions( neighbors.size() );
            for (int j = 0; j < neighbors.size(); j++)
            {
                neighbor_positions[j] = pos_per_vertex[neighbors[j].idx];
            }
            std::sort( neighbor_positions.begin(), neighbor_positions.end() );

            int median_idx = std::ceil( degree * .5f ) - 1;
            medians[i] = { neighbor_positions[median_idx], degree, degree % 2, i };
        }

        bool changed = false;
        std::sort( medians.begin(), medians.end(), [&]( std::tuple<int, int, int, int> lhs, std::tuple<int, int, int, int> rhs ) {
            bool eval;
            // if o(v_1) == o(v_2)
            if (std::get<0>( lhs ) == std::get<0>( rhs ))
            {
                // check if they have the same degree
                if (std::get<1>( lhs ) == std::get<1>( rhs ))
                {
                    // arbitrary so just false
                    eval = false;
                }
                else
                {
                    // different degree, so odd parity to the left
                    eval = std::get<2>( lhs ) > std::get<2>( rhs );
                }
            }
            else
            {
                eval = std::get<0>( lhs ) < std::get<0>( rhs );
            }
            changed |= eval;
            return eval;
            } );

        _new_layer.resize( _layer.size() );
        for (int i = 0; i < _layer.size(); i++) _new_layer[i] = _layer[std::get<3>( medians[i] )];
        return changed;
    }

    int SugiyamaLayout::Crossings(const Dataset& _dataset, const Layer& _layer_1, const Layer& _layer_2)
    {
        std::vector<bool> flags;
        std::vector<std::pair<int, int>> open_edges;
        int count = 0;
        // First open all edges
        for (int i = 0; i < _layer_1.size(); i++)
        {
            int vertex_idx = _layer_1[i];
            auto& neighbors = _dataset.vertices[vertex_idx]->outgoing_neighbors;

            for (auto& n : neighbors)
                open_edges.emplace_back(vertex_idx, n.idx);
        }
        flags.resize(open_edges.size(), false);

        // Then close all edges
        for (int i = _layer_2.size() - 1; i >= 0; i--)
        {
            int vertex_idx = _layer_2[i];
            auto& neighbors = _dataset.vertices[vertex_idx]->incoming_neighbors;
            for (auto& n : neighbors)
            {
                std::pair<int, int> edge(n.idx, vertex_idx);
                // Backwards linear scan
                for (int j = open_edges.size() - 1; j >= 0; j--)
                {
                    if (flags[j]) continue;
                    // Encountered ourselves
                    if (open_edges[j] == edge)
                    {
                        // Remove ourself from open_edges
                        flags[j] = true;
                        break;
                    }

                    if (open_edges[j].first != edge.first && open_edges[j].second != edge.second)
                    {
                        // Found a crossing
                        count++;
                    }
                }
            }
        }

        return count;
    }

    //--------------------------------------------------------------
    // Vertex Positioning
    //--------------------------------------------------------------
    std::vector<float> SugiyamaLayout::VertexPositioning(
        const Dataset& _dataset,
        const std::vector<Layer>& _vertices_per_layer,
        Layer& _layer_per_vertex,
        float _delta)
    {
        const auto& vertices = _dataset.vertices;
        std::vector<int> pos_per_vertex(vertices.size());
        for (auto& layer : _vertices_per_layer)
        {
            for (int i = 0; i < layer.size(); i++)
            {
                pos_per_vertex[layer[i]] = i;
            }
        }

        std::vector<std::pair<int, int>> flags;
        std::vector<int> root, align;
        std::vector<float> x_per_vertex;

        //FlagType1Conflicts( _dataset, _vertices_per_layer, _layer_per_vertex, flags );
        VerticalAlignment(_dataset, _vertices_per_layer, _layer_per_vertex, pos_per_vertex, root, align, flags);
        HorizontalCompaction(_dataset, _vertices_per_layer, _layer_per_vertex, pos_per_vertex, root, align,
                             x_per_vertex, _delta);

        return x_per_vertex;
    }

    void SugiyamaLayout::FlagType1Conflicts(
        const Dataset& _dataset,
        const std::vector<Layer>& _vertices_per_layer,
        Layer& _layer_per_vertex,
        std::vector<std::pair<int, int>>& _flags)
    {
        auto& vertices = _dataset.vertices;
        for (int i = 1; i < _vertices_per_layer.size() - 1; i++)
        {
            int k_0 = 0;
            int l = 1;

            for (int l_1 = 0; l_1 < _vertices_per_layer[i + 1].size(); l_1++)
            {
                int vertex_id = _vertices_per_layer[i + 1][l_1];
                if (l_1 == _vertices_per_layer[i + 1].size() - 1);
            }
        }
    }

    void SugiyamaLayout::VerticalAlignment(
        const Dataset& _dataset,
        const std::vector<Layer>& _vertices_per_layer,
        Layer& _layer_per_vertex,
        const Layer& _pos_per_vertex,
        std::vector<int>& _root,
        std::vector<int>& _align,
        std::vector<std::pair<int, int>>& _flags)
    {
        const auto& vertices = _dataset.vertices;
        _root.resize(vertices.size());
        _align.resize(vertices.size());
        for (int i = 0; i < vertices.size(); i++)
        {
            _root[i] = i;
            _align[i] = i;
        }

        for (size_t i = 0; i < _vertices_per_layer.size(); i++)
        {
            int rank = -1;
            auto& current_layer = _vertices_per_layer[i];
            for (size_t k = 0; k < current_layer.size(); k++)
            {
                int vertex_idx = current_layer[k];
                int d = vertices[vertex_idx]->outgoing_neighbors.size();
                if (d <= 0) continue;
                int ms[2] = {
                    static_cast<int>(std::floorf((d + 1) * 0.5)),
                    static_cast<int>(std::ceilf((d + 1) * 0.5))
                };

                // Sort neighbors on horizontal position
                auto& neighbors = vertices[vertex_idx]->outgoing_neighbors;
                std::sort(neighbors.begin(), neighbors.end(), [&](Neighbor lhs, Neighbor rhs)
                {
                    return _pos_per_vertex[lhs.idx] < _pos_per_vertex[rhs.idx];
                });

                for (int m : ms)
                {
                    int u_m = vertices[vertex_idx]->outgoing_neighbors[m - 1].idx;

                    if (_align[vertex_idx] not_eq vertex_idx) continue;
                    // If not flagged
                    if (std::find(_flags.begin(), _flags.end(), std::pair<int, int>(u_m, vertex_idx)) != _flags.end())
                        continue;
                    if (rank >= _pos_per_vertex[u_m]) continue;

                    _align[u_m] = vertex_idx;
                    _root[vertex_idx] = _root[u_m];
                    _align[vertex_idx] = _root[vertex_idx];
                    rank = _pos_per_vertex[u_m];
                }
            }
        }
    }

    void SugiyamaLayout::HorizontalCompaction(
        const Dataset& _dataset,
        const std::vector<Layer>& _vertices_per_layer,
        const Layer& _layer_per_vertex,
        const Layer& _pos_per_vertex,
        const std::vector<int>& _root,
        const std::vector<int>& _align,
        std::vector<float>& _x_per_vertex,
        float _delta)
    {
        constexpr float undefined = MIN_FLOAT;
        constexpr float infinite = MAX_FLOAT;
        const auto& vertices = _dataset.vertices;
        std::vector<float> sink(vertices.size());
        for (int i = 0; i < vertices.size(); i++)
            sink[i] = i;
        std::vector<float> shift(vertices.size(), infinite);
        _x_per_vertex.resize(vertices.size(), undefined);

        std::function<void(int)> place_block = [&](int v)
        {
            if (_x_per_vertex[v] != undefined) return;
            _x_per_vertex[v] = 0;
            int w = v;
            do
            {
                const int w_pos = _pos_per_vertex[w];
                if (w_pos > 0)
                {
                    const int layer = _layer_per_vertex[w];
                    const int pred_w = _vertices_per_layer[layer][w_pos - 1];
                    int u = _root[pred_w];
                    place_block(u);
                    if (sink[v] == v) sink[v] = sink[u];
                    if (sink[v] == sink[u])
                        _x_per_vertex[v] = std::max(_x_per_vertex[v], _x_per_vertex[u] + _delta);
                }
                w = _align[w];
            }
            while (w != v);

            while (_align[w] != v)
            {
                w = _align[w];
                _x_per_vertex[w] = _x_per_vertex[v];
                sink[w] = sink[v];
            }
        };

        for (size_t i = 0; i < vertices.size(); i++)
        {
            const int vertex_idx = vertices[i]->idx;
            if (_root[vertex_idx] == vertex_idx) place_block(vertex_idx);
        }

        for (size_t i = 0; i < _vertices_per_layer.size(); i++)
        {
            const int v_1 = _vertices_per_layer[i][0];
            if (sink[v_1] != v_1) continue;
            if (shift[sink[v_1]] == infinite) shift[sink[v_1]] = 0;

            int j = i;
            int k = 0;
            int v = undefined;
            do
            {
                v = _vertices_per_layer[j][k];
                while (_align[v] != _root[v])
                {
                    v = _align[v];
                    j--;
                    if (_pos_per_vertex[v] > 0)
                    {
                        const int layer = _layer_per_vertex[v];
                        const int u = _vertices_per_layer[layer][_pos_per_vertex[v] - 1];
                        shift[sink[u]] = std::min(shift[sink[u]],
                                                  shift[sink[v]] + _x_per_vertex[v] - (_x_per_vertex[u] + _delta));
                    }
                }
                k = _pos_per_vertex[v] + 1;
            }
            while (k < _vertices_per_layer[j].size() && sink[v] == sink[_vertices_per_layer[j][k]]);
        }

        for (int i = 0; i < vertices.size(); i++)
            _x_per_vertex[i] = _x_per_vertex[i] + shift[sink[i]];
    }
} // namespace DataVis
