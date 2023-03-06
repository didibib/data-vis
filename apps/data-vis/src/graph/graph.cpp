#include "precomp.h"

// Graph parsing
// https://stackoverflow.com/questions/29496182/read-graphviz-in-boostgraph-pass-to-constructor/29501850#29501850
#include "read_graphviz_new.cpp"

namespace DataVis {
void Graph::Extract::Load(Graph& _graph, std::string _filename) {
	std::string filepath = ofToDataPath(_filename, false);
	std::ifstream file(filepath);

	if (!std::filesystem::exists(filepath)) {
		std::cout << "W/Graph::Load: File doesn't exists: " << filepath << std::endl;
		return;
	}

	_graph.m_graph.clear();
	boost::dynamic_properties dp(boost::ignore_other_properties);
	dp.property("node_id", get(&Vertex::name, _graph.m_graph));
	dp.property("weight", get(&Edge::weight, _graph.m_graph));

	boost::read_graphviz(file, _graph.m_graph, dp);
	printf("-- Finished loading %s \n", _filename.c_str());
	for (size_t i = 0; i < _graph.m_graph.m_vertices.size(); i++)
		_graph.m_nodes.push_back(std::make_unique<ILayout::Node>(i));
	_graph.PostBuild();
}

void Graph::PostBuild() {
	CreateReferenceNodes();
}

void Graph::HandleInput()
{
}

void Graph::Select( const glm::vec3& _position )
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
	for (const auto& edge : m_graph.m_edges) {
		int startIdx = edge.m_source;
		int endIdx = edge.m_target;
		glm::vec3 start = m_nodes[startIdx]->GetPosition();
		glm::vec3 end = m_nodes[endIdx]->GetPosition();
		// Draw edge behind nodes
		start -= 1;
		end -= 1;
		ofSetLineWidth(edge.m_property.weight);
		ofDrawLine(start, end);
	}

	ofSetColor(10);
	ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
	for (size_t i = 0; i < m_nodes.size(); i++)
	{
		glm::vec3 pos = m_nodes[i]->GetPosition();
		ofDrawCircle(pos, radius);
	}
}

void Graph::Gui()
{
}

void Graph::CreateReferenceNodes() {
	// https://jonasdevlieghere.com/containers-of-unique-pointers/
	m_reference_nodes.clear();
	for (auto& n : m_nodes) m_reference_nodes.push_back(std::ref(*n));
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