#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// Attributes
//--------------------------------------------------------------
void Attributes::Init( Model::Attributes& _attributes )
{
	for ( auto& it = _attributes.begin( ); it != _attributes.end( ); it++ ) {
		try {
			float value = std::stof( it->second );
			map.insert( { it->first, value } );
		} catch ( std::exception& ) {
			map.insert( { it->first, it->second } );
		}
	}
}

float Attributes::FindFloat(const std::string& _key, float _default )
{
	auto& it = map.find( _key );
	if ( it != map.end( ) ) {
		return std::visit( VisitFloat{ _default }, it->second );
	}
	map[_key] = _default;
	return _default;
}

int Attributes::FindInt(const std::string& _key, int _default )
{
	auto& it = map.find( _key );
	if ( it != map.end( ) ) {
		return std::visit( VisitInt{ _default }, it->second );
	}
	map[_key] = _default;
	return _default;
}

std::string Attributes::FindString(const std::string& _key )
{
	auto& it = map.find( _key );
	if ( it != map.end( ) ) {
		return std::visit( VisitString{}, it->second );
	}
	return "";
}

//--------------------------------------------------------------
// Dataset
//--------------------------------------------------------------
Dataset::Dataset(const Dataset& _dataset)
{
	m_filename = _dataset.m_filename;
	m_info = _dataset.m_info;
	m_info_idx = _dataset.m_info_idx;
	m_kind = _dataset.m_kind;
	m_vertex_idx = _dataset.m_vertex_idx;
	vertices = _dataset.vertices;
	edges = _dataset.edges;
}

Dataset& Dataset::operator=(const Dataset& _dataset)
{
	*this = Dataset(_dataset);
	return *this;
}

bool Dataset::Load(const std::string _filename )
{
	m_filename = _filename;
	std::string filepath = ofToDataPath( _filename, false );
	std::ifstream file( filepath );

	if ( !std::filesystem::exists( filepath ) ) {
		std::cout << "W/Graph::Load: File doesn't exists: " << filepath << std::endl;
		return false;
	}
	using It = boost::spirit::istream_iterator;

	It f{ file >> std::noskipws }, l;

	bool ok = false;
	try {
		Ast::GraphViz into;
		::Parser::GraphViz<It> parser;
		ok = parse( f, l, parser, into );

		if ( ok ) {
			std::cerr << "Parse success\n";
			Model::MainGraph graph;
			graph = buildModel( into );
			m_kind = Kind::Undirected;
			if ( graph.kind == Model::GraphKind::directed ) {
				m_kind = Kind::Directed;
			}
			for ( auto& node : graph.all_nodes ) {
				Vertex v;
				v.id = node.id( );
				v.idx = vertices.size( );
				v.attributes.Init( node.node.attributes );
				m_vertex_idx.insert( std::make_pair(v.id, vertices.size( )) );
				vertices.push_back( std::move( v ) );
			}

			for ( auto& edge : graph.all_edges ) {
				VertexIdx v_from_idx = m_vertex_idx[edge.from.id];
				VertexIdx v_to_idx = m_vertex_idx[edge.to.id];

				Edge e;
				e.attributes.Init( edge.attributes );
				e.from_idx = v_from_idx;
				e.to_idx = v_to_idx;
				e.idx = edges.size( );

				Neighbor n_to(v_to_idx, edges.size( ));
				vertices[v_from_idx].outgoing_neighbors.push_back( n_to );

				Neighbor n_from(v_from_idx, edges.size( ));
				vertices[v_to_idx].incoming_neighbors.push_back(  n_from );
				
				if ( m_kind == Kind::Undirected ) {
					vertices[v_to_idx].outgoing_neighbors.push_back( n_from );
				}
				edges.push_back( std::move( e ) );
			}
		} else {
			std::cerr << "Parse failed\n";
		}
		if ( f != l )
		{
			//std::cerr << "Remaining unparsed input: '" << std::string( f, l ) << "'\n";
		}
	} catch ( ::Parser::qi::expectation_failure<It> const& e ) {
		std::cerr << e.what( ) << ": " << e.what_ << " at " << std::string( e.first, e.last ) << "\n";
	}
	file.close( );

	// Populate information
	SetInfo( );

	return ok;
}

void Dataset::InfoGui( )
{
	if ( ImGui::TreeNode( "Dataset Info" ) ) {
		static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody;

		ImGui::PushStyleVar( ImGuiStyleVar_CellPadding, ImVec2( 3, 5 ) );
		if ( ImGui::BeginTable( "DatasetInfoTable", 2, flags ) ) {
			for ( const auto& [k, v] : m_info ) {
				ImGui::TableNextRow( );
				// Column 
				ImGui::TableNextColumn( );
				ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 123, 123, 123, 255 ) );
				ImGui::Text( k.c_str( ) );
				ImGui::PopStyleColor( );
				// Column
				ImGui::TableNextColumn( ); ImGui::Text( v.c_str( ) );
			}
			ImGui::EndTable( );
		}
		ImGui::PopStyleVar( );
		ImGui::TreePop( );
	}
}

void Dataset::SetInfo( )
{
	m_info.emplace_back( "filename", m_filename  );
	m_info.emplace_back( "# vertices", std::to_string( vertices.size( ) )  );
	m_info.emplace_back( "# edges", std::to_string( edges.size( ) ) );
}

const std::string& Dataset::GetFilename( )
{
	return m_filename;
}

const Dataset::Kind& Dataset::GetKind( )
{
	return m_kind;
}

void Dataset::SetKind( const Kind& _kind)
{
	m_kind = _kind;
}

void Dataset::AddInfo(const std::string& _key, const std::string& _value){
	int &stored_val = m_info_idx[_key];
	if (stored_val) {
		m_info[stored_val].second = _value;
		return;
	}
	stored_val = m_info.size();
	m_info.emplace_back(_key, _value);
}

} // namespace DataVis
