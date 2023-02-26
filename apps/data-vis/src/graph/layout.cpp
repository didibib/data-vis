#include "precomp.h"

namespace DataVis
{
const char* ILayout::__RANDOM = "Random";
const char* ILayout::__GRID = "Grid";
int ILayout::__idx = 0;

ILayout::ILayout() {
	m_idx = __idx++;
}

ILayout::~ILayout() {
	__idx--;
}

//--------------------------------------------------------------
std::unordered_map<std::string, std::string> ILayout::InitLayoutDescriptions( )
{
	std::unordered_map<std::string, std::string> layout_descriptions;
	std::vector<Data*> data = {
		new RandomData( ),
		new GridData( )
	};
	for ( auto& d : data ) {
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
std::unordered_map<std::string, std::string>& ILayout::LayoutDescriptions( )
{
	static std::unordered_map<std::string, std::string> layout_descriptions = ILayout::InitLayoutDescriptions( );
	return layout_descriptions;
}

//--------------------------------------------------------------
const std::vector <std::pair<std::string, std::function<void(ILayout&, std::string )>>>& ILayout::LayoutFunctions( )
{
	static std::vector <std::pair<std::string, std::function<void(ILayout&, std::string )>>> layout_functions = {
		{ __RANDOM, ILayout::RandomCmdline },
		{ __GRID, ILayout::GridCmdline }
	};
	return layout_functions;
}

//--------------------------------------------------------------
void ILayout::RandomCmdline(ILayout& _layout, std::string _cmdline_input )
{
	static RandomData rd;
	static auto options = rd.Options( );
	ParseCmdline( options, _cmdline_input );
	Random(_layout, rd.width, rd.height );
}

void ILayout::Random( ILayout& _layout, int _width, int _height )
{
	auto& nodes = _layout.Nodes( );
	for ( size_t i = 0; i < nodes.size( ); i++ ) {
		float x = Random::RangeF( _width );
		float y = Random::RangeF( _height );
		float z = 0;
		nodes[i].get().SetPosition(glm::vec3(x, y, z));
	}
}

//--------------------------------------------------------------
void ILayout::GridCmdline(ILayout& _layout, std::string _cmdline_input )
{
	static GridData gd;
	static auto options = gd.Options( );
	ParseCmdline( options, _cmdline_input );
	Grid(_layout, gd.width, gd.height, gd.step );
}

void ILayout::Grid( ILayout& _layout, int _width, int _height, float _step )
{
	auto& nodes = _layout.Nodes( );
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
		nodes[i].get().SetPosition(grid[i]);
	}
}

void ILayout::SetBounds()
{
	glm::vec3 tl {1e30};
	glm::vec3 br{ -1e30 };

	for (auto& wrapper : Nodes())
	{
		auto& node = wrapper.get();
		tl.x = min( node.GetPosition().x - node.GetRadius(), tl.x );
		tl.y = min( node.GetPosition().y - node.GetRadius(), tl.y );
		br.x = max( node.GetPosition().x + node.GetRadius(), br.x );
		br.y = max( node.GetPosition().y + node.GetRadius(), br.y );
	}
	tl.z = 0;
	br.z = 0;
	m_bounds = { tl, br };
}

void ILayout::SetMoveBounds()
{
	auto bb_tl = m_bounds.getTopLeft();
	m_move_bounds = { bb_tl, { bb_tl.x - m_move_bounds_size, bb_tl.y - m_move_bounds_size } };
	//m_move_bounds.clear();
	//m_move_bounds.addVertices( {
	//	{ bb_tl },
	//	{ bb_tl.x - m_move_bounds_size, bb_tl.y, 0},
	//	{ bb_tl.x - m_move_bounds_size, bb_tl.y - m_move_bounds_size, 0},
	//	{ bb_tl.x, bb_tl.y - m_move_bounds_size, 0},
	//	{ bb_tl }
	//});
}

void ILayout::Draw()
{
	ofPushMatrix();
	ofTranslate( m_position );

	// Draw the bounds
	ofNoFill();
	ofSetColor( ofColor::black );
	ofDrawRectangle( m_bounds );

	ofFill();
	ofDrawRectangle( m_move_bounds );
	ofNoFill();
	
	// Draw the actual nodes and edges
	DrawLayout();

	ofPopMatrix();
}
} // DataVis