#pragma once 

namespace DataVis
{
//--------------------------------------------------------------
using AnyType = std::variant<float, std::string>;

class Attributes
{
public:
	Attributes() = default;
	void Init(Model::Attributes& _attributes)
	{
		for (auto& it = _attributes.begin(); it != _attributes.end(); it++) {
			try {
				float value = std::stof(it->second);
				m_attributes.insert({ it->first, value });
			}
			catch (std::exception& e) {
				m_attributes.insert({ it->first, it->second });
			}
		}
	}

	float Find(std::string _key, float _default)
	{
		auto& it = m_attributes.find(_key);
		if (it != m_attributes.end()) {
			return _default;
			/*return std::visit(DataVis::overload{
				[](const float& f, float) {return f; },
				[](const std::string& s, float _default) { return _default; }
				}, it->second, _default);*/
		}
		m_attributes[_key] = _default;
		return _default;
	}

private:
	std::unordered_map<Model::Id, AnyType> m_attributes;
};

//--------------------------------------------------------------
using VertexIdx = uint;
using EdgeIdx = uint;

struct Neighbor
{
	VertexIdx to_idx = std::numeric_limits<VertexIdx>::max();
	EdgeIdx edge_idx = std::numeric_limits<EdgeIdx>::max();
};

struct Vertex
{
	std::string id;
	/// A neighbor is made out of a vertex index and a edge index
	std::shared_ptr<std::vector<Neighbor>> neighbors;
	Attributes attributes;
};

struct Edge
{
	VertexIdx from_idx = std::numeric_limits<VertexIdx>::max();
	VertexIdx to_idx = std::numeric_limits<VertexIdx>::max();
	Attributes attributes;
};

//--------------------------------------------------------------
class Dataset
{
public:
	Dataset() = default;
	bool Load(std::string filename);
	const std::string& GetFilename();
	std::vector<Edge>& GetEdges();
	std::vector<Vertex>& GetVertices();

private:
	std::string m_filename;
	std::unordered_map<std::string, VertexIdx> m_vertex_idx;
	std::vector<Edge> m_edges;
	std::vector<Vertex> m_vertices;
};
} // namespace DataVis