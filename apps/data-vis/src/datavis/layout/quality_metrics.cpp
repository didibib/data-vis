#include "precomp.h"

namespace DataVis
{



void GraphQualityMetrics::MetricGui()
{
	if (m_metrics.size() <= 0) return;

	static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_RowBg
		| ImGuiTableFlags_ContextMenuInBody;

	ImGui::PushStyleVar( ImGuiStyleVar_CellPadding, ImVec2( 3, 5 ) );
	if (ImGui::BeginTable( "Quality Metrics", 2, flags ))
	{
		for (const auto& [k, v] : m_metrics)
		{
			ImGui::TableNextRow();
			// Column 
			ImGui::TableNextColumn();
			ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 123, 123, 123, 255 ) );
			ImGui::Text( k.c_str() );
			ImGui::PopStyleColor();
			// Column
			ImGui::TableNextColumn();
			ImGui::Text( v.c_str() );
		}
		ImGui::EndTable();
	}
	ImGui::PopStyleVar();

}

void GraphQualityMetrics::ComputeMetrics( IStructure& _structure )
{
	m_metrics.clear();

	auto& crossing_data = Crossings( _structure );
	m_metrics.emplace_back( "# of Crossings", std::to_string( crossing_data.first ) );
	if(crossing_data.first > 0)
		m_metrics.emplace_back( "Crossing Resolution", std::to_string( crossing_data.second ) );
	m_metrics.emplace_back( "Stress", std::to_string( Stress( _structure ) ) );
}

// https://stackoverflow.com/questions/14176776/find-out-if-2-lines-intersect
std::pair<int, float> GraphQualityMetrics::Crossings( IStructure& _structure )
{
	auto& edges = _structure.edges;

	int crossings = 0;
	float min_angle = MAX_FLOAT;

	// Loop over all pairs
	for (int i = 0; i < edges.size() - 1; i++)
	{
		for (int j = i + 1; j < edges.size(); j++)
		{
			auto& edge_1 = edges[i]->points;
			auto& edge_2 = edges[j]->points;

			for (int k1 = 0; k1 < edge_1.size() - 1; k1++)
			{
				for (int k2 = 0; k2 < edge_2.size() - 1; k2++)
				{
					auto& p1 = edge_1[k1].new_value;
					auto& p2 = edge_1[k1 + 1].new_value;
					auto& q1 = edge_2[k2].new_value;
					auto& q2 = edge_2[k2 + 1].new_value;

					if ((((q1.x - p1.x) * (p2.y - p1.y) - (q1.y - p1.y) * (p2.x - p1.x))
						* ((q2.x - p1.x) * (p2.y - p1.y) - (q2.y - p1.y) * (p2.x - p1.x)) < 0)
						&&
						(((p1.x - q1.x) * (q2.y - q1.y) - (p1.y - q1.y) * (q2.x - q1.x))
							* ((p2.x - q1.x) * (q2.y - q1.y) - (p2.y - q1.y) * (q2.x - q1.x)) < 0))
					{
						// Segments intersect
						crossings++;
						auto& p = glm::normalize( p2 - p1 );
						auto& q = glm::normalize( q2 - q1 );
						float angle = acos( glm::dot( p, q ) );
						min_angle = min( min_angle, angle );
					}
				}
			}
		}
	}
	return { crossings, min_angle };
}

float GraphQualityMetrics::Stress( IStructure& _structure )
{
	std::unique_ptr<smat::Matrix<double>> D;
	FloydWarshall( *_structure.dataset, D );

	auto& nodes = _structure.nodes;

	float d_ij_min = MAX_FLOAT;
	float d_ij_max = -MAX_FLOAT;
	float projected_min = MAX_FLOAT;
	float projected_max = -MAX_FLOAT;
	for (int i = 0; i < nodes.size(); i++)
	{
		for (int j = 0; j < nodes.size(); j++)
		{
			if (i == j) continue;
			float d_ij = D->get( i, j );
			float projected = glm::length( nodes[i]->GetNewPosition() - nodes[j]->GetNewPosition() );
			d_ij_min = min( d_ij_min, d_ij );
			d_ij_max = max( d_ij_max, d_ij );
			projected_min = min( projected_min, projected );
			projected_max = max( projected_max, projected );
		}
	}

	float sum = 0;
	for (int i = 0; i < nodes.size() - 1; i++)
	{
		for (int j = i + 1; j < nodes.size(); j++)
		{
			float d_ij = D->get( i, j );
			glm::vec3 X_i = nodes[i]->GetNewPosition();
			glm::vec3 X_j = nodes[j]->GetNewPosition();
			float projected = glm::length( X_i - X_j );

			d_ij = ofMap( d_ij, d_ij_min, d_ij_max, 1, 2 );
			projected = ofMap( projected, projected_min, projected_max, 1, 2 );

			projected -= d_ij;
			sum += 1 / (d_ij * d_ij) * (projected * projected);
		}	
	}
	return sum;
}

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

	float delta_n_min = MAX_FLOAT;
	float delta_n_max = -MAX_FLOAT;
	float delta_q_min = MAX_FLOAT;
	float delta_q_max = -MAX_FLOAT;
	for (int i = 0; i < nodes.size(); i++)
	{
		for (int j = 0; j < nodes.size(); j++)
		{
			if (i == j) continue;
			float delta_n = D->get( i, j );
			float delta_q = glm::length( nodes[i]->GetNewPosition() - nodes[j]->GetNewPosition());
			delta_n_min = min( delta_n_min, delta_n );
			delta_n_max = max( delta_n_max, delta_n );
			delta_q_min = min( delta_q_min, delta_q );
			delta_q_max = max( delta_q_max, delta_q );
		}
	}

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
			float delta_q = glm::length(x_i->GetNewPosition() - x_j->GetNewPosition());
			delta_n = ofMap( delta_n, delta_n_min, delta_n_max, 0, 1 );
			delta_q = ofMap( delta_q, delta_q_min, delta_q_max, 0, 1 );
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
			float delta_p = glm::length(nodes[i]->GetNewPosition() - nodes[j]->GetNewPosition());
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
			float dist = glm::length(nodes[i]->GetNewPosition() - nodes[j]->GetNewPosition());
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
			float dist = glm::length(nodes[i]->GetNewPosition() - nodes[j]->GetNewPosition());
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
}