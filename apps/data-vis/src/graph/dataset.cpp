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
			m_attributes.insert( { it->first, value } );
		} catch ( std::exception& ) {
			m_attributes.insert( { it->first, it->second } );
		}
	}
}

float Attributes::FindFloat( std::string _key, float _default )
{
	auto& it = m_attributes.find( _key );
	if ( it != m_attributes.end( ) ) {
		return std::visit( VisitFloat{ _default }, it->second );
	}
	m_attributes[_key] = _default;
	return _default;
}

std::string Attributes::FindString( std::string _key )
{
	auto& it = m_attributes.find( _key );
	if ( it != m_attributes.end( ) ) {
		return std::visit( VisitString{}, it->second );
	}
	return "";
}

//--------------------------------------------------------------
// Dataset
//--------------------------------------------------------------
bool Dataset::Load( std::string _filename )
{
	m_filename = _filename;
	std::string filepath = ofToDataPath( _filename, false );
	std::ifstream file( filepath );

	if ( !std::filesystem::exists( filepath ) ) {
		std::cout << "W/Graph::Load: File doesn't exists: " << filepath << std::endl;
		return false;
	}
	using It = boost::spirit::istream_iterator;
	::Parser::GraphViz<It> parser;

	It f{ file >> std::noskipws }, l;

	Ast::GraphViz into;
	bool ok = false;
	try {
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
				m_vertex_idx.insert( { v.id, vertices.size( ) } );
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

				Neighbor n_to;
				n_to.idx = v_to_idx;
				n_to.edge_idx = edges.size( );

				vertices[v_from_idx].neighbors.push_back( std::move( n_to ) );

				Neighbor n_from;
				n_from.idx = v_from_idx;
				n_from.edge_idx = edges.size( );

				if ( m_kind == Kind::Undirected ) {
					vertices[v_to_idx].neighbors.push_back( std::move( n_from ) );
				}
				else if ( m_kind == Kind::Directed ) {
					vertices[v_to_idx].incoming_neighbors.push_back( std::move( n_from ) );
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
	m_info.push_back( { "filename", m_filename } );
	m_info.push_back( { "# vertices", std::to_string( vertices.size( ) ) } );
	m_info.push_back( { "# edges", std::to_string( edges.size( ) ) } );

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
	m_info.push_back({_key, _value});
}

} // namespace DataVis
