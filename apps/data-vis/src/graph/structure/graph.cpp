#include "precomp.h"

namespace DataVis
{
void Graph::Init(const std::shared_ptr<Dataset> _dataset)
{
	IStructure::Init(_dataset);
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

	// Add layout
	m_layouts.push_back(std::make_unique<ForceDirected>());
}

//--------------------------------------------------------------
void Graph::DrawNodes()
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

	ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
	for (const auto& node : m_nodes) {
		glm::vec3 pos = node->GetPosition();
		ofSetColor(node->color);
		ofDrawCircle(pos, radius);
	}
}

} // namespace DataVis