#include "precomp.h"

namespace DataVis
{
//--------------------------------------------------------------
// AABB
//--------------------------------------------------------------
void AABB::Draw()
{
	ofNoFill();
	if (m_is_focussed)
		ofSetColor(ofColor::green);
	else
		ofSetColor(ofColor::lightGray);
	ofDrawRectangle(m_bounds);

	ofFill();
	ofSetColor(ofColor::black);
	ofDrawRectangle(m_draggable);
	ofNoFill();
}
	
void AABB::Draw(bool _is_focussed)
{
	m_is_focussed = _is_focussed;
	Draw();
}

//--------------------------------------------------------------
float AABB::GetArea() const
{
	return m_bounds.getArea();
}

//--------------------------------------------------------------
bool AABB::Inside(const glm::vec3& _position) const
{
	return m_bounds.inside(_position);
}

//--------------------------------------------------------------
bool AABB::InsideDraggable(const glm::vec3& _position) const
{
	return m_draggable.inside(_position);
}

//--------------------------------------------------------------
void AABB::SetNewBounds(const glm::vec3& _top_left, const glm::vec3& _bottom_right)
{
	// Update bounds
	m_new_top_left = _top_left;
	m_new_bottom_right = _bottom_right;
	StartAnimation();
}

//--------------------------------------------------------------
void AABB::OnStopAnimation()
{
	m_old_top_left = m_new_top_left;
	m_old_bottom_right = m_new_bottom_right;
}

void AABB::Interpolate(float _p)
{
	const glm::vec3 top_left = (1 - _p) * m_old_top_left + _p * m_new_top_left;
	const glm::vec3 bottom_right = (1 - _p) * m_old_bottom_right + _p * m_new_bottom_right;
	m_bounds.set(top_left, bottom_right);

	// Update draggable
	auto bb_tl = m_bounds.getTopLeft();
	m_draggable = { bb_tl, { bb_tl.x - static_cast<float>(m_draggable_size), bb_tl.y - static_cast<float>(m_draggable_size) } };
}

glm::vec3 AABB::Clamp( const glm::vec3& _position ) const
{
	const glm::vec3 out = max( _position, m_bounds.getTopLeft() );
	return min( out, m_bounds.getBottomRight() );
}

} // namespace DataVis