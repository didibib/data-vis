#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// Node
//--------------------------------------------------------------
IStructure::Node::Node(std::string _vertex_id, VertexIdx _vertex_index, glm::vec3 _position)
{
	m_vertex_id = _vertex_id;
	m_vertex_idx = _vertex_index;
	m_position = _position;
	m_old_position = _position;
	m_aabb = ofRectangle(_position - glm::vec2(m_radius), m_radius * 2, m_radius * 2);
}

//--------------------------------------------------------------
// Animation
//--------------------------------------------------------------
void IStructure::Node::OnStopAnimation()
{
	SetPosition(m_new_position);
}

void IStructure::Node::Interpolate(float _p)
{
	SetPosition((1 - _p) * m_old_position + _p * m_new_position);
}

//--------------------------------------------------------------
// Identification
//--------------------------------------------------------------
const std::string& IStructure::Node::GetVertexId() const
{
	return m_vertex_id;
}

VertexIdx IStructure::Node::GetVertexIdx() const
{
	return m_vertex_idx;
}

//--------------------------------------------------------------
// Position
//--------------------------------------------------------------
const glm::vec3& IStructure::Node::GetPosition()
{
	return m_position;
}

void IStructure::Node::SetPosition(glm::vec3& _new_position)
{
	m_position = _new_position;
	m_old_position = _new_position;
}

const glm::vec3& IStructure::Node::GetNewPosition()
{
	return m_new_position;
}

void IStructure::Node::SetNewPosition(glm::vec3& _new_position)
{
	m_new_position = _new_position;
	m_aabb.setPosition(_new_position - glm::vec2(m_radius));
	StartAnimation();
}

//--------------------------------------------------------------
// Radius
//--------------------------------------------------------------
const float& IStructure::Node::GetRadius()
{
	return m_radius;
}

void IStructure::Node::SetRadius(float _radius)
{
	m_radius = _radius;
	m_aabb.setSize(_radius * 2, _radius * 2);
}

//--------------------------------------------------------------
// Interaction
//--------------------------------------------------------------
bool IStructure::Node::Inside(glm::vec3 _position)
{
	return glm::length(m_position - _position) < m_radius;
}

} // namespace DataVis