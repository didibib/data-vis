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

void IStructure::Init(std::shared_ptr<Dataset> _dataset)
{
	m_dataset = _dataset;

	m_layouts.push_back(std::make_unique<Random>());
	m_layouts.push_back(std::make_unique<Grid>());
}

const int& IStructure::Idx() const
{
	return m_idx;
}

void IStructure::Update(float _delta_time)
{
	for (auto& node : m_nodes)
		node->EaseInEaseOut(_delta_time);
}
//--------------------------------------------------------------
// Get
//--------------------------------------------------------------
Dataset& IStructure::GetDataset() const
{
	return *m_dataset;
}

IStructure::VectorOfNodes& IStructure::GetNodes()
{
	return m_nodes;
}

//--------------------------------------------------------------
// Position
//--------------------------------------------------------------
glm::vec3 IStructure::GetPosition() const
{
	return m_position;
};

void IStructure::SetPosition(glm::vec3 _new_position)
{
	m_position = _new_position;
};

//--------------------------------------------------------------
// Displacement
//--------------------------------------------------------------
const glm::vec3& IStructure::Node::GetDisplacement()
{
	return m_displacement;
}

void IStructure::Node::SetDisplacement(glm::vec3 _displacement)
{
	m_displacement = _displacement;
}

//--------------------------------------------------------------
// Interaction
//--------------------------------------------------------------
void IStructure::Move(glm::vec3 _offset)
{
	m_position += _offset;
};

void IStructure::Select(const glm::vec3& _position)
{
	glm::vec3 transformed = _position - m_position;

	for (auto& node : m_nodes)
	{
		if (node->Inside(transformed))
		{
			SetSelectedNode(node);
			return;
		}
	}
	SetSelectedNode(nullptr);
}

void IStructure::SetSelectedNode(std::shared_ptr<Node> _node)
{
	if (m_selected_node != nullptr)
		m_selected_node->color = ofColor::black;
	if (_node != nullptr) _node->color = ofColor::green;
	m_selected_node = _node;
}

//--------------------------------------------------------------
// Bounding Box
//--------------------------------------------------------------
const ofRectangle& IStructure::GetAABB() const
{
	return m_aabb;
};

const ofRectangle& IStructure::GetMoveAABB() const
{
	return m_move_aabb;
};

void IStructure::SetAABB()
{
	glm::vec3 tl{ 1e30 };
	glm::vec3 br{ -1e30 };

	for (const auto& node : GetNodes())
	{
		/*tl.x = min(node->GetPosition().x - node->GetRadius(), tl.x);
		tl.y = min(node->GetPosition().y - node->GetRadius(), tl.y);
		br.x = max(node->GetPosition().x + node->GetRadius(), br.x);
		br.y = max(node->GetPosition().y + node->GetRadius(), br.y);*/

		tl.x = min(node->GetNewPosition().x - node->GetRadius(), tl.x);
		tl.y = min(node->GetNewPosition().y - node->GetRadius(), tl.y);
		br.x = max(node->GetNewPosition().x + node->GetRadius(), br.x);
		br.y = max(node->GetNewPosition().y + node->GetRadius(), br.y);
	}
	tl.z = 0;
	br.z = 0;
	m_aabb = { tl, br };
}

void IStructure::UpdateAABB()
{
	SetAABB();
	SetMoveAABB();
};

void IStructure::SetMoveAABB()
{
	auto bb_tl = m_aabb.getTopLeft();
	m_move_aabb = { bb_tl, { bb_tl.x - m_move_aabb_size, bb_tl.y - m_move_aabb_size } };
}

bool IStructure::InsideAABB(glm::vec3 _position)
{
	return m_aabb.inside(_position - m_position);
}

bool IStructure::InsideMoveAABB(glm::vec3 _position)
{
	return m_move_aabb.inside(_position - m_position);
}

void IStructure::SetOnDeleteCallback(std::function<void(IStructure&)> _callback)
{
	m_on_delete_callback = _callback;
}

void IStructure::Gui()
{
	ImGui::Begin("Structure Settings");

	if (ImGui::Button("Delete"))
	{
		ImGui::End();
		m_on_delete_callback(*this);
		return;
	}

	if (ImGui::TreeNode("Node Settings"))
	{
		ImGui::Checkbox("Draw Label", &m_gui_data.checkbox_node_labels);
		if (ImGui::SliderFloat("Radius", &m_gui_data.slider_radius, 10, 30))
		{
			for (auto& node : m_nodes)
				node->SetRadius(m_gui_data.slider_radius);
		}
		ImGui::TreePop();
	}

	// Loop over all layout gui's
	for (auto& layout : m_layouts)
	{
		if (layout->Gui(*this)) m_active_layout = layout;
	}

	// Call extra gui
	NodeInfoGui();

	ImGui::End();
}

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
	ofNoFill();
	if (_is_focussed)
		ofSetColor(ofColor::green);
	else
		ofSetColor(ofColor::lightGray);
	ofDrawRectangle(m_aabb);

	ofFill();
	ofSetColor(ofColor::black);
	ofDrawRectangle(m_move_aabb);
	ofNoFill();

	if (m_active_layout) m_active_layout->Draw();

	// Draw the actual nodes and edges
	DrawNodes();

	if (m_gui_data.checkbox_node_labels)
	{
		for (auto& node : m_nodes)
			ofDrawBitmapStringHighlight(ofToString(node->GetVertexId()), node->GetPosition() + glm::vec3(10, 10, -1));

	}

	ofPopMatrix();
}


} // DataVis