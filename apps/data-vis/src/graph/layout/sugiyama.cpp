#include "precomp.h"

namespace DataVis
{
const int Sugiyama::VisitedIdx = -1;
const std::string Sugiyama::DummyId = "dummy";
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
	Dataset dataset = BreakCycles( _graph.GetDataset() );
	//_graph.Load( std::make_shared<Dataset>( dataset ) );

	std::vector<Layer> vertices_per_layer;
	Layer layer_per_vertex;
	LayerAssignment( dataset, vertices_per_layer, layer_per_vertex );
	
	Dataset dummy_dataset = AddDummyVertices(dataset, vertices_per_layer, layer_per_vertex);
	CrossingMinimization( dummy_dataset, vertices_per_layer, layer_per_vertex );
	_graph.Load(std::make_shared<Dataset>(dummy_dataset));
	for ( int y = 0; y < vertices_per_layer.size( ); y++ )
	{
		std::vector<int> vertices = vertices_per_layer[y];
		for ( int x = 0; x < vertices.size( ); x++ )
		{
			int idx = vertices[x];
			auto& node = _graph.GetNodes( )[idx];
			node->SetNewPosition( glm::vec3( x * 100 + y * 20, y * -100, 0 ) );
		}
	}
	_graph.UpdateAABB( );
}

//--------------------------------------------------------------
Dataset Sugiyama::BreakCycles( Dataset& _dataset )
{
	// Make a copy
	auto& vertices = _dataset.vertices;

	// Create a new dataset without any edges
	Dataset new_dataset;
	new_dataset.SetKind( _dataset.GetKind( ) );
	new_dataset.edges = _dataset.edges;
	auto& new_vertices = new_dataset.vertices;

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

				auto& edge = new_dataset.edges[neighbor.edge_idx];
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
void Sugiyama::LayerAssignment( Dataset& _dataset, std::vector<Layer>& _vertices_per_layer, Layer& _layer_per_vertex )
{
	int layer = 0;
	Dataset copy_dataset = _dataset;
	int size = _dataset.vertices.size( );
	_layer_per_vertex.resize( size );

	bool hasSources = false;
	do
	{
		hasSources = false;
		_vertices_per_layer.push_back( {} );
		for ( auto& v : copy_dataset.vertices )
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
		for ( auto& v : copy_dataset.vertices )
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
Dataset Sugiyama::AddDummyVertices( Dataset& _dataset, std::vector<Layer>& _vertices_per_layer, Layer& _layer_per_vertex )
{
	Dataset copy_dataset = _dataset;
	auto& vertices = copy_dataset.vertices;
	int size = copy_dataset.edges.size();
	for(int j = 0; j < size; j++)
	{
		int current_edge_idx = j;
		auto& edge = copy_dataset.edges[current_edge_idx];
		int start_layer = _layer_per_vertex[edge.from_idx];
		int end_layer = _layer_per_vertex[edge.to_idx];
		if( end_layer - start_layer <= 1) continue;

		for(int i = start_layer; i < end_layer - 1; i++)
		{
			auto& e = copy_dataset.edges[current_edge_idx];
			RemoveNeighbors(copy_dataset, e );
			Vertex dummy_vertex;
			dummy_vertex.id = DummyId;
			dummy_vertex.idx = copy_dataset.vertices.size();
			copy_dataset.vertices.push_back(dummy_vertex);
			_layer_per_vertex.push_back(i + 1);
			_vertices_per_layer[i + 1].push_back(dummy_vertex.idx);

			Edge dummy_edge;
			dummy_edge.from_idx = dummy_vertex.idx;
			dummy_edge.to_idx = e.to_idx;
			dummy_edge.idx = copy_dataset.edges.size();
			e.to_idx = dummy_vertex.idx;
			
			AddNeighbors(copy_dataset, e );
			AddNeighbors(copy_dataset, dummy_edge);

			current_edge_idx = dummy_edge.idx;
			copy_dataset.edges.push_back(dummy_edge);
		}
	}
	return copy_dataset;
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
		auto& incoming = _dataset.vertices[neighbor.idx].incoming_neighbors;
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

void Sugiyama::RemoveNeighbors( Dataset& _dataset, Edge& _e )
{
	auto& outgoing = _dataset.vertices[_e.from_idx].neighbors;
	for(int i = 0; i < outgoing.size(); i++)
	{
		if(outgoing[i].idx == _e.to_idx)
		{
			outgoing[i] = outgoing[outgoing.size( ) - 1];
			outgoing.resize( outgoing.size( ) - 1 );
			break;
		}
	}

	auto& incoming = _dataset.vertices[_e.to_idx].incoming_neighbors;
	for(int i = 0; i < incoming.size(); i++)
	{
		if(incoming[i].idx == _e.from_idx)
		{
			incoming[i] = incoming[incoming.size( ) - 1];
			incoming.resize( incoming.size( ) - 1 );
			break;
		}
	}
}

void Sugiyama::AddNeighbors( Dataset& _dataset, Edge& _e )
{
	auto& outgoing = _dataset.vertices[_e.from_idx].neighbors;
	outgoing.push_back({ _e.to_idx, _e.idx});
	auto& incoming = _dataset.vertices[_e.to_idx].incoming_neighbors;
	incoming.push_back({ _e.from_idx, _e.idx});
}

void Sugiyama::RemoveIncomingNeighbors( Dataset& _dataset, Vertex& _v )
{
	// Remove incoming from neighbors
	for ( auto& neighbor : _v.incoming_neighbors )
	{
		// Remove edge from old dataset
		auto& outgoing = _dataset.vertices[neighbor.idx].neighbors;
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

//--------------------------------------------------------------
void Sugiyama::CrossingMinimization( Dataset& _dataset, std::vector<Layer>& _vertices_per_layer, Layer& _layer_per_vertex )
{
	auto& vertices = _dataset.vertices;

	std::shuffle( _vertices_per_layer[0].begin(), _vertices_per_layer[0].end(), RandomNumber::random_device );

	GetNeighbors get_neighbors = []( Vertex& v ) { return v.neighbors; };
	GetNeighbors get_reverse_neighbors = []( Vertex& v ) { return v.incoming_neighbors; };

	//auto& neighbors = get_neighbors( _dataset.vertices[0] );

	std::vector<float> all_coords( vertices.size() );

	int crossings = 0;
	int new_crossings = 1e30;

	do
	{
		crossings = new_crossings;
		new_crossings = 0;

		std::fill(all_coords.begin(), all_coords.end(), 0);
		// Go up
		for(int i = 1; i < _vertices_per_layer.size(); i++) 
		{
			Layer new_layer;
			BarycenterHeuristic( _dataset, all_coords, _vertices_per_layer[i - 1], _vertices_per_layer[i], new_layer, get_neighbors, get_reverse_neighbors );
			_vertices_per_layer[i] = new_layer;
		}
		
		std::fill( all_coords.begin(), all_coords.end(), 0 );
		// Go down
		for(int i = _vertices_per_layer.size() - 2; i >= 0; i--)
		{
			Layer new_layer;
			BarycenterHeuristic( _dataset, all_coords, _vertices_per_layer[i + 1], _vertices_per_layer[i], new_layer, get_reverse_neighbors, get_neighbors );
			_vertices_per_layer[i] = new_layer;
		}

		// Count crossings
		for (int i = 1; i < _vertices_per_layer.size(); i++)
		{
			new_crossings += Crossings( _dataset, _vertices_per_layer[i - 1], _vertices_per_layer[i] );
		}
	}
	while(new_crossings < crossings);

	_dataset.AddInfo( "Crossings", std::to_string( new_crossings ) );
}

bool Sugiyama::BarycenterHeuristic( Dataset& _dataset,
	std::vector<float>& _all_coords, 
	Layer& _layer_fixed, 
	Layer& _layer, 
	Layer& _new_layer,
	GetNeighbors _get_neighbors,
	GetNeighbors _get_reverse_neighbors )
{
	auto& vertices = _dataset.vertices;

	for (int i = 0; i < _layer_fixed.size(); i++)
	{
		auto& neighbors = _get_neighbors( vertices[_layer_fixed[i]] );
		for (auto& n : neighbors)
		{
			_all_coords[n.idx] += i;
		}
	}

	// barycenter, vertex idx
	std::vector<std::pair<float, int>> coords( _layer.size() );
	for (int i = 0; i < coords.size(); i++)
	{
		float coor = _all_coords[_layer[i]];
		coor /= (float)_get_reverse_neighbors(vertices[_layer[i]]).size();
		coords[i] = {coor, _layer[i]};
	}

	bool changed = false;
	std::stable_sort( coords.begin(), coords.end(), [&]( pair<float, int> a, pair<float, int> b ) {
		changed |= a.first < b.first;
		return a.first < b.first;
	});

	_new_layer.resize(coords.size());
	for (int i = 0; i < coords.size(); i++) _new_layer[i] = coords[i].second;
	return changed;
}

int Sugiyama::Crossings( Dataset& _dataset, Layer& _layer_1, Layer& _layer_2 )
{
	std::vector<bool> flags;
	std::vector<std::pair<int, int>> open_edges;
	int count = 0;
	// First open all edges
	for (int i = 0; i < _layer_1.size(); i++)
	{
		int vertex_idx = _layer_1[i];
		auto& neighbors = _dataset.vertices[vertex_idx].neighbors;

		for (auto& n : neighbors)
			open_edges.push_back( { vertex_idx, n.idx } );
	}
	flags.resize( open_edges.size(), false);

	// Then close all edges
	for (int i = _layer_2.size() - 1; i >= 0; i--)
	{
		int vertex_idx = _layer_2[i];
		auto& neighbors = _dataset.vertices[vertex_idx].neighbors;
		for (auto& n : neighbors)
		{
			std::pair<int, int> edge( n.idx, vertex_idx );
			// Backwards linear scan
			for (int j = open_edges.size() - 1; j >= 0; j--)
			{
				if (flags[j]) continue;
				// Encountered ourselves
				if (open_edges[j] == edge)
				{
					// Remove ourself from open_edges
					flags[j] = true;
				}

				if (open_edges[j].first != edge.first && open_edges[j].second != edge.second)
				{
					// Found a crossing
					count++;
				}
			}
		}
	}

	return count;
}


//--------------------------------------------------------------
void Sugiyama::VertexPositioning( Graph& _graph )
{

}


} // namespace DataVis