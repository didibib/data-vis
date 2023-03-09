#include "precomp.h"

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

//--------------------------------------------------------------
void Graph::HandleInput()
{
}

void Graph::Select(const glm::vec3& _position)
{
}

void Graph::Update(float delta_time)
{

	Graph::Layout::Force( *this, .5f, 10 );
}

void Graph::DrawLayout()
{
	ofFill();
	ofSetColor(123);
	// TODO
	for (const auto& edge : m_dataset->GetEdges()) {
		auto const& startIdx = edge.from_idx;
		auto const& endIdx = edge.to_idx;
		glm::vec3 start = m_nodes[startIdx]->GetPosition();
		glm::vec3 end = m_nodes[endIdx]->GetPosition();
		// Draw edge behind nodes
		start -= 10;
		end -= 10;
		ofDrawLine(start, end);
	}

	ofSetColor(10);
	ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
	for (const auto& node : m_nodes) {
		glm::vec3 pos = node->GetPosition();
		ofDrawCircle(pos, radius);
	}
}

void Graph::Gui()
{
}

std::vector<std::shared_ptr<IStructure::Node>>& Graph::GetNodes()
{
	return m_nodes;
}

std::vector<std::reference_wrapper<ILayout::Node>> Graph::Nodes()
{
	return m_reference_nodes;
}

void Graph::Layout::Force( Graph& _graph, float _C, int _iterations )
{
	float k = _C * sqrtf( _graph.GetBounds().getArea() / (float)_graph.Nodes().size() );
	float k2 = k * k;

	static float t = .002f;
	
	for (int i = 0; i < _iterations; i++)
	{
		// Repulsion
		for (int j = 0; j < _graph.Nodes().size(); j++)
		{
			auto& v = _graph.m_nodes[j];
			v->SetDisplacement( glm::vec3( 0 ) );
			for (auto& u : _graph.m_nodes)
			{
				if (v == u) continue;

				glm::vec3 delta = v->GetPosition() - u->GetPosition();
				float delta_l = length( delta );
				float fr = k2 / delta_l;
				v->SetDisplacement( v->GetDisplacement() + (delta / delta_l) * fr );
			}
		}

		// Attraction
		for (auto& e : _graph.Edges())
		{
			auto& v = _graph.m_nodes[e.m_source];
			auto& u = _graph.m_nodes[e.m_target];
			glm::vec3 delta = v->GetPosition() - u->GetPosition();
			float delta_l = length( delta );
			glm::vec3 offset = (delta / delta_l) * (delta_l * delta_l / k);
			v->SetDisplacement( v->GetDisplacement() - offset );
			u->SetDisplacement( u->GetDisplacement() + offset );
		}

		/*
			auto& vertex = _graph.Vertices()[j];
			for (auto& e : vertex.m_out_edges)
			{
				auto& u = _graph.m_nodes[e.m_target];
				glm::vec3 delta = v->GetPosition() - u->GetPosition();
				float delta_l = length( delta );
				glm::vec3 offset = (delta / delta_l) * (delta_l * delta_l / k);
				v->SetDisplacement( v->GetDisplacement() - offset );
				u->SetDisplacement( u->GetDisplacement() + offset );
			}*/
		

		for (auto& v : _graph.Nodes())
		{
			auto& node = v.get();
			node.SetPosition( node.GetPosition() + node.GetDisplacement() * t);
		}
		t *= 0.9999f;
	}
	//_graph.UpdateBounds( );
}
}