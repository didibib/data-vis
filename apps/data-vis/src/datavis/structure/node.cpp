#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// Node
//--------------------------------------------------------------
Node::Node(std::string _vertex_id, VertexIdx _vertex_index, const glm::vec3& _position)
{
    m_vertex_id = std::move(_vertex_id);
    m_vertex_idx = _vertex_index;
    m_position.value = _position;
    m_position.old_value = _position;
    m_aabb = ofRectangle(_position - glm::vec2(m_radius), m_radius * 2, m_radius * 2);
}

void Node::Draw()
{
    ofFill();
    ofSetColor(color);
    ofDrawCircle(m_position.value, m_radius);
}

//--------------------------------------------------------------
// Displacement
//--------------------------------------------------------------
const glm::vec3& Node::GetDisplacement()
{
    return m_displacement;
}

void Node::SetDisplacement(const glm::vec3& _displacement)
{
    m_displacement = _displacement;
}

//--------------------------------------------------------------
// Animation
//--------------------------------------------------------------
void Node::OnStopAnimation()
{
    SetPosition(m_position.new_value);
}

void Node::Interpolate(float _p)
{
    m_position.value = (1 - _p) * m_position.old_value + _p * m_position.new_value;
}

//--------------------------------------------------------------
// Identification
//--------------------------------------------------------------
const std::string& Node::GetVertexId() const
{
    return m_vertex_id;
}

VertexIdx Node::GetVertexIdx() const
{
    return m_vertex_idx;
}

//--------------------------------------------------------------
// Position
//--------------------------------------------------------------
const glm::vec3& Node::GetPosition()
{
    return m_position.value;
}

void Node::SetPosition(const glm::vec3& _new_position)
{
    m_position.value = _new_position;
    m_position.old_value = _new_position;
    m_position.new_value = _new_position;
}

const glm::vec3& Node::GetNewPosition()
{
    return m_position.new_value;
}

void Node::SetNewPosition(const glm::vec3& _new_position)
{
    m_position.new_value = _new_position;
    m_aabb.setPosition(_new_position - glm::vec2(m_radius));
    StartAnimation();
}

//--------------------------------------------------------------
// Radius
//--------------------------------------------------------------
const float& Node::GetRadius() const
{
    return m_radius;
}

void Node::SetRadius(float _radius)
{
    m_radius = _radius;
    m_aabb.setSize(_radius * 2, _radius * 2);
}

//--------------------------------------------------------------
// Interaction
//--------------------------------------------------------------
bool Node::Inside(const glm::vec3& _position) const
{
    return glm::length(m_position.value - _position) < m_radius;
}
} // namespace DataVis
