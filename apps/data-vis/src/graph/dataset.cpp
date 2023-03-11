#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// Attributes
//--------------------------------------------------------------
void Attributes::Init(Model::Attributes& _attributes)
{
	for (auto& it = _attributes.begin(); it != _attributes.end(); it++) {
		try {
			float value = std::stof(it->second);
			m_attributes.insert({ it->first, value });
		}
		catch (std::exception&) {
			m_attributes.insert({ it->first, it->second });
		}
	}
}

float Attributes::FindFloat(std::string _key, float _default)
{
	auto& it = m_attributes.find(_key);
	if (it != m_attributes.end()) {
		return std::visit(VisitFloat{ _default }, it->second);
	}
	m_attributes[_key] = _default;
	return _default;
}

std::string Attributes::FindString(std::string _key)
{
	auto& it = m_attributes.find(_key);
	if (it != m_attributes.end()) {
		return std::visit(VisitString{}, it->second);
	}
	return "";
}

//--------------------------------------------------------------
// Dataset
//--------------------------------------------------------------
bool Dataset::Load(std::string _filename)
{
	m_filename = _filename;
	std::string filepath = ofToDataPath(_filename, false);
	std::ifstream file(filepath);

	if (!std::filesystem::exists(filepath)) {
		std::cout << "W/Graph::Load: File doesn't exists: " << filepath << std::endl;
		return false;
	}
	using It = boost::spirit::istream_iterator;
	::Parser::GraphViz<It> parser;

	It f{ file >> std::noskipws }, l;

	Ast::GraphViz into;
	bool ok = false;
	try {
		ok = parse(f, l, parser, into);

		if (ok) {
			std::cerr << "Parse success\n";
			Model::MainGraph graph;
			graph = buildModel(into);

			for (auto& node : graph.all_nodes) {
				Vertex v;
				v.id = node.id();
				v.attributes.Init(node.node.attributes); 
				v.neighbors = std::make_shared<std::vector<Neighbor>>();
				m_vertex_idx.insert({ v.id, m_vertices.size() });
				m_vertices.push_back(std::move(v));
			}

			for (auto& edge : graph.all_edges) {
				VertexIdx v_from_idx = m_vertex_idx[edge.from.id];
				VertexIdx v_to_idx = m_vertex_idx[edge.to.id];
				
				Edge e;
				e.attributes.Init(edge.attributes);
				e.from_idx = v_from_idx;
				e.to_idx = v_to_idx;

				Neighbor n_to;
				n_to.to_idx = v_to_idx;
				n_to.edge_idx = m_edges.size();

				m_vertices[v_from_idx].neighbors->push_back(std::move(n_to));
				if(graph.kind == Model::GraphKind::undirected){
					Neighbor n_from;
					n_from.to_idx = v_from_idx;
					n_from.edge_idx = m_edges.size();
					m_vertices[v_to_idx].neighbors->push_back(std::move(n_from));
				}
				m_edges.push_back(std::move(e));
			}
		}
		else {
			std::cerr << "Parse failed\n";
		}
		if (f != l)
			std::cerr << "Remaining unparsed input: '" << std::string(f, l) << "'\n";
	}
	catch (::Parser::qi::expectation_failure<It> const& e) {
		std::cerr << e.what() << ": " << e.what_ << " at " << std::string(e.first, e.last) << "\n";
	}
	file.close();
	return ok;
}

const std::string& Dataset::GetFilename()
{
	return m_filename;
}

std::vector<Edge>& Dataset::GetEdges()
{
	return m_edges;
}

std::vector<Vertex>& Dataset::GetVertices()
{
	return m_vertices;
}

} // namespace DataVis
