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

void Graph::Select(const ofCamera&, const glm::vec3& _position) {
	
}

void Graph::Update(float delta_time)
{
}

void Graph::Draw()
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

	ofSetColor(255);
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
	static std::function<std::vector<std::reference_wrapper<ILayout::Node>>(std::vector<std::shared_ptr<ILayout::Node>>)> Wrap
		= [&](std::vector<shared_ptr<ILayout::Node>> _nodes) {
		std::vector<std::reference_wrapper<ILayout::Node>> nodes;
		for (auto& n : _nodes) nodes.push_back(std::ref(*n));
		return nodes;
	};
	m_reference_nodes = Wrap(m_nodes);
}

std::vector<std::reference_wrapper<ILayout::Node>> Graph::Nodes()
{
	return m_reference_nodes;
}
}