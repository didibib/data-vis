#include "precomp.h"

// Graph parsing
// https://stackoverflow.com/questions/29496182/read-graphviz-in-boostgraph-pass-to-constructor/29501850#29501850
#include "read_graphviz_new.cpp"

namespace DataVis {
	void Graph::Load(std::string _filename) {
		std::string filepath = ofToDataPath(_filename, false);
		std::ifstream file(filepath);

		if (!std::filesystem::exists(filepath)) {
			std::cout << "W/Graph::Setup: File doesn't exists: " << filepath << std::endl;
			return;
		}

		m_graph.clear();
		boost::dynamic_properties dp(boost::ignore_other_properties);
		dp.property("node_id", get(&Node::name, m_graph));
		dp.property("label", get(&Node::label, m_graph));
		dp.property("shape", get(&Node::shape, m_graph));
		dp.property("label", get(&Edge::label, m_graph));
		dp.property("weight", get(&Edge::weight, m_graph));

		boost::read_graphviz(file, m_graph, dp);
		RandomPos();
	}

	void Graph::RandomPos() {
		for (size_t i = 0; i < m_graph.m_vertices.size(); i++)
		{
			auto& position = m_graph.m_vertices[i].m_property.position;
			float x = ofRandomWidth();
			float y = ofRandomHeight();
			float z = -1;
			position = glm::vec3(x, y, z);
		}
	}

	void Graph::Update() {

	}

	void Graph::Draw()
	{
		ofFill();
		
		ofSetColor(123);
		for (const auto& edge : m_graph.m_edges) {
			int startIdx = edge.m_source;
			int endIdx = edge.m_target;
			glm::vec3 start = m_graph.m_vertices[startIdx].m_property.position;
			glm::vec3 end = m_graph.m_vertices[endIdx].m_property.position;
			ofDrawLine(start, end);
		}

		ofSetColor(255);
		for (size_t i = 0; i < m_graph.m_vertices.size(); i++)
		{
			glm::vec3 pos = m_graph.m_vertices[i].m_property.position;
			ofDrawCircle(pos.x, pos.y, radius);
		}
	}

	void Graph::Exit() {

	}
}