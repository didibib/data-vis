#include "precomp.h"

// Graph parsing
// https://stackoverflow.com/questions/29496182/read-graphviz-in-boostgraph-pass-to-constructor/29501850#29501850
#include "read_graphviz_new.cpp"

namespace DataVis {
	void Graph::Load(std::string _filename) {
		std::string filepath = ofToDataPath(_filename, false);
		std::ifstream file(filepath);

		if (!std::filesystem::exists(filepath)) {
			std::cout << "W/Graph::Load: File doesn't exists: " << filepath << std::endl;
			return;
		}

		m_graph.clear();
		boost::dynamic_properties dp(boost::ignore_other_properties);
		dp.property("node_id", get(&Node::name, m_graph));
		dp.property("shape", get(&Node::shape, m_graph));
		dp.property("weight", get(&Edge::weight, m_graph));

		boost::read_graphviz(file, m_graph, dp);
		printf( "-- Finished loading %s \n", _filename.c_str());
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
			// draw edge behind nodes
			start -= 1;
			end -= 1;
			ofSetLineWidth( edge.m_property.weight );
			ofDrawLine(start, end);
		}

		ofSetColor(255);
		ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
		for (size_t i = 0; i < m_graph.m_vertices.size(); i++)
		{
			auto& property = m_graph.m_vertices[i].m_property;
			glm::vec3 pos = property.position;
			ofDrawCircle(pos, radius);
			//std::string text = property.name;
			//ofDrawBitmapStringHighlight(text, pos);
		}
	}
}