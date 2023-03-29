#pragma once

namespace DataVis
{
//--------------------------------------------------------------
// Parser
//--------------------------------------------------------------
namespace Parser
{
    bool DotFile( const std::string& filename, Model::MainGraph& );
};

//--------------------------------------------------------------
using AnyType = std::variant<float, std::string>;

struct VisitFloat
{
    VisitFloat(const float _f) : default_value(_f)
    {
    }

    float operator()(const float& _f) const { return CheckParam(_f, default_value); }
    float operator()(const std::string& _s) const { return CheckParam(_s, default_value); }
    static float CheckParam(const float& _f, float) { return _f; }
    static float CheckParam(const std::string&, const float _default) { return _default; }
    float default_value = 0;
};

struct VisitString
{
    std::string operator()(const float& _f) const { return std::to_string(_f); }
    std::string operator()(const std::string& _s) const { return _s; }
};

//--------------------------------------------------------------
// Attributes
//--------------------------------------------------------------
struct Attributes
{
    Attributes() = default;
    void Init(Model::Attributes& _attributes);
    std::unordered_map<Model::Id, AnyType> map;
    float FindFloat(const std::string& key, float default_value);
    int FindInt(const std::string& key, int default_value);
    std::string FindString(const std::string& key);
};

//--------------------------------------------------------------
// Dataset
//-------------------------------------------------------------- 
using VertexIdx = int;
using EdgeIdx = int;

constexpr int VISITED_IDX = -1;
constexpr int UNINIT_IDX = -1;
constexpr const char* DUMMY_ID = "dummy";
constexpr int REMOVE_IDX = -3;

//--------------------------------------------------------------
struct Neighbor
{
    Neighbor() = default;
    Neighbor(VertexIdx _idx, EdgeIdx _edge_idx) :
        idx(_idx), edge_idx(_edge_idx)
    {
    }

    VertexIdx idx = MIN_INT;
    EdgeIdx edge_idx = MIN_INT;
};

//--------------------------------------------------------------
struct Vertex
{
    std::string id;
    std::string owner;
    VertexIdx idx = MIN_INT;
    /// A neighbor is made out of a vertex index and a edge index
    std::vector<Neighbor> outgoing_neighbors;
    std::vector<Neighbor> incoming_neighbors;
    Attributes attributes;
};

//--------------------------------------------------------------
struct Edge
{
    EdgeIdx idx = std::numeric_limits<VertexIdx>::max();
    VertexIdx from_idx = std::numeric_limits<VertexIdx>::min();
    VertexIdx to_idx = std::numeric_limits<VertexIdx>::min();
    Attributes attributes;
};

class Dataset
{
public:
    enum class Kind { Undirected, Directed };

    Dataset() = default;
    virtual ~Dataset() = default;
    Dataset(const Dataset&);
    Dataset& operator=(const Dataset&);
    const std::string& GetId();
    const std::string& GetFilename();
    void Load( const Model::MainGraph&, const std::string& filename );
    // This overload assumes you will update the vertices and edges manually
    void Load( const std::string& id, const std::string& filename, const Kind&);
    void InfoGui();
    [[nodiscard]] const Kind& GetKind() const;
    void AddInfo(const std::string& key, const std::string& value);

    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<Edge> edges;

protected:
    virtual void Convert( const Model::MainGraph& );
    virtual void SetInfo();
    Kind m_kind = Kind::Undirected;
    std::string m_id;
    std::string m_filename;

    std::unordered_map<std::string, VertexIdx> m_vertex_idx;
    std::unordered_map<std::string, int> m_info_idx;
    std::vector<std::pair<std::string, std::string>> m_info;
};

//--------------------------------------------------------------
// DatasetClusters
//--------------------------------------------------------------

/**
 * vertices: contain all the vertices, but their idx corresponds with their idx within a cluster
 * edges: only contain the inter edges; the edges between clusters 
 */
class ClusterDataset final : public Dataset
{
public:
    std::vector<std::shared_ptr<Dataset>> clusters;

protected:
    void Convert( const Model::MainGraph& ) override;
    void SetInfo( ) override;

    std::unordered_map<std::string, uint> m_cluster_idx;

};
} // namespace DataVis
