#include "precomp.h"

namespace DataVis {
	float Optimizer::LocalSearchSimple(Graph& _graph, int _iterations)
	{		
		float cost = CalculateCost(_graph);
		for (size_t i = 0; i < _iterations; i++)
		{
			SwapPos(_graph, cost);
		}
		return cost;
	}

	float Optimizer::CalculateCost(Graph& _graph)
	{
		float cost = 0;
		for (auto& edge : _graph.Edges()) {
			int startIdx = edge.m_source;
			int endIdx = edge.m_target;
			glm::vec3 start = _graph.Nodes()[startIdx].m_property.position;
			glm::vec3 end = _graph.Nodes()[endIdx].m_property.position;
			cost += glm::distance(start, end);
		}
		return cost;
	}

	void Optimizer::SwapPos(Graph& _graph, float& _currCost)
	{		
		auto& nodes = _graph.Nodes();
		int range = nodes.size() - 1;
		uint idx0 = RandomRange(range);
		uint idx1 = RandomRange(range);
		while(idx0 == idx1) idx1 = RandomRange(range);
		auto& pos0 = nodes[idx0].m_property.position;
		auto& pos1 = nodes[idx1].m_property.position;
		auto temp = pos0;
		pos0 = pos1;
		pos1 = temp;
		float newCost = CalculateCost(_graph);
		if (newCost >= _currCost) {
			pos1 = pos0;
			pos0 = temp;
		}
		else _currCost = newCost;
	}
}