#include "precomp.h"

namespace DataVis
{
void Graph::Init( const std::shared_ptr<Dataset> _dataset )
{
	IStructure::Init( _dataset );
	Load(_dataset);

	// Add layout
	m_layouts.push_back( std::make_unique<ForceDirected>( ) );
	m_layouts.push_back( std::make_unique<Sugiyama>( ) );
}

void Graph::Load(const std::shared_ptr<Dataset> _dataset)
{
	m_dataset = _dataset;
	m_nodes.clear( );
	m_nodes.reserve( _dataset->vertices.size( ) );
	auto& vertices = _dataset->vertices;
	// Add nodes
	for ( size_t i = 0; i < _dataset->vertices.size( ); i++ )
	{
		auto& vertex = vertices[i];
		m_nodes.push_back( std::make_shared<Node>( vertex.id, i ) );
	}
	// Add neighbors
	for ( size_t i = 0; i < m_nodes.size( ); i++ )
	{
		auto& vertex = vertices[i];
		if ( vertex.neighbors.empty( ) ) continue;
		for ( const auto& n : vertex.neighbors )
		{
			m_nodes[i]->neighbors.push_back( m_nodes[n.idx] );
		}
	}
}

//--------------------------------------------------------------
void Graph::DrawNodes( )
{
	ofFill( );
	ofSetColor( 123 );
	for ( const auto& edge : m_dataset->edges )
	{
		auto const& startIdx = edge.from_idx;
		auto const& endIdx = edge.to_idx;
		glm::vec3 start = m_nodes[startIdx]->GetPosition( );
		glm::vec3 end = m_nodes[endIdx]->GetPosition( );
		glm::vec3 dir = normalize(end - start);
		start += dir * m_nodes[startIdx]->GetRadius();
		end -= dir * m_nodes[endIdx]->GetRadius();

		// Draw edge behind nodes
		start--;
		end--;
		if ( m_dataset->GetKind( ) == Dataset::Kind::Undirected )
			ofDrawLine( start, end );
		else if ( m_dataset->GetKind( ) == Dataset::Kind::Directed )
		{
			end -= dir * m_nodes[endIdx]->GetRadius();
			ofDrawArrow( start, end, m_nodes[endIdx]->GetRadius( ) );
		}
	}

	for ( const auto& node : m_nodes )
	{
		if (node->GetVertexId() == "dummy") continue;
		glm::vec3 pos = node->GetPosition( );
		ofSetColor( node->color );
		ofDrawCircle( pos, node->GetRadius() );
	}
}

} // namespace DataVis