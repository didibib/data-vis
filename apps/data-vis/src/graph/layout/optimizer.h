#pragma once

namespace DataVis
{
class Optimizer
{
	//--------------------------------------------------------------
	// Local Search
	//--------------------------------------------------------------
public:
	static float LocalSearch(IStructure&, int iterations);
private:
	static float CalculateCost(IStructure&);
	static float CalculateIncrementalCost(IStructure&, uint idx0, uint idx1);
	static float CalculateNodeCost(IStructure&, uint idx);
	static void SwapPos(IStructure&, float& current_cost);
	//--------------------------------------------------------------
	// Force Directed
	//--------------------------------------------------------------
public:
	static void ForceDirected(IStructure&, float C, float t, int iterations);

};
}