#include "precomp.h"

namespace DataVis
{

//--------------------------------------------------------------
void Graph::Init(const std::shared_ptr<Dataset> _dataset)
{
    IStructure::Init(_dataset);
    Load(_dataset);
    InitEdges();
    // Add layout
    m_layouts.push_back(std::make_unique<ForceDirectedLayout>());
    m_layouts.push_back(std::make_unique<Sugiyama>());
}

//--------------------------------------------------------------
void Graph::Load(const std::shared_ptr<Dataset> _dataset)
{
    dataset = _dataset;
    nodes.clear();
    nodes.reserve(_dataset->vertices.size());
    auto& vertices = _dataset->vertices;
    // Add nodes
    for (size_t i = 0; i < _dataset->vertices.size(); i++)
    {
        const auto& vertex = vertices[i];
        nodes.push_back(std::make_shared<Node>(vertex->id, i));
    }
}

//--------------------------------------------------------------
void Graph::DrawNodes()
{
    ofFill();
    ofSetColor(ImGuiExtensions::Vec3ToOfColor(m_gui_data.coloredit_edge_color));

    for (const auto& edge : edges)
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
