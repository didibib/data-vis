#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// DR Quality Metrics
//--------------------------------------------------------------
DRQualityMetrics::~DRQualityMetrics()
{
	delete m_shepard_xs;
	delete m_shepard_ys;
}

void DRQualityMetrics::MetricGui()
{
	if (m_metrics.size() <= 0) return;

	static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg
		| ImGuiTableFlags_ContextMenuInBody;

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(3, 5));
	if (ImGui::BeginTable("DR Quality Metrics", 2, flags))
	{
		for (const auto& [k, v] : m_metrics)
		{
			ImGui::TableNextRow();
			// Column 
			ImGui::TableNextColumn();
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(123, 123, 123, 255));
			ImGui::Text(k.c_str());
			ImGui::PopStyleColor();
			// Column
			ImGui::TableNextColumn();
			ImGui::Text(v.c_str());
		}
		ImGui::EndTable();
	}
	ImGui::PopStyleVar();

	if (ImPlot::BeginPlot("Shepard Plot"))
	{
		ImPlot::PlotScatter("Shepard Points", m_shepard_xs, m_shepard_ys, m_shepard_count);
		/*ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
		ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 6, ImPlot::GetColormapColor(1), IMPLOT_AUTO, ImPlot::GetColormapColor(1));
		ImPlot::PlotScatter("Data 2", xs2, ys2, 50);
		ImPlot::PopStyleVar();*/
		ImPlot::EndPlot();
	}
}

void DRQualityMetrics::ComputeMetrics(IStructure& _structure)
{
	m_metrics.clear();

	m_metrics.emplace_back("Normalized Stress", std::to_string(NormalizedStress(_structure)));
	m_metrics.emplace_back("Trustworthiness", std::to_string(Trustworthiness(_structure, 10)));
	m_metrics.emplace_back("Continuity", std::to_string(Continuity(_structure, 10)));

	auto& shepard_points = ShepardPoints(_structure);
	m_shepard_count = shepard_points.first.size();
	delete m_shepard_xs;
	delete m_shepard_ys;
	m_shepard_xs = new float[m_shepard_count];
	m_shepard_ys = new float[m_shepard_count];
	std::copy(shepard_points.first.begin(), shepard_points.first.end(), m_shepard_xs);
	std::copy(shepard_points.second.begin(), shepard_points.second.end(), m_shepard_ys);
}

float DRQualityMetrics::NormalizedStress(IStructure& _structure)
{
	std::unique_ptr<smat::Matrix<double>> D;
	FloydWarshall(*_structure.dataset, D);

	auto& nodes = _structure.nodes;

	float sum_dif = 0;
	float sum = 0;
	for (int i = 0; i < nodes.size(); i++)
	{
		auto& x_i = nodes[i];
		for (int j = 0; j < nodes.size(); j++)
		{
			if (i == j) continue;
			auto& x_j = nodes[j];
			float delta_n = D->get(i, j);
			float delta_q = glm::length(x_i->GetPosition() - x_j->GetPosition());
			float diff = delta_n - delta_q;
			sum_dif += diff * diff;
			sum += delta_n * delta_n;
		}
	}
	return sum_dif / sum;
}

std::pair<std::vector<float>, std::vector<float>> DRQualityMetrics::ShepardPoints(IStructure& _structure)
{
	std::unique_ptr<smat::Matrix<double>> D;
	FloydWarshall(*_structure.dataset, D);

	auto& nodes = _structure.nodes;
	std::vector<float> xs;
	std::vector<float> ys;
	for (int i = 0; i < nodes.size(); i++)
	{
		for (int j = 0; j < nodes.size(); j++)
		{
			if (i == j) continue;
			float delta_p = glm::length(nodes[i]->GetPosition() - nodes[j]->GetPosition());
			float delta_n = D->get(i, j);
			xs.emplace_back(delta_n);
			ys.emplace_back(delta_p);
		}
	}
	return { xs, ys };
}

float DRQualityMetrics::Trustworthiness(IStructure& _structure, int _K)
{
	auto& nodes = _structure.nodes;
	// K nearest neighbors of vertex i before projection are:
	// First K elements of v_i_K[i]
	uint N = nodes.size();
	qdtsne::NeighborList<int, double> v_i_K;
	FloydWarshall(*_structure.dataset, v_i_K);

	float sum = 0;
	for (int i = 0; i < nodes.size(); i++)
	{
		std::vector<std::pair<int, double>> dists;
		for (int j = 0; j < nodes.size(); j++)
		{
			if (i == j) continue;
			float dist = glm::length(nodes[i]->GetPosition() - nodes[j]->GetPosition());
			dists.emplace_back(j, dist);
		}
		std::sort(dists.begin(), dists.end(), [](std::pair<int, double> lhs, std::pair<int, double> rhs) {
			return lhs.second < rhs.second;
			});

		// Loop over each point in m_i_K
		for (int j = 0; j < _K; j++)
		{
			auto& pair = dists[j];
			// Check if point in v_i_K
			bool original_neighbour = false;
			for (int k = 0; k < _K; k++)
				if (v_i_K[i][k].first == pair.first)
				{
					original_neighbour = true;
					break;
				}

			if (original_neighbour) continue;

			// We have a false neighbour
			int rank = 0;
			// Loop over the original dataset
			for (int k = _K; k < v_i_K[i].size(); k++)
				if (v_i_K[i][k].first == pair.first)
				{
					rank = k;
					break;
				}

			sum += rank - _K;
		}
	}
	return 1 - sum * 2 / static_cast<float>(N * _K * (2 * N - 3 * _K - 1));
}

float DRQualityMetrics::Continuity(IStructure& _structure, int _K)
{
	auto& nodes = _structure.nodes;
	// K nearest neighbors of vertex i before projection are:
	// First K elements of v_i_K[i]
	uint N = nodes.size();
	qdtsne::NeighborList<int, double> v_i_K;
	FloydWarshall(*_structure.dataset, v_i_K);

	float sum = 0;
	for (int i = 0; i < nodes.size(); i++)
	{
		std::vector<std::pair<int, double>> dists;
		for (int j = 0; j < nodes.size(); j++)
		{
			if (i == j) continue;
			float dist = glm::length(nodes[i]->GetPosition() - nodes[j]->GetPosition());
			dists.emplace_back(j, dist);
		}
		std::sort(dists.begin(), dists.end(), [](std::pair<int, double> lhs, std::pair<int, double> rhs) {
			return lhs.second < rhs.second;
			});

		// Loop over each point in v_i_K
		for (int j = 0; j < _K; j++)
		{
			auto& pair = v_i_K[i][j];
			// Check if point in m_i_K
			bool projected_neighbour = false;
			for (int k = 0; k < _K; k++)
				if (dists[j].first == pair.first)
				{
					projected_neighbour = true;
					break;
				}

			if (projected_neighbour) continue;

			// We have a missing neighbour
			int rank = 0;
			// Loop over the projected dataset
			for (int k = _K; k < dists.size(); k++)
				if (dists[k].first == pair.first)
				{
					rank = k;
					break;
				}

			sum += rank - _K;
		}
	}
	return 1 - sum * 2 / static_cast<float>(N * _K * (2 * N - 3 * _K - 1));
}

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
			ComputeMetrics(_structure);
			active = true;
		}

		MetricGui();

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
			ComputeMetrics(_structure);
			active = true;
		}

		MetricGui();

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
