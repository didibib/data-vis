#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// CurvedPath
//--------------------------------------------------------------
EdgePath::EdgePath(bool is_directed)
{
    m_is_directed = is_directed;
}

void EdgePath::Draw()
{
    if (m_update_arrow)
    {
        m_end_arrow = m_path[m_path.size() - 1];
        m_start_arrow = m_path[m_path.size() - 2];
        m_update_arrow = false;
    }

    ofNoFill();
    ofSetColor(123);
    m_path.draw();

    if (m_is_directed)
        ofDrawArrow(m_start_arrow, m_end_arrow, 10);
}

//--------------------------------------------------------------
void EdgePath::Init(EdgeIdx _edge_idx)
{
    m_edge_idx = _edge_idx;
}

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
// Path
//--------------------------------------------------------------
void EdgePath::SetIsDirected(bool _directed)
{
    m_is_directed = _directed;
}

//--------------------------------------------------------------
void EdgePath::AddPoint(const glm::vec3& _point)
{
    if (m_path.size() < 3)
        m_path.lineTo(_point);
    else
        m_path.curveTo(_point);
    m_update_arrow = true;
}

//--------------------------------------------------------------
void EdgePath::UpdatePoint(const size_t _index, const glm::vec3& _point)
{
    if (_index >= m_path.size()) return;
    m_path[_index] = _point;
    if (_index >= m_path.size()) m_update_arrow = true;
}
}
