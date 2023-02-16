#pragma once

namespace DataVis {
	class Optimizer {
	public:
		static float LocalSearchSimple(Graph&, int iterations);
	private:
		static float CalculateCost(Graph&);
		static void SwapPos(Graph&, float& current_cost);
	};
}