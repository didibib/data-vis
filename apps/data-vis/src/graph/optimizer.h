#pragma once

namespace DataVis {
	class Optimizer {
	public:
		static float LocalSearch(Graph&, int iterations);
	private:
		static float CalculateCost(Graph&);
		static float CalculateIncrementalCost( Graph&, uint idx0, uint idx1 );
		static float CalculateNodeCost( Graph&, uint idx );
		static void SwapPos(Graph&, float& current_cost);
	};
}