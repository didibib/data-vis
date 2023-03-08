#include "precomp.h"

namespace DataVis {
bool Graph::Extract::Load(Graph& _graph, std::string _filename) {
	std::string filepath = ofToDataPath(_filename, false);
	std::ifstream file(filepath);

	if (!std::filesystem::exists(filepath)) {
		std::cout << "W/Graph::Load: File doesn't exists: " << filepath << std::endl;
		return false;
	}

	using It = boost::spirit::istream_iterator;
	Parser::GraphViz<It> parser;
		
	It f{ file >> std::noskipws }, l;

	Ast::GraphViz into;
	bool ok = false;
	try {
		ok = parse(f, l, parser, into);

		if (ok) {
			std::cerr << "Parse success\n";
			_graph.m_graph = buildModel(into);

			for (auto& it = _graph.m_graph.all_nodes.begin(); it != _graph.m_graph.all_nodes.end(); it++) {
				it->
			}
			_graph.PostBuild();
		}
		else {
			std::cerr << "Parse failed\n";
		}
		if (f != l)
			std::cerr << "Remaining unparsed input: '" << std::string(f, l) << "'\n";
	}
	catch (Parser::qi::expectation_failure<It> const& e) {
		std::cerr << e.what() << ": " << e.what_ << " at " << std::string(e.first, e.last) << "\n";
	}

	return ok;
}

void Graph::PostBuild() {
	m_nodes.clear();
	m_nodes.reserve(m_graph.all_nodes.size());
	for (auto& it = m_graph.all_nodes.begin(); it != m_graph.all_nodes.end(); it++)
	{
		m_nodes.push_back(std::make_unique<ILayout::Node>(it->id()));
	}
	CreateReferenceNodes();
}

void Graph::HandleInput()
{
}

void Graph::Select(const glm::vec3& _position)
{

}

void Graph::Update(float delta_time)
{
}

void Graph::DrawLayout()
{
	ofFill();
	ofSetColor(123);
	for (const auto& edge : m_graph.all_edges) {
		auto const& startIdx = edge.from.id;
		auto const& endIdx = edge.to;
		//glm::vec3 start = m_nodes[startIdx]->GetPosition();
		//glm::vec3 end = m_nodes[endIdx]->GetPosition();
		//// Draw edge behind nodes
		//start -= 1;
		//end -= 1;
		//ofDrawLine(start, end);
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
	m_reference_nodes.clear();
	for (auto& n : m_nodes) m_reference_nodes.push_back(std::ref(*n));
}

std::vector<std::reference_wrapper<ILayout::Node>> Graph::Nodes()
{
	return m_reference_nodes;
}
}