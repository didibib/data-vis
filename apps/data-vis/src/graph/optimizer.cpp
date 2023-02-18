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

	float Optimizer::CalculateIncrementalCost( Graph& _graph, uint idx0, uint idx1 )
	{
		float cost = 0;
		// Subtract cost of all edges before swap at both indices
		cost -= CalculateNodeCost( _graph, idx0 );
		cost -= CalculateNodeCost( _graph, idx1 );
		// Temporarily swap the positions of the two nodes
		auto& nodes = _graph.Nodes();
		auto& pos0 = nodes[idx0].m_property.position;
		auto& pos1 = nodes[idx1].m_property.position;
		auto temp = pos0;
		pos0 = pos1;
		pos1 = temp;
		// Add cost of all edges after swap at both indices
		cost += CalculateNodeCost( _graph, idx0 );
		cost += CalculateNodeCost( _graph, idx1 );
		// Swap back
		pos1 = pos0;
		pos0 = temp;
		return cost;
	}

	float Optimizer::CalculateNodeCost( Graph& _graph, uint idx )
	{
		float cost = 0;
		auto& node = _graph.Nodes()[idx];
		for ( auto& edge : node.m_out_edges )
		{
			int targetIdx = edge.m_target;
			glm::vec3 start = node.m_property.position;
			glm::vec3 end = _graph.Nodes()[targetIdx].m_property.position;
			cost += glm::distance( start, end );
		}
		return cost;
	}

#if 1
	void Optimizer::SwapPos( Graph& _graph, float& _currCost )
	{
		auto& nodes = _graph.Nodes();
		int range = nodes.size() - 1;
		uint idx0 = RandomRange( range );
		uint idx1 = RandomRange( range );
		while ( idx0 == idx1 ) idx1 = RandomRange( range );
		float costDif = CalculateIncrementalCost( _graph, idx0, idx1 );
		if ( costDif < 0 )
		{
			// Swap the nodes
			auto& pos0 = nodes[idx0].m_property.position;
			auto& pos1 = nodes[idx1].m_property.position;
			auto temp = pos0;
			pos0 = pos1;
			pos1 = temp;
			_currCost += costDif;
		}
	}
#else
	void Optimizer::SwapPos( Graph& _graph, float& _currCost )
	{
		auto& nodes = _graph.Nodes();
		int range = nodes.size() - 1;
		uint idx0 = RandomRange( range );
		uint idx1 = RandomRange( range );
		while ( idx0 == idx1 ) idx1 = RandomRange( range );
		auto& pos0 = nodes[idx0].m_property.position;
		auto& pos1 = nodes[idx1].m_property.position;
		auto temp = pos0;
		pos0 = pos1;
		pos1 = temp;
		float newCost = CalculateCost( _graph );
		if ( newCost >= _currCost )
		{
			pos1 = pos0;
			pos0 = temp;
		}
		else _currCost = newCost;
	}
#endif
}