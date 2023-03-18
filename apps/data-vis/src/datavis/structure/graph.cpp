#include "precomp.h"

namespace DataVis
{
    void Graph::Init(const std::shared_ptr<Dataset> _dataset)
    {
        IStructure::Init(_dataset);
        Load(_dataset);

        // Add layout
        m_layouts.push_back(std::make_unique<ForceDirectedLayout>());
        m_layouts.push_back(std::make_unique<Sugiyama>());
    }

    void Graph::Load(const std::shared_ptr<Dataset> _dataset)
    {
        dataset = _dataset;
        nodes.clear();
        nodes.reserve(_dataset->vertices.size());
        auto& vertices = _dataset->vertices;
        // Add nodes
        for (size_t i = 0; i < _dataset->vertices.size(); i++)
        {
            auto& vertex = vertices[i];
            nodes.push_back(std::make_shared<Node>(vertex.id, i));
        }
        // Add neighbors
        for (size_t i = 0; i < nodes.size(); i++)
        {
            auto& vertex = vertices[i];
            if (vertex.neighbors.empty()) continue;
            for (const auto& n : vertex.neighbors)
            {
                nodes[i]->neighbors.push_back(nodes[n.idx]);
            }
        }
    }

    //--------------------------------------------------------------
    void Graph::DrawNodes()
    {
        ofFill();
        ofSetColor(m_gui_data.coloredit_edge_color.x, m_gui_data.coloredit_edge_color.y, m_gui_data.coloredit_edge_color.z);

        //for (const auto& edge : dataset->edges)
        //{
        //    auto const& startIdx = edge.from_idx;
        //    auto const& endIdx = edge.to_idx;
        //    glm::vec3 start = nodes[startIdx]->GetPosition();
        //    glm::vec3 end = nodes[endIdx]->GetPosition();
        //    glm::vec3 dir = normalize(end - start);
        //    start += dir * nodes[startIdx]->GetRadius();
        //    end -= dir * nodes[endIdx]->GetRadius();
        //
        //    // Draw edge behind nodes
        //    --start;
        //    --end;
        //    if (dataset->GetKind() == Dataset::Kind::Undirected)
        //    {
        //        ofDrawLine(start, end);
        //    }
        //    else if (dataset->GetKind() == Dataset::Kind::Directed)
        //    {
        //        end -= dir * nodes[endIdx]->GetRadius();
        //        ofDrawArrow(start, end, nodes[endIdx]->GetRadius());
        //    }
        //}
         for(const auto& edge : edges)
         {
             edge->Draw();
         }
        
        for (const auto& node : nodes)
        {
            if (node->GetVertexId() == "dummy") continue;
            node->Draw();
        }
    }
} // namespace DataVis
