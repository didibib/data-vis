#include "precomp.h"
#include "ofApp.h"

namespace DataVis
{
void Graph::Init(const std::shared_ptr<Dataset> _dataset)
{
	m_dataset = _dataset;
	m_nodes.clear();
	m_nodes.reserve(_dataset->GetVertices().size());
	auto& vertices = _dataset->GetVertices();
	// Add nodes
	for (size_t i = 0; i < _dataset->GetVertices().size(); i++) {
		auto& vertex = vertices[i];
		m_nodes.push_back(std::make_shared<Node>(vertex.id, i));
	}
	// Add neighbors
	for (size_t i = 0; i < m_nodes.size(); i++) {
		auto& vertex = vertices[i];
		if (vertex.neighbors->empty()) continue;
		for (const auto& n : *vertex.neighbors) {
			m_nodes[i]->neighbors.push_back(m_nodes[n.to_idx]);
		}
	}
}

IStructure::VectorOfNodes& Graph::GetNodes()
{
	return m_nodes;
}

//--------------------------------------------------------------
void Graph::HandleInput()
{
}

//--------------------------------------------------------------
void Graph::Select(const glm::vec3& _position)
{
}

//--------------------------------------------------------------
void Graph::Update(float delta_time)
{
	if(m_imgui_data.button_fd_enabled)
		Optimizer::ForceDirected(*this, m_imgui_data.input_fd_C, m_imgui_data.input_fd_t, m_imgui_data.input_fd_iter_per_frame);
}

//--------------------------------------------------------------
void Graph::DrawLayout()
{
	ofFill();
	ofSetColor(123);
	for (const auto& edge : m_dataset->GetEdges()) {
		auto const& startIdx = edge.from_idx;
		auto const& endIdx = edge.to_idx;
		glm::vec3 start = m_nodes[startIdx]->GetPosition();
		glm::vec3 end = m_nodes[endIdx]->GetPosition();
		// Draw edge behind nodes
		start--;
		end--;
		ofDrawLine(start, end);
	}

	ofSetColor(10);
	ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
	for (const auto& node : m_nodes) {
		glm::vec3 pos = node->GetPosition();
		ofDrawCircle(pos, radius);
	}
}

//--------------------------------------------------------------
void Graph::Gui()
{
	ImGui::Begin("Graph Settings");

	//--------------------------------------------------------------
	// Optimize Layout
	//--------------------------------------------------------------
	if (ImGui::TreeNode("Local Search")) {
		ImGui::InputInt("# of iterations", &(m_imgui_data.input_optimize_iterations));
		if (ImGui::Button("Optimize Graph"))
		{
			DataVis::Optimizer::LocalSearch(*this, m_imgui_data.input_optimize_iterations);
		}

		ImGui::TreePop();
		ImGui::Separator();
	}

	if (ImGui::TreeNode("Force Directed"))
	{
		ImGui::Checkbox("Enabled", &m_imgui_data.button_fd_enabled);
		ImGui::InputInt("Iterations per frame", &(m_imgui_data.input_fd_iter_per_frame));
		ImGui::InputFloat("C", &(m_imgui_data.input_fd_C));
		ImGui::InputFloat("t", &(m_imgui_data.input_fd_t));

		ImGui::TreePop();
		ImGui::Separator();
	}

	if (ImGui::Button("Delete"))
	{
		/*auto app = m_app.lock();
		app.get()->DeleteStructure(*this);*/
		m_on_delete_callback(*this);
	}
	ImGui::End();
}

//--------------------------------------------------------------

} // namespace DataVis