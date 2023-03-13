#include "precomp.h"

namespace DataVis
{
const int Sugiyama::VisitedIdx = -1;
const int Sugiyama::DummyIdx = -2;
const int Sugiyama::RemoveIdx = -3;
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
	_graph.Load( std::make_shared<Dataset>( dataset ) );

	std::vector<std::vector<int>> vertices_per_layer;
	std::vector<int> layer_per_vertex;
	LayerAssignment( _graph, vertices_per_layer, layer_per_vertex );
	for ( int y = 0; y < vertices_per_layer.size( ); y++ )
	{
		std::vector<int> vertices = vertices_per_layer[y];
		for ( int x = 0; x < vertices.size( ); x++ )
		{
			int idx = vertices[x];
			auto& node = _graph.GetNodes( )[idx];
			node->SetNewPosition( glm::vec3( x * 100, y * -100, 0 ) );
		}
	}

	_graph.UpdateAABB( );
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
			if ( v.incoming_neighbors.size( ) == 0 && v.neighbors.size( ) == 0 )
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

				auto& edge = new_dataset.GetEdges( )[neighbor.edge_idx];
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
void Sugiyama::LayerAssignment( Graph& _graph, std::vector<std::vector<int>>& _vertices_per_layer, std::vector<int>& _layer_per_vertex )
{
	int layer = 0;
	Dataset copy_dataset = _graph.GetDataset( );
	int size = _graph.GetDataset( ).GetVertices( ).size( );
	_layer_per_vertex.resize( size );

	bool hasSources = false;
	do
	{
		hasSources = false;
		_vertices_per_layer.push_back( {} );
		for ( auto& v : copy_dataset.GetVertices( ) )
		{
			if ( v.idx == VisitedIdx ) continue;

			if ( IsSource( v ) )
			{
				hasSources = true;
				// Assign layer to sink
				_layer_per_vertex[v.idx] = layer;
				_vertices_per_layer[layer].push_back( v.idx );
				v.idx = RemoveIdx;
			}
		}
		for ( auto& v : copy_dataset.GetVertices( ) )
		{
			if ( v.idx == RemoveIdx ) 
			{
				// Remove sink from copy_dataset (including incoming edges)
				RemoveOutgoingNeighbors( copy_dataset, v );
				v.idx = VisitedIdx;
			}
		}
		layer++;
	} while ( hasSources );
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

void Sugiyama::RemoveOutgoingNeighbors( Dataset& _dataset, Vertex& _v )
{
	// Add outgoing to new dataset and remove from neighbors
	for ( auto& neighbor : _v.neighbors )
	{
		// Remove edge from dataset
		auto& incoming = _dataset.GetVertices( )[neighbor.idx].incoming_neighbors;
		for ( int i = 0; i < incoming.size( ); i++ )
		{
			if ( incoming[i].edge_idx == neighbor.edge_idx )
			{
				incoming[i] = incoming[incoming.size( ) - 1];
				incoming.resize( incoming.size( ) - 1 );
			}
		}
	}
	_v.neighbors.clear( );
}
void Sugiyama::RemoveIncomingNeighbors( Dataset& _dataset, Vertex& _v )
{
	// Remove incoming from neighbors
	for ( auto& neighbor : _v.incoming_neighbors )
	{
		// Remove edge from old dataset
		auto& outgoing = _dataset.GetVertices( )[neighbor.idx].neighbors;
		for ( int i = 0; i < outgoing.size( ); i++ )
		{
			if ( outgoing[i].edge_idx == neighbor.edge_idx )
			{
				outgoing[i] = outgoing[outgoing.size( ) - 1];
				outgoing.resize( outgoing.size( ) - 1 );
			}
		}
	}
	_v.incoming_neighbors.clear( );
}

} // namespace DataVis