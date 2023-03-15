#pragma once 

namespace DataVis
{
//--------------------------------------------------------------
using AnyType = std::variant<float, std::string>;

struct VisitFloat
{
	VisitFloat( float _f ) : default_value( _f ) {}
	float operator()( float& _f ) { return CheckParam( _f, default_value ); }
	float operator()( std::string& _s ) { return CheckParam( _s, default_value ); }
	float CheckParam( const float& _f, float _default ) { return _f; }
	float CheckParam( const std::string&, float _default ) { return _default; }
	float default_value = 0;
};

struct VisitString
{
	std::string operator()( float& _f ) { return std::to_string( _f ); }
	std::string operator()( std::string& _s ) { return _s; }
};

//--------------------------------------------------------------
// Attributes
//--------------------------------------------------------------
class Attributes
{
public:
	Attributes( ) = default;
	void Init( Model::Attributes& _attributes );
	const std::unordered_map<Model::Id, AnyType>& Get( ) { return m_attributes; }

	/// <summary>
	/// Find the float value for the given key. 
	/// If the key is not present it will be inserted with the default value
	/// </summary>
	/// <param name="_key"></param>
	/// <param name="_default">: The default value if the key is not present.</param>
	/// <returns></returns>
	float FindFloat( std::string _key, float _default );

	/// <summary>
	/// Find any value for the given key. Any value will be returned as string.
	/// If the key is not present it will return a empty string;
	/// </summary>
	/// <param name="_key"></param>
	/// <returns>Key value or empty string</returns>
	std::string FindString( std::string _key );

private:
	std::unordered_map<Model::Id, AnyType> m_attributes;
};

//--------------------------------------------------------------
// Dataset
//-------------------------------------------------------------- 
using VertexIdx = int;
using EdgeIdx = VertexIdx;

struct Neighbor
{
	VertexIdx idx = std::numeric_limits<VertexIdx>::min( );
	EdgeIdx edge_idx = std::numeric_limits<EdgeIdx>::min( );
};

struct Vertex
{
	std::string id;
	VertexIdx idx = std::numeric_limits<VertexIdx>::min( );
	/// A neighbor is made out of a vertex index and a edge index
	std::vector<Neighbor> neighbors;
	std::vector<Neighbor> incoming_neighbors;
	Attributes attributes;
};

struct Edge
{
	EdgeIdx idx = std::numeric_limits<VertexIdx>::max( );
	VertexIdx from_idx = std::numeric_limits<VertexIdx>::min( );
	VertexIdx to_idx = std::numeric_limits<VertexIdx>::min( );
	Attributes attributes;
};


//--------------------------------------------------------------
class Dataset
{
public:
	enum class Kind { Undirected, Directed };
	Dataset( ) = default;
	bool Load( std::string filename );
	void InfoGui( );
	const std::string& GetFilename( );
	const Kind& GetKind( );
	void SetKind( const Kind& );
	std::vector<Vertex> vertices;
	std::vector<Edge> edges;
	void AddInfo(const std::string& key, const std::string& value);
	std::shared_ptr<Dataset> DeepCopy();

private:
	void SetInfo( );
	Kind m_kind;
	std::string m_filename;
	std::unordered_map<std::string, VertexIdx> m_vertex_idx;

	std::unordered_map<std::string, int> m_info_idx;
	std::vector<std::pair<std::string, std::string>> m_info;
};
} // namespace DataVis