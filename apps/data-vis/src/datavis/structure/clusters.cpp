#include "precomp.h"

namespace DataVis
{
void Clusters::Init(const std::shared_ptr<Dataset> _dataset)
{

}

bool Clusters::InsideDraggable(const glm::vec3& _pos)
{
    glm::vec3 transformed = _pos - m_position;
    if(IStructure::InsideDraggable(transformed))
    {
        m_focussed_graph = nullptr;
        return true;
    }
        

    for(auto& graph : m_sub_graphs)
    {
        if(graph->InsideDraggable(transformed))
        {
            m_dragging_graph = graph;
            return true;
        }
    }
    return false;   
}

void Clusters::Move(const glm::vec3& _offset)
{
    if(m_dragging_graph)
    {
        m_dragging_graph->Move(_offset);
        return;
    }
    IStructure::Move(_offset);
}

void Clusters::Select(const glm::vec3& _pos)
{
    glm::vec3 transformed = _pos - m_position;
    if(m_focussed_graph)
    {
        m_focussed_graph->Select(transformed);
        return;
    }
    
    for(auto& graph : m_sub_graphs)
    {
        if(graph->Inside(transformed))
        {
            m_focussed_graph = graph;
            return;
        }
    }
}

void Clusters::Gui()
{
    if(m_focussed_graph)
    {
        m_focussed_graph->Gui();
        return;
    }

    ImGui::Begin("Clusters Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button("Delete"))
    {
        ImGui::End();
        m_on_delete_callback(*this);
        return;
    }

    dataset->InfoGui();

    if (ImGui::TreeNode("Settings"))
    {
        constexpr int color_edit_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop;
        ImGuiExtensions::ColorEdit3("Inter Edge Color", m_gui_data.coloredit_inter_edge_color, color_edit_flags);

        ImGui::TreePop();
    }

    ImGui::Separator();
    // Loop over all layout gui's
    for (const auto& layout : m_layouts)
    {
        if (layout->Gui(*this)) m_active_layout = layout;
    }

    ImGui::End();
}

void Clusters::DrawNodes()
{
    for(auto& graph : m_sub_graphs)
    {
        graph->Draw(graph == m_focussed_graph);
    }
    
    ofSetColor(ImGuiExtensions::Vec3ToOfColor(m_gui_data.coloredit_inter_edge_color));
    for(auto& edge : inter_edges)
        edge->Draw();
}
} // namespace DataVis