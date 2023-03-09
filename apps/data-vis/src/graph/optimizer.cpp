#include "precomp.h"

namespace DataVis {
	float Optimizer::LocalSearch(IStructure& _graph, int _iterations)
	{		
		float cost = CalculateCost(_graph);
		for (size_t i = 0; i < _iterations; i++)
		{
			SwapPos(_graph, cost);
		}
		return cost;
	}

	float Optimizer::CalculateCost(IStructure& _structure)
	{
		float cost = 0;
		for (auto& edge : _structure.GetDataset().GetEdges()) {
			VertexIdx startIdx = edge.from_idx;
			VertexIdx endIdx = edge.to_idx;
			glm::vec3 start = _structure.GetNodes()[startIdx]->GetPosition();
			glm::vec3 end = _structure.GetNodes()[endIdx]->GetPosition();
			cost += glm::distance(start, end);
		}
		return cost;
	}

	float Optimizer::CalculateIncrementalCost(IStructure& _structure, uint _idx0, uint _idx1 )
	{
		float cost = 0;
		// Subtract cost of all edges before swap at both indices
		cost -= CalculateNodeCost(_structure, _idx0 );
		cost -= CalculateNodeCost(_structure, _idx1 );
		// Temporarily swap the positions of the two nodes
		auto& node0 = _structure.GetNodes()[_idx0];
		auto& node1 = _structure.GetNodes()[_idx1];
		auto pos0 = node0->GetPosition();
		auto pos1 = node1->GetPosition();
		node0->SetPosition(pos1);
		node1->SetPosition(pos0);
		// Add cost of all edges after swap at both indices
		cost += CalculateNodeCost(_structure, _idx0 );
		cost += CalculateNodeCost(_structure, _idx1 );
		// Swap back
		node0->SetPosition(pos0);
		node1->SetPosition(pos1);
		return cost;
	}

	float Optimizer::CalculateNodeCost(IStructure& _structure, uint _idx )
	{
		float cost = 0;
		auto& node = _structure.GetNodes()[_idx];
		for ( auto& neigbor : node->neighbors)
		{
			glm::vec3 start = node->GetPosition();
			glm::vec3 end = neigbor->GetPosition();
			cost += glm::distance( start, end );
		}
		return cost;
	}

	void Optimizer::SwapPos(IStructure& _structure, float& _currCost )
	{
		auto& nodes = _structure.GetNodes();
		int range = nodes.size() - 1;
		uint idx0 = Random::Range( range );
		uint idx1 = Random::Range( range );
		while ( idx0 == idx1 ) idx1 = Random::Range( range );
		float costDif = CalculateIncrementalCost(_structure, idx0, idx1 );
		if ( costDif < 0 )
		{
			// Swap the nodes
			auto& node0 = _structure.GetNodes()[idx0];
			auto& node1 = _structure.GetNodes()[idx1];
			auto pos0 = node0->GetPosition();
			auto pos1 = node1->GetPosition();
			node0->SetPosition(pos1);
			node1->SetPosition(pos0);
			_currCost += costDif;
		}
	}
}