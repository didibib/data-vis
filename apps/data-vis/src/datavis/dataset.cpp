#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// Attributes
//--------------------------------------------------------------
void Attributes::Init(Model::Attributes& _attributes)
{
    for (auto& it = _attributes.begin(); it != _attributes.end(); it++)
    {
        try
        {
            float value = std::stof(it->second);
            map.insert({it->first, value});
        }
        catch (std::exception&)
        {
            map.insert({it->first, it->second});
        }
    }
}

float Attributes::FindFloat(const std::string& _key, float _default)
{
    auto& it = map.find(_key);
    if (it != map.end())
    {
        return std::visit(VisitFloat{_default}, it->second);
    }
    map[_key] = _default;
    return _default;
}

std::string Attributes::FindString(const std::string& _key)
{
    auto& it = map.find(_key);
    if (it != map.end())
    {
        return std::visit(VisitString{}, it->second);
    }
    return "";
}

//--------------------------------------------------------------
// Parser
//--------------------------------------------------------------
namespace Parser
{
    bool DotFile(const std::string& _filename, Model::MainGraph& _graph)
    {
        std::string filepath = ofToDataPath(_filename, false);
        std::ifstream file(filepath);

        if (!std::filesystem::exists(filepath))
        {
            std::cout << "W/Graph::Load: File doesn't exists: " << filepath << std::endl;
            return false;
        }
        using It = boost::spirit::istream_iterator;
        It f{file >> std::noskipws}, l;

        bool ok = false;
        try
        {
            Ast::GraphViz into;
            ::Parser::GraphViz<It> parser;
            ok = parse(f, l, parser, into);

            if (ok)
            {
                std::cerr << "Parse success\n";
                _graph = buildModel(into);
            }
            else
            {
                std::cerr << "Parse failed\n";
            }
            if (f != l)
            {
                //std::cerr << "Remaining unparsed input: '" << std::string( f, l ) << "'\n";
            }
        }
        catch (::Parser::qi::expectation_failure<It> const& e)
        {
            std::cerr << e.what() << ": " << e.what_ << " at " << std::string(e.first, e.last) << "\n";
        }
        file.close();
        return ok;
    }
} // namespace Parser
//--------------------------------------------------------------
// Dataset
//--------------------------------------------------------------
Dataset::Dataset(const Dataset& _dataset)
{
    *this = _dataset;
}

Dataset& Dataset::operator=(const Dataset& _dataset)
{
    m_id = _dataset.m_id;
    m_filename = _dataset.m_filename;
    m_info = _dataset.m_info;
    m_info_idx = _dataset.m_info_idx;
    m_kind = _dataset.m_kind;
    m_vertex_idx = _dataset.m_vertex_idx;
    uint size = _dataset.vertices.size();
    vertices.resize(size);
    for(int i = 0; i <size; i++)
    {
        vertices[i] = std::make_shared<Vertex>(*_dataset.vertices[i]);        
    }
    edges = _dataset.edges;
    return *this;
}

const std::string& Dataset::GetId()
{
    return m_id;
}

const std::string& Dataset::GetFilename()
{
    return m_filename;
}

void Dataset::Load(const Model::MainGraph& _graph, const std::string& _filename)
{
    m_filename = _filename;
    m_kind = Kind::Undirected;
    if (_graph.kind == Model::GraphKind::directed)
        m_kind = Kind::Directed;
    Convert(_graph);
    SetInfo();
}

void Dataset::Load(const std::string& id, const std::string& filename, const Kind& _kind)
{
    m_id = id;
    m_filename = filename;
    m_kind = _kind;
}

void Dataset::Convert(const Model::MainGraph& _graph)
{
    for (auto& node : _graph.all_nodes)
    {
        auto& v = *vertices.emplace_back(std::make_shared<Vertex>());
        v.id = node.id();
        v.idx = vertices.size() - 1;
        v.attributes.Init(node.node.attributes);
        m_vertex_idx.emplace(v.id, vertices.size() - 1);
    }

    for (auto& edge : _graph.all_edges)
    {
        const VertexIdx v_from_idx = m_vertex_idx[edge.from.id];
        const VertexIdx v_to_idx = m_vertex_idx[edge.to.id];

        auto& e = edges.emplace_back();
        e.attributes.Init(edge.attributes);
        e.from_idx = v_from_idx;
        e.to_idx = v_to_idx;
        const int edge_idx = edges.size() - 1;
        e.idx = edge_idx;

        vertices[v_from_idx]->outgoing_neighbors.emplace_back(v_to_idx, edge_idx);
        vertices[v_to_idx]->incoming_neighbors.emplace_back(v_from_idx, edge_idx);

        if (m_kind == Kind::Undirected)
        {
            vertices[v_to_idx]->outgoing_neighbors.emplace_back(v_from_idx, edge_idx);
            vertices[v_from_idx]->incoming_neighbors.emplace_back(v_to_idx, edge_idx);            
        }
    }
}

void Dataset::InfoGui()
{
    if (ImGui::TreeNode("Dataset Info"))
    {
        static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_ContextMenuInBody;

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(3, 5));
        if (ImGui::BeginTable("DatasetInfoTable", 2, flags))
        {
            for (const auto& [k, v] : m_info)
            {
                ImGui::TableNextRow();
                // Column 
                ImGui::TableNextColumn();
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(123, 123, 123, 255));
                ImGui::Text(k.c_str());
                ImGui::PopStyleColor();
                // Column
                ImGui::TableNextColumn();
                ImGui::Text(v.c_str());
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
        ImGui::TreePop();
    }
}

void Dataset::SetInfo()
{
    m_info.emplace_back("id", m_id);
    m_info.emplace_back("# vertices", std::to_string(vertices.size()));
    m_info.emplace_back("# edges", std::to_string(edges.size()));
}

const Dataset::Kind& Dataset::GetKind() const
{
    return m_kind;
}

void Dataset::AddInfo(const std::string& _key, const std::string& _value)
{
    int& stored_val = m_info_idx[_key];
    if (stored_val)
    {
        m_info[stored_val].second = _value;
        return;
    }
    stored_val = m_info.size();
    m_info.emplace_back(_key, _value);
}

//--------------------------------------------------------------
// SubDataset
//--------------------------------------------------------------
void DatasetClusters::Convert(const Model::MainGraph& _graph)
{
    // Create clusters
    for (const auto& [id, subgraph] : _graph.graph.subgraphs)
    {
        clusters.emplace_back(std::make_shared<Dataset>());
        m_cluster_idx.emplace(id, clusters.size() - 1);
        clusters.back()->Load(id, m_filename, m_kind);
    }

    // Create vertices
    for (auto& node : _graph.all_nodes)
    {
        auto& cluster = *clusters[m_cluster_idx[node.owner->id]];
        // Create vertex
        cluster.vertices.emplace_back(std::make_shared<Vertex>());
        auto& v = cluster.vertices.back();
        v->id = node.id();
        v->owner = node.owner->id;
        v->idx = cluster.vertices.size() - 1;
        v->attributes.Init(node.node.attributes);
        // Add vertex to global vertices
        vertices.emplace_back(v);
        m_vertex_idx.emplace(v->id, vertices.size() - 1);
    }

    // Create edges
    for (auto& edge : _graph.all_edges)
    {
        const VertexIdx v_from_idx = m_vertex_idx[edge.from.id];
        const VertexIdx v_to_idx = m_vertex_idx[edge.to.id];
        const auto& vertex_from = *vertices[v_from_idx];
        const auto& vertex_to = *vertices[v_to_idx];
        
        const auto& cluster_from_id = vertex_from.owner;
        const auto& cluster_to_id = vertex_to.owner;

        if(cluster_from_id == cluster_to_id)
        {
            auto& cluster = *clusters[m_cluster_idx[cluster_from_id]];
            cluster.edges.emplace_back();
           
            auto& e =  cluster.edges.emplace_back();
            e.attributes.Init(edge.attributes);
            e.from_idx = vertex_from.idx;
            e.to_idx = vertex_to.idx;
            const int edge_idx = cluster.edges.size() - 1;
            e.idx = edge_idx;

            vertices[v_from_idx]->outgoing_neighbors.emplace_back(v_to_idx, edge_idx);
            vertices[v_to_idx]->incoming_neighbors.emplace_back(v_from_idx, edge_idx);

            if (m_kind == Kind::Undirected)
            {
                vertices[v_to_idx]->outgoing_neighbors.emplace_back(v_from_idx, edge_idx);
                vertices[v_from_idx]->incoming_neighbors.emplace_back(v_to_idx, edge_idx);            
            }
        }
        else
        {
            auto& e = edges.emplace_back();
            e.attributes.Init(edge.attributes);
            e.from_idx = v_from_idx;
            e.to_idx = v_to_idx;
            const int edge_idx = edges.size() - 1;
            e.idx = edge_idx;
        }
    }
}

void DatasetClusters::SetInfo()
{
}
} // namespace DataVis
