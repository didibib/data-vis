#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// Animation
//--------------------------------------------------------------
void EdgePath::OnStopAnimation()
{
}

//--------------------------------------------------------------
void EdgePath::Interpolate(float _percentage)
{
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
EdgePath::EdgePath(bool is_directed)
{
    m_is_directed = is_directed;
    m_edge_idx = -1;
}

void EdgePath::SetStyle(const Style& _style)
{
    if (m_style == _style) return;
    m_style = _style;
    m_update_path = true;
}

//--------------------------------------------------------------
void EdgePath::SetArrowOffset(float _offset)
{
    m_arrow_offset = _offset;
}

//--------------------------------------------------------------
void EdgePath::SetEdgeIdx(EdgeIdx _edge_idx)
{
    m_edge_idx = _edge_idx;
}

//--------------------------------------------------------------
// Path
//--------------------------------------------------------------
void EdgePath::UpdatePath()
{
    if (not m_update_path) return;
    m_update_path = false;
    switch (m_style)
    {
    case Style::Line: BuildLinePath();
        break;
    case Style::Curve: BuildCurvePath();
        break;
    }
    m_path.translate(glm::vec3(0, 0, -1));
    UpdateArrow();
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
void EdgePath::BuildLinePath()
{
    m_path.clear();
    for (const auto& point : m_points)
        m_path.lineTo(point);
}

//--------------------------------------------------------------
void EdgePath::BuildCurvePath()
{
    m_path.clear();
    // https://github.com/openframeworks/openFrameworks/issues/6869
    m_path.addVertex(m_points[0]);
    if (m_start_ctrl_point_set)
        m_path.curveTo(m_start_ctrl_point);
    // else compute start ctrl point

    for (const auto& point : m_points)
        m_path.curveTo(point);

    if (m_end_ctrl_point_set)
        m_path.curveTo(m_end_ctrl_point);
    // else compute end ctrl point

}

void EdgePath::SetIsDirected(bool _directed)
{
    m_is_directed = _directed;
}

//--------------------------------------------------------------
void EdgePath::Clear()
{
    m_points.clear();
    m_path.clear();
}

//--------------------------------------------------------------
void EdgePath::AddPoint(const glm::vec3& _point)
{
    m_points.push_back(_point);
    m_update_path = true;
}

//--------------------------------------------------------------
void EdgePath::SetStartCtrlPoint(const glm::vec3& _point)
{
    m_start_ctrl_point = _point;
    m_start_ctrl_point_set = true;
    m_update_path = true;
}

//--------------------------------------------------------------
void EdgePath::SetEndCtrlPoint(const glm::vec3& _point)
{
    m_end_ctrl_point = _point;
    m_end_ctrl_point_set = true;
    m_update_path = true;
}

//--------------------------------------------------------------
void EdgePath::UpdateStartPoint(const glm::vec3& _point)
{
    m_points[0] = _point;
}

//--------------------------------------------------------------
void EdgePath::UpdateEndPoint(const glm::vec3& _point)
{
    m_points.back() = _point;
}

//--------------------------------------------------------------
void EdgePath::UpdatePoint(const size_t _index, const glm::vec3& _point)
{
    if (_index >= m_path.size()) return;
    m_points[_index] = _point;
    m_path[_index] = _point;
    m_update_path = true;
}
}
