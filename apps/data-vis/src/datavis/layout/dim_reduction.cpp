#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// TSNE Layout
//--------------------------------------------------------------
bool TSNELayout::Gui(IStructure& _structure)
{
	bool active = false;
	if (ImGui::TreeNode("TSNE Projection"))
	{
		ImGui::InputInt("Iterations", &m_iterations);
		ImGui::InputInt("Scale", &m_scale);

		if (ImGui::Button("Apply"))
		{
			Apply(_structure, m_iterations, m_scale);
			metrics->ComputeMetrics(_structure);
			active = true;
		}

		ImGui::TreePop();
		ImGui::Separator();
	}
	return active;
}

void TSNELayout::Apply(IStructure& _structure, const int _iterations, const int _scale)
{
	qdtsne::NeighborList<int, double> D;
	FloydWarshall(*_structure.dataset, D);

	const int size = _structure.dataset->vertices.size();
	// Run T-SNE
	qdtsne::Tsne tsne;
	tsne.set_max_iter(_iterations);
	std::vector<double> Y = qdtsne::initialize_random(size); // initial coordinates
	tsne.run(D, Y.data());

	for (size_t i = 0; i < _structure.nodes.size(); i++)
	{
		auto& node = _structure.nodes[i];
		glm::vec3 pos = glm::vec3(Y[i], Y[i + 1], 0);
		pos *= _scale;
		node->SetNewPosition(pos);
	}
	_structure.UpdateAABB();
	_structure.UpdateEdges();
}

//--------------------------------------------------------------
// MDS Layout
//--------------------------------------------------------------
bool MDSLayout::Gui(IStructure& _structure)
{
	bool active = false;
	if (ImGui::TreeNode("MDS Projection"))
	{
		ImGui::InputInt("Iterations", &m_iterations);
		ImGui::InputInt("Scale", &m_scale);

		if (ImGui::Button("Apply"))
		{
			Apply(_structure, m_iterations, m_scale);
			metrics->ComputeMetrics(_structure);
			active = true;
		}

		ImGui::TreePop();
		ImGui::Separator();
	}
	return active;
}

void MDSLayout::Apply(IStructure& _structure, const int _iterations, const int _scale)
{
	std::unique_ptr<smat::Matrix<double>> D;
	FloydWarshall(*_structure.dataset, D);

	// Run MDS
	auto Y = smat::MDS_UCF(D.get(), nullptr, 2, _iterations);
	for (size_t i = 0; i < _structure.nodes.size(); i++)
	{
		auto& node = _structure.nodes[i];
		glm::vec3 pos = glm::vec3(Y->get(i, 0), Y->get(i, 1), 0);
		pos *= _scale;
		node->SetNewPosition(pos);
	}
	_structure.UpdateAABB();
	_structure.UpdateEdges();
}
}
