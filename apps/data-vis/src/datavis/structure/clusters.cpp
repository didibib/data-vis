#include "precomp.h"

namespace DataVis
{
void Clusters::Init(const std::shared_ptr<Dataset> _dataset)
{
    dataset = _dataset;
    dataset_clusters = std::dynamic_pointer_cast<DatasetClusters>(_dataset);
    
    int count = 0;
    for(const auto& cluster : dataset_clusters->clusters)
    {
        std::shared_ptr<Graph> graph = make_shared<Graph>();
        graph->Init(cluster);
        m_sub_graphs.emplace_back(graph);
        RandomLayout::Apply(*graph, 800, 600);
        graph->SetPosition(glm::vec3(count * 800, count++ * 600, 0));
        
        for(auto& node : graph->nodes)
        {
            nodes.push_back(node);
        }
    }
    UpdateEdges();
    UpdateAABB();
    // Add edge bundling layout
}

void Clusters::Update(const float _delta_time)
{
    for(const auto& graph : m_sub_graphs)
        graph->Update(_delta_time);
    IStructure::Update(_delta_time);
}

void Clusters::UpdateEdges()
{
    edges.clear();
    edges.resize(dataset->edges.size());
    // Loop over inter edges
    for (int i = 0; i < dataset->edges.size(); i++)
    {
        const auto& edge = dataset->edges[i];
        auto const& start_vertex = dataset->vertices[edge.from_idx];
        auto const& end_vertex = dataset->vertices[edge.to_idx];
        auto const& start_graph = FindSubGraph(start_vertex->owner);
        auto const& end_graph = FindSubGraph(end_vertex->owner);

        glm::vec3 start = start_graph->nodes[start_vertex->idx]->GetNewPosition() + start_graph->GetPosition();
        glm::vec3 end = end_graph->nodes[end_vertex->idx]->GetNewPosition() + end_graph->GetPosition();

        edges[i] = std::make_shared<EdgePath>();
        const auto& edge_path = edges[i];
        edge_path->AddPoint(start);
        edge_path->AddPoint(end);
        edge_path->SetArrowOffset(end_graph->nodes[end_vertex->idx]->GetRadius() * 2);
        if (dataset->GetKind() == Dataset::Kind::Directed)
            edge_path->SetIsDirected(true);
    }
}

void Clusters::UpdateAABB()
{
    glm::vec3 tl{MAX_FLOAT};
    glm::vec3 br{-MAX_FLOAT};

    for (const auto& node : nodes)
    {
        auto& owner = FindSubGraph(dataset_clusters->vertices[node->GetVertexIdx()]->owner);
        glm::vec3 global_pos = node->GetNewPosition() + owner->GetPosition();
        tl.x = min(global_pos.x - node->GetRadius(), tl.x);
        tl.y = min(global_pos.y - node->GetRadius(), tl.y);
        br.x = max(global_pos.x + node->GetRadius(), br.x);
        br.y = max(global_pos.y + node->GetRadius(), br.y);
    }
    tl.z = 0;
    br.z = 0;
    m_aabb.SetNewBounds(tl, br);
}

bool Clusters::InsideDraggable(const glm::vec3& _pos)
{
    glm::vec3 transformed = _pos - m_position;
    if(IStructure::InsideDraggable(transformed))
    {
        m_dragging_graph = nullptr;
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
    if(m_focussed_graph && m_focussed_graph->Inside(transformed))
    {
        m_focussed_graph->Select(transformed);
        return;
    }
    
    m_focussed_graph = nullptr;
    for(auto& graph : m_sub_graphs)
    {
        if(graph->Inside(transformed))
        {
            m_focussed_graph = graph;
            return;
        }
    }
}

std::shared_ptr<Graph> Clusters::FindSubGraph(const std::string _id)
{
    for(auto& graph : m_sub_graphs)
    {
        if(graph->dataset->GetId() == _id) return graph;
    }
    return nullptr;
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
        if (ImGuiExtensions::ColorEdit3("Node Color", m_gui_data.coloredit_node_color, color_edit_flags))
        {
            SetNodesColor(ImGuiExtensions::Vec3ToOfColor(m_gui_data.coloredit_node_color));
        }
        if (ImGuiExtensions::ColorEdit3("Intra Edge Color", m_gui_data.coloredit_intra_edge_color, color_edit_flags))
        {
            for(auto& graph : m_sub_graphs) graph->SetEdgeColor(m_gui_data.coloredit_intra_edge_color);
        }
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