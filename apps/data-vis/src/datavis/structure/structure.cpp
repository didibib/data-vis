#include "precomp.h"

namespace DataVis
{
int IStructure::__idx = 0;
//--------------------------------------------------------------
// IStructure
//--------------------------------------------------------------
IStructure::IStructure()
{
    m_idx = __idx++;
}

IStructure::~IStructure()
{
    __idx--;
}

void IStructure::Init(const std::shared_ptr<Dataset> _dataset)
{
    dataset = _dataset;
    InitNodes();
    InitEdges();
    m_layouts.clear();
    m_layouts.push_back(std::make_unique<RandomLayout>());
    m_layouts.push_back(std::make_unique<GridLayout>());
}

const int& IStructure::Idx() const
{
    return m_idx;
}

void IStructure::Update(const float _delta_time)
{
    m_aabb.Update(_delta_time);
    for (const auto& node : nodes)
        node->Update(_delta_time);
    for (const auto& edge : edges)
        edge->Update(_delta_time);
    if (m_active_layout)
        m_active_layout->Update(_delta_time);
}

//--------------------------------------------------------------
// Position
//--------------------------------------------------------------
const glm::vec3& IStructure::GetPosition() const
{
    return m_position;
}

//--------------------------------------------------------------
void IStructure::SetPosition(const glm::vec3& _position)
{
    m_position = _position;
}

//--------------------------------------------------------------
// Edges
//--------------------------------------------------------------
void IStructure::UpdateEdges(const bool _force)
{
    for (int i = 0; i < dataset->edges.size(); i++)
    {
        const auto& edge = dataset->edges[i];
        auto const& startIdx = edge.from_idx;
        auto const& endIdx = edge.to_idx;
        glm::vec3 start = nodes[startIdx]->GetNewPosition();
        glm::vec3 end = nodes[endIdx]->GetNewPosition();

        const auto& edge_path = edges[i];
        edge_path->UpdateStartPoint(start);
        edge_path->UpdateEndPoint(end);
        if(_force) edge_path->ForceUpdate();
        edge_path->SetArrowOffset(nodes[endIdx]->GetRadius() * 2);
    }
}

//--------------------------------------------------------------
void IStructure::InitEdges()
{
    edges.clear();
    edges.reserve(dataset->edges.size());
    for (int i = 0; i < dataset->edges.size(); i++)
    {
        const auto& edge = dataset->edges[i];
        auto const& startIdx = edge.from_idx;
        auto const& endIdx = edge.to_idx;
        glm::vec3 start = nodes[startIdx]->GetNewPosition();
        glm::vec3 end = nodes[endIdx]->GetNewPosition();

        const auto& edge_path = edges.emplace_back(
            std::make_shared<EdgePath>(edge.idx, dataset->GetKind()));
        edge_path->AddPoint(start);
        edge_path->AddPoint(end);
        edge_path->SetArrowOffset(nodes[endIdx]->GetRadius() * 2);
    }
}

//--------------------------------------------------------------
// Nodes
//--------------------------------------------------------------
void IStructure::InitNodes()
{
    nodes.clear();
    nodes.reserve(dataset->vertices.size());
    const auto& vertices = dataset->vertices;
    // Add nodes
    for (size_t i = 0; i < dataset->vertices.size(); i++)
    {
        const auto& vertex = vertices[i];
        nodes.push_back(std::make_shared<Node>(vertex->id, i));
    }
}

//--------------------------------------------------------------
// Interaction
//--------------------------------------------------------------
void IStructure::Move(const glm::vec3& _offset)
{
    m_position += _offset;
}

void IStructure::Select(const glm::vec3& _position)
{
    const glm::vec3 transformed = _position - m_position;

    for (const auto& node : nodes)
    {
        if (node->Inside(transformed))
        {
            SetSelectedNode(node);
            return;
        }
    }
    SetSelectedNode(nullptr);
}

void IStructure::SetSelectedNode(const std::shared_ptr<Node>& _node)
{
    if (m_selected_node != nullptr)
        m_selected_node->color = ofColor::black;
    if (_node != nullptr) _node->color = ofColor::green;
    m_selected_node = _node;
}

//--------------------------------------------------------------
// Bounding Box
//--------------------------------------------------------------
void IStructure::UpdateAABB()
{
    glm::vec3 tl{MAX_FLOAT};
    glm::vec3 br{-MAX_FLOAT};

    for (const auto& node : nodes)
    {
        tl.x = min(node->GetNewPosition().x - node->GetRadius(), tl.x);
        tl.y = min(node->GetNewPosition().y - node->GetRadius(), tl.y);
        br.x = max(node->GetNewPosition().x + node->GetRadius(), br.x);
        br.y = max(node->GetNewPosition().y + node->GetRadius(), br.y);
    }
    tl.z = 0;
    br.z = 0;
    m_aabb.SetNewBounds(tl, br);
}

const AABB& IStructure::GetAABB()
{
    return m_aabb;
}

//--------------------------------------------------------------
bool IStructure::Inside(const glm::vec3& _position) const
{
    return m_aabb.Inside(_position - m_position);
}

//--------------------------------------------------------------
bool IStructure::InsideDraggable(const glm::vec3& _position)
{
    return m_aabb.InsideDraggable(_position - m_position);
}

//--------------------------------------------------------------
float IStructure::GetArea() const
{
    return m_aabb.GetArea();
}

//--------------------------------------------------------------
// Callback
//--------------------------------------------------------------
void IStructure::SetOnDeleteCallback(const std::function<void(IStructure&)>& _callback)
{
    m_on_delete_callback = _callback;
}

void IStructure::SetNodesColor(ofColor _color)
{
    for (auto& node : nodes)
    {
        node->color = _color;
    }
}

void IStructure::SetEdgeColor(const glm::vec3& _color)
{
    m_gui_data.coloredit_edge_color = _color;
}

//--------------------------------------------------------------
// Gui
//--------------------------------------------------------------
void IStructure::Gui()
{
    ImGui::Begin("Structure Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (m_on_delete_callback && ImGui::Button("Delete"))
    {
        ImGui::End();
        m_on_delete_callback(*this);
        return;
    }

    dataset->InfoGui();

    if (ImGui::TreeNode("Settings"))
    {
        ImGui::Checkbox("Draw Label", &m_gui_data.checkbox_node_labels);
        if (ImGui::SliderFloat("Radius", &m_gui_data.slider_radius, 10, 30))
        {
            for (const auto& node : nodes)
                node->SetRadius(m_gui_data.slider_radius);
        }

        constexpr int color_edit_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop;
        if (ImGuiExtensions::ColorEdit3("Node Color", m_gui_data.coloredit_node_color, color_edit_flags))
        {
            SetNodesColor(ImGuiExtensions::Vec3ToOfColor(m_gui_data.coloredit_node_color));
        }

        ImGuiExtensions::ColorEdit3("Edge Color", m_gui_data.coloredit_edge_color, color_edit_flags);

        ImGui::TreePop();
    }

    ImGui::Separator();
    // Loop over all layout gui's
    for (const auto& layout : m_layouts)
    {
        if (layout->Gui(*this)) m_active_layout = layout;
    }

    // Call extra gui
    NodeInfoGui();

    ImGui::End();
}

//--------------------------------------------------------------
void IStructure::NodeInfoGui()
{
    if (m_selected_node != nullptr)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::BeginChild("SelectedNode", ImVec2(0, 70), true);

        ImGui::Text("Selected Node:");
        ImGui::Text("Vertex: %s", m_selected_node->GetVertexId().c_str());
        ImGui::Text("Position: (%f.0, %f.0)", m_selected_node->GetPosition().x, m_selected_node->GetPosition().y);

        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
}

//--------------------------------------------------------------
// Rendering
//--------------------------------------------------------------
void IStructure::Draw(bool _is_focussed)
{
    ofPushMatrix();
    ofTranslate(m_position);

    // Draw the bounds
    m_aabb.Draw(_is_focussed);

    if (m_active_layout)
        m_active_layout->Draw();

    // Draw the actual nodes and edges
    DrawNodes();

    //ofSetDrawBitmapMode( );
    if (m_gui_data.checkbox_node_labels)
    {
        for (const auto& node : nodes)
            ofDrawBitmapStringHighlight(ofToString(node->GetVertexId()), node->GetPosition() + glm::vec3(10, 10, -1));
    }
    ofPopMatrix();
}
} // DataVis
