#include "precomp.h"

namespace DataVis
{
const char* IStructure::__RANDOM = "Random";
const char* IStructure::__GRID = "Grid";
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

glm::vec3 IStructure::GetPosition() const {
	return m_position; 
};

void IStructure::SetPosition(glm::vec3 _new_position) { 
	m_position = _new_position; 
};

void IStructure::Move(glm::vec3 _offset) { 
	m_position += _offset; 
};

const ofRectangle& IStructure::GetAABB() const {
	return m_aabb; 
};

const ofRectangle& IStructure::GetMoveAABB() const {
	return m_move_aabb; 
};

void IStructure::UpdateAABB() { 
	SetAABB(); 
	SetMoveAABB(); 
};

void IStructure::SetMoveAABB()
{
	auto bb_tl = m_aabb.getTopLeft();
	m_move_aabb = { bb_tl, { bb_tl.x - m_move_aabb_size, bb_tl.y - m_move_aabb_size } };
	//m_move_bounds.clear();
	//m_move_bounds.addVertices( {
	//	{ bb_tl },
	//	{ bb_tl.x - m_move_bounds_size, bb_tl.y, 0},
	//	{ bb_tl.x - m_move_bounds_size, bb_tl.y - m_move_bounds_size, 0},
	//	{ bb_tl.x, bb_tl.y - m_move_bounds_size, 0},
	//	{ bb_tl }
	//});
}

void IStructure::Draw()
{
	ofPushMatrix();
	ofTranslate(m_position);

	// Draw the bounds
	ofNoFill();
	ofSetColor(ofColor::black);
	ofDrawRectangle(m_aabb);

	ofFill();
	ofDrawRectangle(m_move_aabb);
	ofNoFill();

	// Draw the actual nodes and edges
	DrawLayout();

	ofPopMatrix();
}

//--------------------------------------------------------------
// Node
//--------------------------------------------------------------
IStructure::Node::Node(std::string _vertex_id, VertexIdx _vertex_index, glm::vec3 _position)
{
	m_vertex_id = _vertex_id;
	m_vertex_idx = _vertex_index;
	m_position = _position;
	m_old_position = _position;
	m_bounding_box = ofRectangle(_position - glm::vec2(m_radius), m_radius * 2, m_radius * 2);
}

void IStructure::Node::EaseInEaseOut(float _t, float _speed)
{
	if (m_time > 1) {
		m_animate = false;
		m_time = 0;
	}
	if (m_animate) {
		m_time += _t * _speed;
		float p = Curves::Bezier(m_time);
		SetPosition((1 - p) * m_old_position + p * m_new_position);
		if (p >= .999f) {
			m_animate = false;
			m_time = 0;
			SetPosition(m_new_position);
			m_old_position = m_new_position;
		}
	}
}

const std::string& IStructure::Node::GetVertexId() const
{
	return m_vertex_id;
}

VertexIdx IStructure::Node::GetVertexIdx() const
{
	return m_vertex_idx;
}

const glm::vec3& IStructure::Node::GetPosition()
{
	return m_position;
}

void IStructure::Node::SetPosition(glm::vec3& _position)
{
	m_position = _position;
	m_bounding_box.setPosition(_position - glm::vec2(m_radius));
}

void IStructure::Node::SetNewPosition(glm::vec3& _new_position)
{
	m_new_position = _new_position;
	m_animate = true;
}

const float& IStructure::Node::GetRadius()
{
	return m_radius;
}

void IStructure::Node::SetRadius(float _radius)
{
	m_radius = _radius;
	m_bounding_box.setSize(_radius * 2, _radius * 2);
}

bool IStructure::Node::Inside(glm::vec3 _position)
{
	return glm::length(m_position - _position) < m_radius;
}

//--------------------------------------------------------------
// Layout
//--------------------------------------------------------------
std::unordered_map<std::string, std::string> IStructure::InitLayoutDescriptions( )
{
	std::unordered_map<std::string, std::string> layout_descriptions;
	std::vector<Data*> data = {
		new RandomData( ),
		new GridData( )
	};
	for (const auto& d : data ) {
		auto& desc = d->Options( );
		std::stringstream ss;
		desc.print( ss );
		layout_descriptions.insert( { desc.caption( ), ss.str( ) } );
	}
	// Free memory
	for ( auto& d : data ) delete d;
	data.clear( );

	return layout_descriptions;
}

//--------------------------------------------------------------
std::unordered_map<std::string, std::string>& IStructure::LayoutDescriptions( )
{
	static std::unordered_map<std::string, std::string> layout_descriptions = IStructure::InitLayoutDescriptions( );
	return layout_descriptions;
}

//--------------------------------------------------------------
const std::vector <std::pair<std::string, std::function<void(IStructure&, std::string )>>>& IStructure::LayoutFunctions( )
{
	static std::vector <std::pair<std::string, std::function<void(IStructure&, std::string )>>> layout_functions = {
		{ __RANDOM, IStructure::RandomCmdline },
		{ __GRID, IStructure::GridCmdline }
	};
	return layout_functions;
}

//--------------------------------------------------------------
void IStructure::RandomCmdline(IStructure& _layout, std::string _cmdline_input )
{
	static RandomData rd;
	static auto options = rd.Options( );
	Parser::Cmdline( options, _cmdline_input );
	Random(_layout, rd.width, rd.height );
}

void IStructure::Random( IStructure& _layout, int _width, int _height )
{
	auto& nodes = _layout.GetNodes( );
	for ( size_t i = 0; i < nodes.size( ); i++ ) {
		float x = Random::RangeF( _width );
		float y = Random::RangeF( _height );
		float z = 0;
		nodes[i]->SetPosition(glm::vec3(x, y, z));
	}
}

//--------------------------------------------------------------
void IStructure::GridCmdline(IStructure& _layout, std::string _cmdline_input )
{
	static GridData gd;
	static auto options = gd.Options( );
	Parser::Cmdline( options, _cmdline_input );
	Grid(_layout, gd.width, gd.height, gd.step );
}

void IStructure::Grid( IStructure& _layout, int _width, int _height, float _step )
{
	auto& nodes = _layout.GetNodes( );
	std::vector<glm::vec3> grid;
	// Increment width and height if there are more nodes then positions
	while ( nodes.size( ) > _width * _height ) _width++, _height++;
	grid.reserve( _width * _height );
	// Generate positions
	for ( size_t j = 0; j < _height; j++ )
		for ( size_t i = 0; i < _width; i++ ) {
			float x = i * _step;
			float y = j * _step;
			float z = 0;
			grid.push_back( glm::vec3( x, y, z ) );
		}
	// Shuffle vector
	std::shuffle( std::begin( grid ), std::end( grid ), Random::MT19937 );
	// Assign positions
	for ( size_t i = 0; i < nodes.size( ); i++ ) {
		nodes[i]->SetPosition(grid[i]);
	}
}

void IStructure::SetAABB()
{
	glm::vec3 tl {1e30};
	glm::vec3 br{ -1e30 };

	for (const auto& node : GetNodes())
	{
		tl.x = min( node->GetPosition().x - node->GetRadius(), tl.x );
		tl.y = min( node->GetPosition().y - node->GetRadius(), tl.y );
		br.x = max( node->GetPosition().x + node->GetRadius(), br.x );
		br.y = max( node->GetPosition().y + node->GetRadius(), br.y );
	}
	tl.z = 0;
	br.z = 0;
	m_aabb = { tl, br };
}


} // DataVis