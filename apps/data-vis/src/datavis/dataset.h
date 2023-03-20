#pragma once

namespace DataVis
{
//--------------------------------------------------------------
using AnyType = std::variant<int, float, std::string>;

struct VisitFloat
{
    VisitFloat(float _f) : default_value(_f)
    {
    }

    float operator()(const float& _f) const { return CheckParam(_f, default_value); }
    float operator()(const std::string& _s) const { return CheckParam(_s, default_value); }
    float operator()(const int& _i) const { return CheckParam(_i, default_value); }
    static float CheckParam(const float& _f, float _default) { return _f; }
    static float CheckParam(const std::string&, float _default) { return _default; }
    static float CheckParam(const int& _i, float) { return static_cast<float>(_i); }
    float default_value = 0;
};

struct VisitInt
{
    VisitInt(int _f) : default_value(_f)
    {
    }

    float operator()(const float& _f) const { return CheckParam(_f, default_value); }
    float operator()(const std::string& _s) const { return CheckParam(_s, default_value); }
    float operator()(const int& _i) const { return CheckParam(_i, default_value); }
    static float CheckParam(const float& _f, int _default) { return _default; }
    static float CheckParam(const std::string&, int _default) { return _default; }
    static float CheckParam(const int& _i, int) { return _i; }
    float default_value = 0;
};

struct VisitString
{
    std::string operator()(const float& _f) const { return std::to_string(_f); }
    std::string operator()(const std::string& _s) const { return _s; }
    std::string operator()(const int& _i) const { return std::to_string(_i); }
};

//--------------------------------------------------------------
// Attributes
//--------------------------------------------------------------
struct Attributes
{
    Attributes() = default;
    void Init(Model::Attributes& _attributes);
    std::unordered_map<Model::Id, AnyType> map;
    float FindFloat(const std::string& key, float default);
    int FindInt(const std::string& key, int default);
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

//--------------------------------------------------------------
namespace Parser
{
    bool DotFile( const std::string& filename, Model::MainGraph& );
};

class Dataset
{
public:
    enum class Kind { Undirected, Directed };

    Dataset() = default;
    ~Dataset() = default;
    Dataset(const Dataset&);
    Dataset& operator=(const Dataset&);
    void Load( const Model::MainGraph& );
    void InfoGui();
    std::string filename;
    const Kind& GetKind();
    void SetKind(const Kind&);
    void AddInfo(const std::string& key, const std::string& value);

    std::vector<Vertex> vertices;
    std::vector<Edge> edges;

protected:
    virtual void Convert( const Model::MainGraph& );
    virtual void SetInfo();
    Kind m_kind = Kind::Undirected;
    std::string m_id;

    std::unordered_map<std::string, VertexIdx> m_vertex_idx;
    std::unordered_map<std::string, int> m_info_idx;
    std::vector<std::pair<std::string, std::string>> m_info;
};

class DatasetClusters final : public Dataset
{
public:
    std::vector<Dataset> clusters;

protected:
    virtual void Convert( const Model::MainGraph& ) override;
    virtual void SetInfo( ) override;

    std::unordered_map<std::string, uint> m_dataset_idx;

};
} // namespace DataVis
