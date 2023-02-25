#include "precomp.h"

namespace DataVis {
	float Optimizer::LocalSearch(Graph& _graph, int _iterations)
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
			glm::vec3 start = _graph.Nodes()[startIdx].get().GetPosition();
			glm::vec3 end = _graph.Nodes()[endIdx].get().GetPosition();
			cost += glm::distance(start, end);
		}
		return cost;
	}

	float Optimizer::CalculateIncrementalCost( Graph& _graph, uint _idx0, uint _idx1 )
	{
		float cost = 0;
		// Subtract cost of all edges before swap at both indices
		cost -= CalculateNodeCost( _graph, _idx0 );
		cost -= CalculateNodeCost( _graph, _idx1 );
		// Temporarily swap the positions of the two nodes
		auto& node0 = _graph.Nodes()[_idx0].get();
		auto& node1 = _graph.Nodes()[_idx1].get();
		auto pos0 = node0.GetPosition();
		auto pos1 = node1.GetPosition();
		node0.SetPosition(pos1);
		node1.SetPosition(pos0);
		// Add cost of all edges after swap at both indices
		cost += CalculateNodeCost( _graph, _idx0 );
		cost += CalculateNodeCost( _graph, _idx1 );
		// Swap back
		node0.SetPosition(pos0);
		node1.SetPosition(pos1);
		return cost;
	}

	float Optimizer::CalculateNodeCost( Graph& _graph, uint _idx )
	{
		float cost = 0;
		auto& node = _graph.Vertices()[_idx];
		for ( auto& edge : node.m_out_edges )
		{
			int targetIdx = edge.m_target;
			glm::vec3 start = _graph.Nodes()[_idx].get().GetPosition();
			glm::vec3 end = _graph.Nodes()[targetIdx].get().GetPosition();
			cost += glm::distance( start, end );
		}
		return cost;
	}

	void Optimizer::SwapPos( Graph& _graph, float& _currCost )
	{
		auto& nodes = _graph.Vertices();
		int range = nodes.size() - 1;
		uint idx0 = Random::Range( range );
		uint idx1 = Random::Range( range );
		while ( idx0 == idx1 ) idx1 = Random::Range( range );
		float costDif = CalculateIncrementalCost( _graph, idx0, idx1 );
		if ( costDif < 0 )
		{
			// Swap the nodes
			auto& node0 = _graph.Nodes()[idx0].get();
			auto& node1 = _graph.Nodes()[idx1].get();
			auto pos0 = node0.GetPosition();
			auto pos1 = node1.GetPosition();
			node0.SetPosition(pos1);
			node1.SetPosition(pos0);
			_currCost += costDif;
		}
	}
}