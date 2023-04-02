#include "precomp.h"

namespace DataVis
{
bool EdgeBundlingLayout::Gui(IStructure& _structure)
{
	bool active = false;
	if (ImGui::TreeNode("Edge Bundling Layout"))
	{
		ImGui::InputInt("Cycles", &m_C);
		ImGui::InputInt("Iterations", &m_l);
		ImGui::InputFloat("Stiffnes", &m_K);
		ImGui::InputInt("Initial Subdivisions", &m_n);
		ImGui::InputFloat("Step Size", &m_s);
		ImGui::InputFloat( "Compatibility Threshold", &m_comp_threshold );
		ImGui::Checkbox("Quadratic?", &m_quadratic);
		ImGui::Checkbox("Check Owners?", &m_check_owners);

		if (ImGui::Button("Apply"))
		{
			try
			{
				auto& clusters = dynamic_cast<Clusters&>(_structure);
				for (auto& graph : clusters.sub_graphs)
				{
					ForceDirectedLayout::Apply(*graph, 0.5f, 0.002f, 500);
					graph->UpdateEdges(true);
				}
				_structure.UpdateEdges(true);
			}
			catch (std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}

			Apply(_structure, m_C, m_l, m_K, m_n, m_s, m_comp_threshold, m_quadratic, m_check_owners, Compatibility);
			for (const auto& edge : _structure.edges)
				edge->ForceUpdate();
			active = true;
		}

		ImGui::TreePop();
		ImGui::Separator();
	}
	return active;
}

void EdgeBundlingLayout::Apply(IStructure& _structure, int _C, int _l, float _K, int _n, float _s, float _threshold, bool _quadratic, bool _check_owners, CompatibilityFunction _f)
{
	const auto& vertices = _structure.dataset->vertices;
	const auto& edges = _structure.dataset->edges;

	std::unordered_map<EdgeIdx, std::unordered_map<EdgeIdx, float>> edge_compatibility;
	for(const auto& p : _structure.edges)
	{
		for(const auto& q : _structure.edges)
		{
			if(p->GetEdgeIdx() == q->GetEdgeIdx()) continue;
			if(_check_owners)
			{
				//Check if these edges connect to the same owners
				const auto& p_owner_0 = vertices[edges[p->GetEdgeIdx()].from_idx]->owner;
				const auto& p_owner_1 = vertices[edges[p->GetEdgeIdx()].to_idx]->owner;
				const auto& q_owner_0 = vertices[edges[q->GetEdgeIdx()].from_idx]->owner;
				const auto& q_owner_1 = vertices[edges[q->GetEdgeIdx()].to_idx]->owner;

				if (not (p_owner_0 == q_owner_0 && p_owner_1 == q_owner_1) &&
					not (p_owner_0 == q_owner_1 && p_owner_1 == q_owner_0))
						continue;
			}
			edge_compatibility[p->GetEdgeIdx()][q->GetEdgeIdx()] = _f(*p, *q);
		}
	}
	
	// For c cycles
	for (int c = 0; c < _C; c++)
	{
		for (auto& edge : _structure.edges)
			edge->Subdivide(_n);

		// For l iterations calculate forces
		for (int l = 0; l < _l; l++)
		{
			std::cout << "Cycle: " << c << ", Iteration: " << l << std::endl;
			for (auto& p : _structure.edges)
			{
				const glm::vec3 edge = p->points.back().value - p->points.front().value;
				const float k_p = (_K / glm::length(edge)) * (p->points.size() - 1);
				const auto& p_edge_compatibility = edge_compatibility[p->GetEdgeIdx()];
				// For each point in edge P
				for (int i = 1; i < p->points.size() - 1; i++)
				{
					auto& P = p->points[i];
					// Spring force of previous segment
					const auto k_prev = p->points[i - 1] - P;
					// Spring force of next segment
					const auto k_next = p->points[i + 1] - P;
					//
					glm::vec3 sum_force = k_p * (k_prev.value + k_next.value);

					for (auto& q : _structure.edges)
					{
						// if (q->GetEdgeIdx() == p->GetEdgeIdx()) continue;
						//
						// if(_check_owners)
						// {
						// 	//Check if these edges connect to the same owners
						// 	const auto& p_owner_0 = vertices[edges[p->GetEdgeIdx()].from_idx]->owner;
						// 	const auto& p_owner_1 = vertices[edges[p->GetEdgeIdx()].to_idx]->owner;
						// 	const auto& q_owner_0 = vertices[edges[q->GetEdgeIdx()].from_idx]->owner;
						// 	const auto& q_owner_1 = vertices[edges[q->GetEdgeIdx()].to_idx]->owner;
						//
						// 	if (not (p_owner_0 == q_owner_0 && p_owner_1 == q_owner_1) &&
						// 		not (p_owner_0 == q_owner_1 && p_owner_1 == q_owner_0))
						// 		continue;
						// }
						//
						// float compatibility = edge_compatibility[p->GetEdgeIdx()][q->GetEdgeIdx()];

						auto it = p_edge_compatibility.find(q->GetEdgeIdx());
						if(it == p_edge_compatibility.end()) continue;

						float compatibility = it->second;
						if (compatibility < _threshold) continue;
						
						if (_quadratic)
						{
							for (int j = 1; j < q->points.size() - 1; j++)
							{
								glm::vec3 v = q->points[j].value - P.value;
								sum_force += compatibility * v;
							}
						}
						else 
						{
							glm::vec3 v = q->points[i].value - P.value;
							sum_force += compatibility * v;
						}
					}
					P.new_value = sum_force;
				}
			}
		}

		// Apply calculated forces
		for (auto& p : _structure.edges)
		{
			for (int i = 1; i < p->points.size() - 1; i++)
			{
				p->points[i].value += _s * p->points[i].new_value;
				p->points[i].new_value = p->points[i].value;
			}				
		}

		// Update parameters for next cycle
		_s *= .5f;
		_l = _l * 2 / 3;
	}
}

//--------------------------------------------------------------
// Compatibility
//--------------------------------------------------------------
float EdgeBundlingLayout::Compatibility(const EdgePath& _e1, const EdgePath& _e2)
{
	return AngleCompatibility(_e1, _e2) * ScaleCompatibility(_e1, _e2) * DistanceCompatibility(_e1, _e2) * VisibilityCompatibility(_e1, _e2);
}

//--------------------------------------------------------------
float EdgeBundlingLayout::AngleCompatibility(const EdgePath& _e1, const EdgePath& _e2)
{
	const glm::vec3 P = _e1.points.back().value - _e1.points.front().value;
	const glm::vec3 Q = _e2.points.back().value - _e2.points.front().value;
	const float cos = glm::dot(P, Q) / (glm::length(P) * glm::length(Q));
	return glm::abs(cos);
}

//--------------------------------------------------------------
float EdgeBundlingLayout::ScaleCompatibility(const EdgePath& _e1, const EdgePath& _e2)
{
	glm::vec3 P = _e1.points.back().value - _e1.points.front().value;
	glm::vec3 Q = _e2.points.back().value - _e2.points.front().value;
	float length_P = glm::length(P);
	float length_Q = glm::length(Q);
	P /= glm::min(length_P, length_Q);
	Q /= glm::min(length_P, length_Q);
	length_P = glm::length(P);
	length_Q = glm::length(Q);
	const float length_avg = (length_P + length_Q) * 0.5f;
	const float denom = length_avg * glm::min(length_P, length_Q) + (glm::max(length_P, length_Q) / length_avg);
	return 2 / denom;
}

//--------------------------------------------------------------
float EdgeBundlingLayout::DistanceCompatibility(const EdgePath& _e1, const EdgePath& _e2)
{
	const glm::vec3 P = _e1.points.back().value - _e1.points.front().value;
	const glm::vec3 Q = _e2.points.back().value - _e2.points.front().value;
	const float length_P = glm::length(P);
	const float length_Q = glm::length(Q);
	const float length_avg = (length_P + length_Q) * 0.5f;
	const glm::vec3 P_m = _e1.points.front().value + P * 0.5f;
	const glm::vec3 Q_m = _e2.points.front().value + Q * 0.5f;
	return length_avg / (length_avg + length(P_m - Q_m));
}

//--------------------------------------------------------------
float EdgeBundlingLayout::VisibilityCompatibility(const EdgePath& _e1, const EdgePath& _e2)
{
	return glm::min(Visibility(_e1, _e2), Visibility(_e2, _e1));
}

//--------------------------------------------------------------
float EdgeBundlingLayout::Visibility(const EdgePath& _e1, const EdgePath& _e2)
{
	const glm::vec2 P0 = _e1.points.front().value;
	const glm::vec2 P1 = _e1.points.back().value;
	const glm::vec2 r = P1 - P0;

	const glm::vec2 Q0 = _e2.points.front().value;
	const glm::vec2 Q1 = _e2.points.back().value;
	const glm::vec2 Qdir = Q1 - Q0;
	// Normal of _e2
	const glm::vec2 s = glm::vec2(-Qdir.y, Qdir.x);

	// Intersections
	const auto t0 = Math::Cross(Q0 - P0, s) / Math::Cross(r, s);
	const auto t1 = Math::Cross(Q1 - P0, s) / Math::Cross(r, s);
	const auto i0 = P0 + t0 * r;
	const auto i1 = P0 + t1 * r;

	auto i_m = (i0 + i1) * 0.5f;
	auto p_m = (P0 + P1) * 0.5f;

	return glm::max(1 - (2 * glm::length(p_m - i_m)) / (glm::length(i0 - i1)), 0.f);
}
}

