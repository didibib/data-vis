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
	static void Apply( Graph& );
private:
	using Layer = std::vector<int>;
	using GetNeighbors = std::function<std::vector<Neighbor> (Vertex&)>;

	static const int VisitedIdx;
	static const std::string DummyId;
	static const int RemoveIdx;

	static Dataset BreakCycles( Dataset& );
	static void LayerAssignment( Dataset&, std::vector<Layer>& vertices_per_layer, Layer& layer_per_vertex );
	static Dataset AddDummyVertices( Dataset&, std::vector<Layer>& vertices_per_layer, Layer& layer_per_vertex );
	static void CrossingMinimization( Dataset&, std::vector<Layer>& _vertices_per_layer, Layer& _layer_per_vertex);
	static void VertexPositioning( Graph& );

	// Layer Assignment
	static bool IsSink( Vertex& );
	static bool IsSource( Vertex& );
	static bool Has( std::function<bool( Vertex& )>, std::vector<Vertex>, Vertex& out );
	static void RemoveOutgoingNeighbors( Dataset&, Vertex& );
	static void RemoveIncomingNeighbors( Dataset&, Vertex& );
	static void RemoveNeighbors( Dataset&, Edge& );
	static void AddNeighbors( Dataset&, Edge&);

	// OSCM
	static bool BarycenterHeuristic( Dataset&, std::vector<float>& all_coords, Layer& layer_fixed, Layer& layer, Layer& new_layer,
		GetNeighbors get_neighbors,
		GetNeighbors get_reverse_neighbors );

	static int Crossings( Dataset&, Layer& layer_1, Layer& layer_2 );

};
} // namespace DataVis