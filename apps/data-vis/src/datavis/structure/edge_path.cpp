#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// Animation
//--------------------------------------------------------------
void EdgePath::OnStopAnimation()
{
	for (auto& point : points)
	{
		point.value = point.new_value;
		point.old_value = point.new_value;
	}
	m_start_ctrl_point.value = m_start_ctrl_point.new_value;
	m_start_ctrl_point.old_value = m_start_ctrl_point.new_value;
	m_end_ctrl_point.value = m_start_ctrl_point.new_value;
	m_end_ctrl_point.old_value = m_start_ctrl_point.new_value;
	BuildPath();
}

//--------------------------------------------------------------
void EdgePath::Interpolate(float _p)
{
	for (auto& point : points)
		point.value = (1 - _p) * point.old_value + _p * point.new_value;
	m_start_ctrl_point.value = (1 - _p) * m_start_ctrl_point.old_value + _p * m_start_ctrl_point.new_value;
	m_end_ctrl_point.value = (1 - _p) * m_end_ctrl_point.old_value + _p * m_end_ctrl_point.new_value;
	BuildPath();
}

//--------------------------------------------------------------
// Draw
//--------------------------------------------------------------
void EdgePath::Draw()
{
	UpdatePath();
	ofFill();
	m_path.draw();

	if (m_is_directed)
		ofDrawArrow(m_start_arrow, m_end_arrow, 12);
}

//--------------------------------------------------------------
// CurvedPath
//--------------------------------------------------------------
EdgePath::EdgePath(const Dataset::Kind& _kind)
{
	m_is_directed = false;
	if (_kind == Dataset::Kind::Directed)
		m_is_directed = true;
	m_edge_idx = -1;
}

void EdgePath::SetStyle(const Style& _style)
{
	if (m_style == _style) return;
	m_style = _style;
	m_update_path = true;
}

//--------------------------------------------------------------
// Setters
//--------------------------------------------------------------
void EdgePath::SetEdgeIdx(EdgeIdx _edge_idx)
{
	m_edge_idx = _edge_idx;
}

//--------------------------------------------------------------
void EdgePath::SetArrowOffset(float _offset)
{
	m_arrow_offset = _offset;
}

//--------------------------------------------------------------
void EdgePath::SetIsDirected(bool _directed)
{
	m_is_directed = _directed;
}

//--------------------------------------------------------------
// Path
//--------------------------------------------------------------
void EdgePath::UpdatePath()
{
	if (not m_update_path) return;
	m_update_path = false;
	StartAnimation();
}

//--------------------------------------------------------------
void EdgePath::UpdateArrow()
{
	if (not m_is_directed) return;
	// Ignore the last point, is used as control point
	float length = m_path.getLengthAtIndex(m_path.size() - 1);
	length -= m_arrow_offset;
	m_end_arrow = m_path.getPointAtLength(length);
	m_start_arrow = m_path.getPointAtLength(length - 1);
}

//--------------------------------------------------------------
// Build Paths
//--------------------------------------------------------------
void EdgePath::BuildPath()
{
	switch (m_style)
	{
	case Style::Line:
		BuildLinePath();
		break;
	case Style::Curve:
		BuildCurvePath();
		break;
	}
	m_path.translate(glm::vec3(0, 0, -.1f));
	UpdateArrow();
}

//--------------------------------------------------------------
// Line Path
//--------------------------------------------------------------
void EdgePath::BuildLinePath()
{
	m_path.clear();
	for (const auto& point : points)
		m_path.lineTo(point.value);
}

//--------------------------------------------------------------
// Curve Path
//--------------------------------------------------------------
void EdgePath::BuildCurvePath()
{
	m_path.clear();
	// https://github.com/openframeworks/openFrameworks/issues/6869
	m_path.addVertex(points[0].value);
	if (m_start_ctrl_point_set)
	{
		glm::vec3 dir = m_start_ctrl_point.value - points[0].value;
		dir = glm::normalize(dir);
		m_path.curveTo(points[0].value + dir);
	}
	else printf("No start control point set\n");
	// TODO: Calculate control start point

	for (const auto& point : points)
		m_path.curveTo(point.value);

	if (m_end_ctrl_point_set)
	{
		glm::vec3 dir = m_end_ctrl_point.value - points.back().value;
		dir = glm::normalize(dir);
		m_path.curveTo(points.back().value + dir);
	}
	else printf("No end control point set\n");
	// TODO: Calculate control end point
}

//--------------------------------------------------------------
void EdgePath::Clear()
{
	points.clear();
	m_path.clear();
}

//--------------------------------------------------------------
// Points
//--------------------------------------------------------------
void EdgePath::AddPoint(const glm::vec3& _point)
{
	points.emplace_back();
	points.back().new_value = _point;
	m_update_path = true;
}

//--------------------------------------------------------------
void EdgePath::SetStartCtrlPoint(const glm::vec3& _point)
{
	m_start_ctrl_point.new_value = _point;
	m_start_ctrl_point_set = true;
	m_update_path = true;
}

//--------------------------------------------------------------
void EdgePath::SetEndCtrlPoint(const glm::vec3& _point)
{
	m_end_ctrl_point.new_value = _point;
	m_end_ctrl_point_set = true;
	m_update_path = true;
}

//--------------------------------------------------------------
void EdgePath::UpdateStartPoint(const glm::vec3& _point)
{
	points[0].new_value = _point;
	m_update_path = true;
	// TODO: Animate all other points
}

//--------------------------------------------------------------
void EdgePath::UpdateEndPoint(const glm::vec3& _point)
{
	points.back().new_value = _point;
	m_update_path = true;
	// TODO: Animate all other points
}

//--------------------------------------------------------------
void EdgePath::UpdatePoint(const size_t _index, const glm::vec3& _point)
{
	if (_index >= m_path.size()) return;
	points[_index].new_value = _point;
	m_path[_index] = _point;
	m_update_path = true;
}

void EdgePath::ForceUpdate()
{
	StopAnimation();
}

//--------------------------------------------------------------
// Operations
//--------------------------------------------------------------
void EdgePath::Subdivide(uint _subdivisions)
{
    std::vector<InterpolateValue3> new_points;
    // |m_points| - 1 * _subdivisions new points created
	new_points.reserve(points.size() * _subdivisions);

    new_points.emplace_back(points[0].value);
	for (size_t i = 0; i < points.size() - 1; i++)
	{
		const auto& start = points[i];
		const auto& end = points[i+1];
        glm::vec3 dir = end.value - start.value;
        const float total_length = glm::length(dir);
        const float subdivision_length = total_length / static_cast<float>(_subdivisions);
        dir /= total_length;
        
        for(int j = 1; j <= _subdivisions; j++)
        {
            glm::vec3 new_point = start.value + subdivision_length * j * dir;
            new_points.emplace_back(new_point); 
        }
		new_points.emplace_back(end);
	}
	points = new_points;
}
}
