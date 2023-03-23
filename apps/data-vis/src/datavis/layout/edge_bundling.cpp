#include "precomp.h"

namespace DataVis
{
bool EdgeBundlingLayout::Gui(IStructure&)
{
	return false;
}

void EdgeBundlingLayout::BundleEdges(IStructure& _structure, float _K, int _C, int _l, int _n, float _s, InteractionFunction _f)
{
	for (int c = 0; c < _C; c++)
	{
		for (auto& edge : _structure.edges)
			edge->Subdivide(_n);

		// For l iterations perform:
		for (int l = 0; l < _l; l++)
		{
			for (auto& p : _structure.edges)
			{
				const glm::vec3 P = p->points.back().value - p->points.front().value;
				const float k_p = (_K / glm::length(P)) * (p->points.size() - 1);

				for (int i = 1; i < p->points.size() - 1; i++)
				{
					// spring force of previous segment
					const auto k_prev = p->points[i - 1] - p->points[i];
					// spring force of next segment
					const auto k_next = p->points[i] - p->points[i + 1];
					const float force_p_i = k_p * (length(k_prev.value) + length(k_next.value));
				}
			}
		}

		_s /= 2.f;
		// Divide by int
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
	float cos = glm::dot(P, Q) / (glm::length(P) * glm::length(Q));
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
	const glm::vec2 s = glm::vec2(-Qdir.y, Qdir.x);

	// Intersections
	auto t0 = cross(Q0 - P0, s) / cross(r, s);
	auto t1 = cross(Q1 - P0, s) / cross(r, s);
	auto i0 = P0 + t0 * r;
	auto i1 = P0 + t1 * r;
    
    auto i_m = (i0 + i1) * 0.5f;
    auto p_m = (p0 + p1) * 0.5f;

	return glm::max(1 - (2 * glm::length(p_m - i_m)) / (glm::length(i0 - i1)), 0);
}
}

