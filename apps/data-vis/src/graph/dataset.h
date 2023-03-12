#pragma once 

namespace DataVis
{
//--------------------------------------------------------------
using AnyType = std::variant<float, std::string>;

struct VisitFloat
{
	VisitFloat(float _f) : default_value(_f) {}
	float operator()(float& _f) { return CheckParam(_f, default_value); }
	float operator()(std::string& _s) { return CheckParam(_s, default_value); }
	float CheckParam(const float& _f, float _default) { return _f; }
	float CheckParam(const std::string&, float _default) { return _default; }
	float default_value = 0;
};

struct VisitString
{
	std::string operator()(float& _f) { return std::to_string(_f); }
	std::string operator()(std::string& _s) { return _s; }
};

//--------------------------------------------------------------
// Attributes
//--------------------------------------------------------------
class Attributes
{
public:
	Attributes() = default;
	void Init(Model::Attributes& _attributes);
	const std::unordered_map<Model::Id, AnyType>& Get() { return m_attributes; }

	/// <summary>
	/// Find the float value for the given key. 
	/// If the key is not present it will be inserted with the default value
	/// </summary>
	/// <param name="_key"></param>
	/// <param name="_default">: The default value if the key is not present.</param>
	/// <returns></returns>
	float FindFloat(std::string _key, float _default);

	/// <summary>
	/// Find any value for the given key. Any value will be returned as string.
	/// If the key is not present it will return a empty string;
	/// </summary>
	/// <param name="_key"></param>
	/// <returns>Key value or empty string</returns>
	std::string FindString(std::string _key);

private:
	std::unordered_map<Model::Id, AnyType> m_attributes;
};

//--------------------------------------------------------------
// Dataset
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
	void InfoGui();
	const std::string& GetFilename();
	std::vector<Edge>& GetEdges();
	std::vector<Vertex>& GetVertices();

private:
	void SetInfo();

	std::string m_filename;
	std::unordered_map<std::string, VertexIdx> m_vertex_idx;
	std::vector<Vertex> m_vertices;
	std::vector<Edge> m_edges;
	std::vector<std::pair<std::string, std::string>> m_info;
};
} // namespace DataVis