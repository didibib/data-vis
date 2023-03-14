#pragma once

namespace DataVis
{
//--------------------------------------------------------------
// Layout
//--------------------------------------------------------------
class Layout
{
public:
	virtual bool Gui( IStructure& ) = 0;
	virtual void Update( float ) {};
	virtual void Draw( ) {}
};

//--------------------------------------------------------------
// Random
//--------------------------------------------------------------
class Random : public Layout
{
public:
	bool Gui( IStructure& ) override;
	static void Apply( IStructure&, int width, int height );

private:
	int m_width = 800, m_height = 800;
};

//--------------------------------------------------------------
// Grid
//--------------------------------------------------------------
class Grid : public Layout
{
public:
	bool Gui( IStructure& ) override;
	static void Apply( IStructure&, int width, int height, float step );

private:
	int m_width = 800, m_height = 800;
	float m_step = 100;
};

//--------------------------------------------------------------
// Local Search
//--------------------------------------------------------------
class LocalSearch : public Layout
{
public:
	bool Gui( IStructure& ) override;
	static void Apply( IStructure&, int _iterations );
private:
	static float CalculateCost( IStructure& _structure );
	static float CalculateIncrementalCost( IStructure& _structure, uint _idx0, uint _idx1 );
	static float CalculateNodeCost( IStructure& _structure, uint _idx );
	static void SwapPos( IStructure& _structure, float& _currCost );
	int m_iterations = 10000;
};

//--------------------------------------------------------------
// Radial
//--------------------------------------------------------------
class Radial : public Layout
{
public:
	bool Gui( IStructure& ) override;
	void Update( float ) override;
	void Draw( ) override;
	static void Apply( Tree&, float start, float step );
private:
	// Pavlo, 2006 https://scholarworks.rit.edu/cgi/viewcontent.cgi?referer=&httpsredir=1&article=1355&context=theses
	static void SubTree( Tree::Node&, float angle_start, float angle_end, int depth, float step, float delta_angle );
	float m_start = 100, m_step = 100;
	Rings m_rings;
};

//--------------------------------------------------------------
// Force Directed
//--------------------------------------------------------------
class ForceDirected : public Layout
{
public:
	bool Gui( IStructure& ) override;
	static void Apply( IStructure&, float _C, float _t, int _iterations );
private:
	float m_C = 0.5, m_T = 0.002;
	int m_iterations = 10;
	bool m_enabled = false;
};

//--------------------------------------------------------------
// Force Directed
//--------------------------------------------------------------
class Sugiyama : public Layout
{
public:
	bool Gui( IStructure& ) override;
	static void Apply( Graph&, int oscm_iterations, float delta_x );
private:
	using Layer = std::vector<int>;
	using GetNeighbors = std::function<std::vector<Neighbor>( Vertex& )>;

	static const int VisitedIdx;
	static const std::string DummyId;
	static const int RemoveIdx;

	int m_oscm_iterations = 100;

	// Sugiyama Framework
	static Dataset BreakCycles( Dataset& );
	static void LayerAssignment( Dataset&, std::vector<Layer>& vertices_per_layer, Layer& layer_per_vertex );
	static void AddDummyVertices( Dataset&, std::vector<Layer>& vertices_per_layer, Layer& layer_per_vertex );
	static int CrossingMinimization( Dataset&, std::vector<Layer>& vertices_per_layer, int iterations );
	static void VertexPositioning( Dataset& _dataset, std::vector<Layer>& vertices_per_layer, Layer& layer_per_vertex, float delta_x );

	//--------------------------------------------------------------
	// Layer Assignment
	//--------------------------------------------------------------
	static bool IsSink( Vertex& );
	static bool IsSource( Vertex& );
	static bool Has( std::function<bool( Vertex& )>, std::vector<Vertex>, Vertex& out );
	static void RemoveOutgoingNeighbors( Dataset&, Vertex& );
	static void RemoveIncomingNeighbors( Dataset&, Vertex& );
	static void RemoveNeighbors( Dataset&, Edge& );
	static void AddNeighbors( Dataset&, Edge& );

	//--------------------------------------------------------------
	// OSCM
	//--------------------------------------------------------------
	static bool BarycenterHeuristic( Dataset&, std::vector<float>& all_coords, Layer& layer_fixed, Layer& layer, Layer& new_layer,
		GetNeighbors get_neighbors,
		GetNeighbors get_reverse_neighbors );

	static int Crossings( Dataset&, Layer& layer_1, Layer& layer_2 );

	//--------------------------------------------------------------
	// Node Positioning
	//--------------------------------------------------------------
	static void FlagType1Conflicts(
		Dataset&, std::vector<Layer>& vertices_per_layer,
		Layer& layer_per_vertex,
		std::vector<std::pair<int, int>>& flags );

	static void VerticalAlignment(
		Dataset&,
		std::vector<Layer>& vertices_per_layer,
		Layer& layer_per_vertex,
		Layer& pos_per_vertex,
		std::vector<int>& root,
		std::vector<int>& align,
		std::vector<std::pair<int, int>>& flags );

	static void HorizontalCompaction(
		Dataset&,
		std::vector<Layer>& vertices_per_layer,
		Layer& layer_per_vertex,
		Layer& pos_per_vertex,
		std::vector<int>& root,
		std::vector<int>& align, 
		std::vector<float>& x_per_vertex,
		float delta);
};
} // namespace DataVis