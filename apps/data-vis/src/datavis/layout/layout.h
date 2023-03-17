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
	virtual void Update( float ) {}
	virtual void Draw( ) {}
	virtual ~Layout() = default;
};

//--------------------------------------------------------------
// Random
//--------------------------------------------------------------
class RandomLayout final : public Layout
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
class GridLayout final : public Layout
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
class LocalSearch  final : public Layout
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
class RadialLayout final  : public Layout
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
class ForceDirectedLayout final  : public Layout
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
// Sugiyama Framework
//--------------------------------------------------------------
class Sugiyama final  : public Layout
{
public:
	using Layer = std::vector<int>;
	using GetNeighbors = std::function<std::vector<Neighbor>( Vertex& )>;
	using OSCMHeuristic = std::function<bool(Dataset&, Layer&, Layer&, Layer&, GetNeighbors)>;

	Sugiyama();
	bool Gui( IStructure& ) override;
	static void Apply( Graph&, const  OSCMHeuristic& oscm_heuristic, const  glm::vec2& node_offset, const  int& oscm_iterations );
private:

	std::vector<std::pair<std::string, OSCMHeuristic>> m_oscm_heuristics;
	OSCMHeuristic m_oscm_heuristic;
	int m_oscm_heuristic_idx = 0;

	int m_oscm_iterations = 100;
	glm::vec2 m_node_offset;

	// Sugiyama Framework
	static Dataset BreakCycles( Dataset& );
	static void LayerAssignment( Dataset&, std::vector<Layer>& vertices_per_layer, Layer& layer_per_vertex );
	static void AddDummyVertices( Dataset&, std::vector<Layer>& vertices_per_layer, Layer& layer_per_vertex );
	static int CrossingMinimization( Dataset&, std::vector<Layer>& vertices_per_layer,const OSCMHeuristic& heuristic, int iterations );
	static std::vector<float> VertexPositioning( Dataset& _dataset, std::vector<Layer>& vertices_per_layer, Layer& layer_per_vertex, float delta_x );

	//--------------------------------------------------------------
	// Layer Assignment
	//--------------------------------------------------------------
	static bool IsSink(const Vertex& );
	static bool IsSource(const Vertex& );
	static bool Has( std::function<bool( Vertex& )>, std::vector<Vertex>, Vertex& out );
	static void RemoveOutgoingNeighbors( Dataset&, Vertex& );
	static void RemoveIncomingNeighbors( Dataset&, Vertex& );
	static void RemoveNeighbors( Dataset&, Edge& );
	static void AddNeighbors( Dataset&, Edge& );

	//--------------------------------------------------------------
	// OSCM
	//--------------------------------------------------------------
	static bool OSCMBarycenterHeuristic( Dataset&, const Layer& layer_fixed, Layer& layer, Layer& new_layer, const GetNeighbors& get_neighbors );
	static bool OSCMMedianHeuristic( Dataset&, const Layer& layer_fixed, Layer& layer, Layer& new_layer, const GetNeighbors& get_neighbors );

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