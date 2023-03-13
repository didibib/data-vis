#pragma once

namespace DataVis
{
//--------------------------------------------------------------
// Layout
//--------------------------------------------------------------
class Layout
{
public:
	virtual bool Gui(IStructure&) = 0;
	virtual void Update(float) {};
	virtual void Draw() {}
};

//--------------------------------------------------------------
// Random
//--------------------------------------------------------------
class Random : public Layout
{
public:
	bool Gui(IStructure&) override;
	static void Apply(IStructure&, int width, int height);
	
private:
	int m_width = 800, m_height = 800;
};

//--------------------------------------------------------------
// Grid
//--------------------------------------------------------------
class Grid : public Layout
{
public:
	bool Gui(IStructure&) override;
	static void Apply(IStructure&, int width, int height, float step);
	
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
	bool Gui(IStructure&) override;
	static void Apply(IStructure&, int _iterations);
private:
	static float CalculateCost(IStructure& _structure);
	static float CalculateIncrementalCost(IStructure& _structure, uint _idx0, uint _idx1);
	static float CalculateNodeCost(IStructure& _structure, uint _idx);
	static void SwapPos(IStructure& _structure, float& _currCost);
	int m_iterations = 10000;
};

//--------------------------------------------------------------
// Radial
//--------------------------------------------------------------
class Radial : public Layout
{
public:
	bool Gui(IStructure&) override;
	void Update(float) override;
	void Draw() override;
	static void Apply(Tree&, float start, float step);
private:
	// Pavlo, 2006 https://scholarworks.rit.edu/cgi/viewcontent.cgi?referer=&httpsredir=1&article=1355&context=theses
	static void SubTree(Tree::Node&, float angle_start, float angle_end, int depth, float step, float delta_angle);
	float m_start = 100, m_step = 100;
	Rings m_rings;
};

//--------------------------------------------------------------
// Force Directed
//--------------------------------------------------------------
class ForceDirected : public Layout
{
public:
	bool Gui(IStructure&) override;
	static void Apply(IStructure&, float _C, float _t, int _iterations);
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
	bool Gui(IStructure&) override;
	static void Apply(Graph&);
private:
	static const int VisitedIdx;
	static const int DummyIdx;
	static const int RemoveIdx;
	static Dataset BreakCycles(Graph&);
	static void LayerAssignment(Graph&, std::vector<std::vector<int>>& vertices_per_layer, std::vector<int>& layer_per_vertex);
	static void CrossingMinimization(Graph&);
	static void VertexPositioning(Graph&);
	
	static bool IsSink( Vertex&  );
	static bool IsSource( Vertex&  );
	static bool Has( std::function<bool( Vertex& )>, std::vector<Vertex>, Vertex& out );
	static void RemoveOutgoingNeighbors(Dataset&, Vertex&);
	static void RemoveIncomingNeighbors(Dataset&, Vertex&);
};
} // namespace DataVis