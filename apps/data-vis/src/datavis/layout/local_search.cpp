#include "precomp.h"

//--------------------------------------------------------------
// Local Search
//--------------------------------------------------------------
namespace DataVis
{
#pragma region Local Search
bool LocalSearch::Gui(IStructure& _structure)
{
	bool active = false;
	if (ImGui::TreeNode("Local Search"))
	{
		ImGui::InputInt("Iterations", &m_iterations);

		if (ImGui::Button("Apply"))
		{
			Apply(_structure, m_iterations);
			active = true;
		}

		ImGui::TreePop();
		ImGui::Separator();
	}
	return active;
}

void LocalSearch::Apply(IStructure& _structure, int _iterations)
{
	float cost = CalculateCost(_structure);
	for (size_t i = 0; i < _iterations; i++)
	{
		SwapPos(_structure, cost);
	}
}

float LocalSearch::CalculateCost(const IStructure& _structure)
{
	float cost = 0;
	for (const auto& edge : _structure.dataset->edges)
	{
		const VertexIdx startIdx = edge.from_idx;
		const VertexIdx endIdx = edge.to_idx;
		glm::vec3 start = _structure.nodes[startIdx]->GetPosition();
		glm::vec3 end = _structure.nodes[endIdx]->GetPosition();
		cost += glm::distance(start, end);
	}
	return cost;
}

float LocalSearch::CalculateIncrementalCost(IStructure& _structure, uint _idx0, uint _idx1)
{
	float cost = 0;
	// Subtract cost of all edges before swap at both indices
	cost -= CalculateNodeCost(_structure, _idx0);
	cost -= CalculateNodeCost(_structure, _idx1);
	// Temporarily swap the positions of the two nodes
	const auto& node0 = _structure.nodes[_idx0];
	const auto& node1 = _structure.nodes[_idx1];
	const auto pos0 = node0->GetPosition();
	const auto pos1 = node1->GetPosition();
	node0->SetPosition(pos1);
	node1->SetPosition(pos0);
	// Add cost of all edges after swap at both indices
	cost += CalculateNodeCost(_structure, _idx0);
	cost += CalculateNodeCost(_structure, _idx1);
	// Swap back
	node0->SetPosition(pos0);
	node1->SetPosition(pos1);
	return cost;
}

float LocalSearch::CalculateNodeCost(const IStructure& _structure, uint _idx)
{
	float cost = 0;
	const auto& node = _structure.nodes[_idx];
	for (const auto& neigbor : node->neighbors)
	{
		glm::vec3 start = node->GetPosition();
		glm::vec3 end = neigbor->GetPosition();
		cost += glm::distance(start, end);
	}
	return cost;
}

void LocalSearch::SwapPos(IStructure& _structure, float& _currCost)
{
	const auto& nodes = _structure.nodes;
	const int range = nodes.size() - 1;
	const uint idx0 = Random::Range(range);
	uint idx1 = Random::Range(range);
	while (idx0 == idx1) idx1 = Random::Range(range);
	const float costDif = CalculateIncrementalCost(_structure, idx0, idx1);
	if (costDif < 0)
	{
		// Swap the nodes
		const auto& node0 = _structure.nodes[idx0];
		const auto& node1 = _structure.nodes[idx1];
		const auto pos0 = node0->GetPosition();
		const auto pos1 = node1->GetPosition();
		node0->SetPosition(pos1);
		node1->SetPosition(pos0);
		_currCost += costDif;
	}
}
#pragma endregion // Local Search
} // DataVis