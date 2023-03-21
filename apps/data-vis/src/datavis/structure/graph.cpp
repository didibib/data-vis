#include "precomp.h"

namespace DataVis
{

//--------------------------------------------------------------
void Graph::Init(const std::shared_ptr<Dataset> _dataset)
{
    IStructure::Init(_dataset);
    Load(_dataset);
    // Add layout
    m_layouts.push_back(std::make_unique<ForceDirectedLayout>());
    m_layouts.push_back(std::make_unique<Sugiyama>());
}

//--------------------------------------------------------------
void Graph::Load(const std::shared_ptr<Dataset> _dataset)
{
    dataset = _dataset;
    InitNodes();
    InitEdges();
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
        //if (node->GetVertexId() == "dummy") continue;
        node->Draw();
    }
}
} // namespace DataVis
