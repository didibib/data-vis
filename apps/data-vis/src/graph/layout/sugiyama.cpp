#include "precomp.h"

namespace DataVis
{
const int Sugiyama::VisitedIdx = -1;
//--------------------------------------------------------------
bool Sugiyama::Gui( IStructure& _structure )
{
	bool active = false;
	if ( ImGui::TreeNode( "Sugiyama Layout" ) )
	{
		if ( ImGui::Button( "Apply" ) )
		{
			try
			{
				Graph& graph = dynamic_cast<Graph&>( _structure );
				Apply( graph );
				active = true;
			} catch ( std::exception& e )
			{
				std::cout << e.what( ) << std::endl;
			}
		}
		ImGui::TreePop( );
	}
	return active;
}

//--------------------------------------------------------------
void Sugiyama::Apply( Graph& _graph )
{
	Dataset dataset = BreakCycles( _graph );
	_graph.Init(std::make_shared<Dataset>(dataset));
}

//--------------------------------------------------------------
Dataset Sugiyama::BreakCycles( Graph& _graph )
{
	// Make a copy
	auto dataset = _graph.GetDataset( );
	auto& vertices = dataset.GetVertices( );

	// Create a new dataset without any edges
	Dataset new_dataset;
	new_dataset.SetKind( dataset.GetKind( ) );
	new_dataset.GetEdges( ) = dataset.GetEdges( );
	auto& new_vertices = new_dataset.GetVertices( );

	for ( int i = 0; i < vertices.size( ); i++ )
	{
		auto& vertex = vertices[i];
		Vertex v;
		v.id = vertex.id;
		v.idx = vertex.idx;
		v.attributes = vertex.attributes;
		new_vertices.push_back( std::move( v ) );
	}

	// While there are unvisited vertices
	Vertex dummy;
	while ( Has( []( Vertex& ) { return true; }, vertices, dummy ) )
	{
		// Remove sinks
		Vertex sink;
		while ( Has( Sugiyama::IsSink, vertices, sink ) )
		{
			for ( auto& neighbor : sink.incoming_neighbors )
			{
				// Add edge to new dataset
				new_vertices[sink.idx].incoming_neighbors.push_back( neighbor );
				Neighbor rev_neighbor = neighbor;
				rev_neighbor.idx = sink.idx;
				new_vertices[neighbor.idx].neighbors.push_back( rev_neighbor );

				// Remove edge from old dataset
				auto& outgoing = vertices[neighbor.idx].neighbors;
				for ( int i = 0; i < outgoing.size( ); i++ )
				{
					if ( outgoing[i].edge_idx == neighbor.edge_idx )
					{
						outgoing[i] = outgoing[outgoing.size( ) - 1];
						outgoing.resize( outgoing.size( ) - 1 );
					}
				}
			}
			vertices[sink.idx].incoming_neighbors.clear( );
			vertices[sink.idx].idx = VisitedIdx;
		}

		// Delete all isolated nodes
		for ( auto& v : vertices )
			if ( v.incoming_neighbors.size( ) == 0 && 	v.neighbors.size( ) == 0 )
				v.idx = VisitedIdx;

		// Remove sources
		Vertex source;
		while ( Has( Sugiyama::IsSource, vertices, source ) )
		{
			for ( auto& neighbor : source.neighbors )
			{
				// Add edge to new dataset
				new_vertices[source.idx].neighbors.push_back( neighbor );
				Neighbor rev_neighbor = neighbor;
				rev_neighbor.idx = source.idx;
				new_vertices[neighbor.idx].incoming_neighbors.push_back( rev_neighbor );

				// Remove edge from old dataset
				auto& incoming = vertices[neighbor.idx].incoming_neighbors;
				for ( int i = 0; i < incoming.size( ); i++ )
				{
					if ( incoming[i].edge_idx == neighbor.edge_idx )
					{
						incoming[i] = incoming[incoming.size( ) - 1];
						incoming.resize( incoming.size( ) - 1 );
					}
				}
			}
			vertices[source.idx].neighbors.clear( );
			vertices[source.idx].idx = VisitedIdx;
		}

		// Check if vertices is empty
		int max_vertex_idx = VisitedIdx;
		int max_difference = -1e30;
		for ( int i = 0; i < vertices.size( ); i++ )
		{
			if ( vertices[i].idx != VisitedIdx )
			{
				int difference = vertices[i].neighbors.size( ) - vertices[i].incoming_neighbors.size( );
				if ( difference > max_difference )
				{
					max_vertex_idx = i;
					max_difference = difference;
				}
			}
		}

		// Remove max vertex with outgoing edges
		if ( max_difference > -1e30 && max_vertex_idx != VisitedIdx )
		{
			Vertex& max_vertex = vertices[max_vertex_idx];
			// Add outgoing to new dataset and remove from neighbors
			for ( auto& neighbor : max_vertex.neighbors )
			{
				// Add edge to new dataset
				new_vertices[max_vertex.idx].neighbors.push_back( neighbor );
				Neighbor rev_neighbor = neighbor;
				rev_neighbor.idx = max_vertex.idx;
				new_vertices[neighbor.idx].incoming_neighbors.push_back( rev_neighbor );

				// Remove edge from old dataset
				auto& incoming = vertices[neighbor.idx].incoming_neighbors;
				for ( int i = 0; i < incoming.size( ); i++ )
				{
					if ( incoming[i].edge_idx == neighbor.edge_idx )
					{
						incoming[i] = incoming[incoming.size( ) - 1];
						incoming.resize( incoming.size( ) - 1 );
					}
				}
			}

			// Remove incoming from neighbors
			for ( auto& neighbor : max_vertex.incoming_neighbors )
			{
				// !!! Add FLIPPED edge to new dataset
				Neighbor rev_neighbor = neighbor;
				rev_neighbor.idx = max_vertex.idx;
				new_vertices[max_vertex.idx].neighbors.push_back( neighbor );
				new_vertices[neighbor.idx].incoming_neighbors.push_back( rev_neighbor );

				auto& edge = new_dataset.GetEdges()[neighbor.edge_idx];
				auto temp = edge.to_idx;
				edge.to_idx = edge.from_idx;
				edge.from_idx = temp;

				// Remove edge from old dataset
				auto& outgoing = vertices[neighbor.idx].neighbors;
				for ( int i = 0; i < outgoing.size( ); i++ )
				{
					if ( outgoing[i].edge_idx == neighbor.edge_idx )
					{
						outgoing[i] = outgoing[outgoing.size( ) - 1];
						outgoing.resize( outgoing.size( ) - 1 );
					}
				}
			}
			max_vertex.neighbors.clear( );
			max_vertex.incoming_neighbors.clear( );
			max_vertex.idx = VisitedIdx;
		}
	}
	return new_dataset;
}

bool Sugiyama::Has( std::function<bool( Vertex& )> _f, std::vector<Vertex> _vs, Vertex& _out )
{
	for ( auto& v : _vs )
	{
		if ( v.idx != VisitedIdx && _f( v ) )
		{
			_out = v;
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------
void Sugiyama::LayerAssignment( Graph& _graph )
{

}

//--------------------------------------------------------------
void Sugiyama::CrossingMinimization( Graph& _graph )
{

}

//--------------------------------------------------------------
void Sugiyama::VertexPositioning( Graph& _graph )
{

}

bool Sugiyama::IsSink( Vertex& _v )
{
	return _v.neighbors.size( ) == 0;
}
bool Sugiyama::IsSource( Vertex& _v )
{
	return _v.incoming_neighbors.size( ) == 0;
}


} // namespace DataVis