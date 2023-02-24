#include "precomp.h"

namespace DataVis
{
const char* Layout::RANDOM = "Random";
const char* Layout::GRID = "Grid";
const char* Layout::RADIAL = "Radial";

//--------------------------------------------------------------
std::unordered_map<std::string, std::string> Layout::InitLayoutDescriptions( )
{
	std::unordered_map<std::string, std::string> layout_descriptions;
	std::vector<Data*> data = {
		new RandomData( ),
		new GridData( ),
		new RadialData( )
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

std::unordered_map<std::string, std::string>& Layout::LayoutDescriptions( )
{
	static std::unordered_map<std::string, std::string> layout_descriptions = Layout::InitLayoutDescriptions( );
	return layout_descriptions;
}

const std::vector <std::pair<std::string, std::function<void( Graph&, std::string )>>>& Layout::GraphLayoutFunctions( )
{
	static std::vector <std::pair<std::string, std::function<void( Graph&, std::string )>>> layout_functions = {
		{ RANDOM, Layout::RandomCmdline },
		{ GRID, Layout::GridCmdline }
	};
	return layout_functions;
}
const std::vector <std::pair<std::string, std::function<void( Tree&, std::string )>>>& Layout::TreeLayoutFunctions( )
{
	static std::vector <std::pair<std::string, std::function<void( Tree&, std::string )>>> layout_functions = {
		{ RADIAL, Layout::RadialCmdline }
	};
	return layout_functions;
}

//--------------------------------------------------------------
void Layout::RandomCmdline( Graph& _graph, std::string _cmdline_input )
{
	static RandomData rd;
	static auto options = rd.Options( );
	ParseCmdline( options, _cmdline_input );
	Random( _graph, rd.width, rd.height );
}

void Layout::Random( Graph& _graph, int _width, int _height )
{
	auto& nodes = _graph.Vertices( );
	for ( size_t i = 0; i < nodes.size( ); i++ ) {
		auto& current_position = nodes[i].m_property.current_position;
		float x = Random::RangeF( _width );
		float y = Random::RangeF( _height );
		float z = 0;
		current_position = glm::vec3( x, y, z );
	}
}

//--------------------------------------------------------------
void Layout::GridCmdline( Graph& _graph, std::string _cmdline_input )
{
	static GridData gd;
	static auto options = gd.Options( );
	ParseCmdline( options, _cmdline_input );
	Grid( _graph, gd.width, gd.height, gd.step );
}

void Layout::Grid( Graph& _graph, int _width, int _height, float _step )
{
	auto& nodes = _graph.Vertices( );
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
		auto& current_position = nodes[i].m_property.current_position;
		current_position = grid[i];
	}
}

//--------------------------------------------------------------
void Layout::RadialCmdline( Tree& _tree, std::string _cmdline_input )
{
	static RadialData rd;
	static auto options = rd.Options( );
	ParseCmdline( options, _cmdline_input );
	Radial( _tree, rd.step, rd.delta_angle );
}

void Layout::Radial( Tree& _tree, float _step, float _delta_angle )
{
	auto& node = _tree.Root( );
	node->new_position.x = 0; node->new_position.y = 0;
	RadialSubTree( *node, 0, TWO_PI, 0, _step, _delta_angle );
}

void Layout::RadialSubTree( Tree::Node& _node, float _wedge_start, float _wedge_end, int _depth, float _step, float _delta_angle )
{
	float new_wedge_start = _wedge_start;
	float radius = _step + ( _delta_angle * _depth );
	float parent_leaves = Tree::Leaves( std::make_shared<Tree::Node>( _node ) );
	for ( auto& child : _node.children ) {
		float child_leaves = Tree::Leaves( child );

		float new_wedge_end = new_wedge_start + ( child_leaves / parent_leaves * ( _wedge_end - _wedge_start ) );
		float angle = ( new_wedge_start + new_wedge_end ) * .5f;
		child->new_position.x = radius * glm::cos( angle );
		child->new_position.y = radius * glm::sin( angle );

		if ( child->children.size( ) > 0 )
			RadialSubTree( *child, new_wedge_start, new_wedge_end, _depth + 1, _step, _delta_angle );
		new_wedge_start = new_wedge_end;
	}
}
} // DataVis