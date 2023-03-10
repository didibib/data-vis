#include "precomp.h"

namespace DataVis
{
int IStructure::__idx = 0;
//--------------------------------------------------------------
// IStructure
//--------------------------------------------------------------
IStructure::IStructure() {
	m_idx = __idx++;
}

IStructure::~IStructure() {
	__idx--;
}

const int& IStructure::Idx() const {
	return m_idx;
}

Dataset& IStructure::GetDataset() const
{
	return *m_dataset;
}

//--------------------------------------------------------------
// Position
//--------------------------------------------------------------
glm::vec3 IStructure::GetPosition() const {
	return m_position; 
};

void IStructure::SetPosition(glm::vec3 _new_position) { 
	m_position = _new_position; 
};

//--------------------------------------------------------------
// Displacement
//--------------------------------------------------------------
const glm::vec3& IStructure::Node::GetDisplacement()
{
	return m_displacement;
}

void IStructure::Node::SetDisplacement(glm::vec3 _displacement)
{
	m_displacement = _displacement;
}

//--------------------------------------------------------------
// Move
//--------------------------------------------------------------
void IStructure::Move(glm::vec3 _offset) { 
	m_position += _offset; 
};

//--------------------------------------------------------------
// Bounding Box
//--------------------------------------------------------------
const ofRectangle& IStructure::GetAABB() const {
	return m_aabb; 
};

const ofRectangle& IStructure::GetMoveAABB() const {
	return m_move_aabb; 
};

void IStructure::SetAABB()
{
	glm::vec3 tl{ 1e30 };
	glm::vec3 br{ -1e30 };

	for (const auto& node : GetNodes())
	{
		tl.x = min(node->GetPosition().x - node->GetRadius(), tl.x);
		tl.y = min(node->GetPosition().y - node->GetRadius(), tl.y);
		br.x = max(node->GetPosition().x + node->GetRadius(), br.x);
		br.y = max(node->GetPosition().y + node->GetRadius(), br.y);
	}
	tl.z = 0;
	br.z = 0;
	m_aabb = { tl, br };
}

void IStructure::UpdateAABB() { 
	SetAABB(); 
	SetMoveAABB(); 
};

void IStructure::SetMoveAABB()
{
	auto bb_tl = m_aabb.getTopLeft();
	m_move_aabb = { bb_tl, { bb_tl.x - m_move_aabb_size, bb_tl.y - m_move_aabb_size } };
}

bool IStructure::InsideAABB(glm::vec3 _position)
{
	return m_aabb.inside(_position - m_position);
}

bool IStructure::InsideMoveAABB(glm::vec3 _position)
{
	return m_move_aabb.inside(_position - m_position);
}

//--------------------------------------------------------------
// Rendering
//--------------------------------------------------------------
void IStructure::Draw(bool _is_focussed)
{
	ofPushMatrix();
	ofTranslate(m_position);

	// Draw the bounds
	ofNoFill();
	if (_is_focussed)
		ofSetColor(ofColor::green);
	else
		ofSetColor(ofColor::black);
	ofDrawRectangle(m_aabb);

	ofFill();
	ofSetColor(ofColor::black);
	ofDrawRectangle(m_move_aabb);
	ofNoFill();

	// Draw the actual nodes and edges
	DrawLayout();

	ofPopMatrix();
}


} // DataVis