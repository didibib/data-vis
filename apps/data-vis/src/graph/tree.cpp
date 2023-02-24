#include "precomp.h"

namespace DataVis
{
Tree Tree::Extract::MSP( Graph& _graph, int _root )
{
	// Implement Prim's Algorithm
	// https://www.wikiwand.com/en/Prim%27s_algorithm
	auto& vertices = _graph.Vertices( );

	// Keep track of the parent of each vertex so we can construct a tree after
	std::vector<int> parents;
	// Keep track of which vertex are already processed
	std::vector<bool> included;
	// Keep track of the minimum edge cost of a vertex
	std::vector<float> costs;
	// Keep track of the actual minimum edge associated with the cost
	std::vector<Edge*> edges;
	parents.resize( vertices.size( ) );
	included.resize( vertices.size( ) );
	costs.resize( vertices.size( ) );
	// Initialize all but the root as infinite, so that the root is picked first
	// Make parent of root -1
	for ( int i = 0; i < vertices.size( ); i++ ) {
		if ( i != _root ) costs[i] = 1e30f;
		else parents[i] = -1;
	}

	// Graph will have |vertices| nodes
	for ( int i = 0; i < vertices.size( ); i++ ) {
		// Argmin for costs
		float min = 1e30;
		int idx;
		for ( int v = 0; v < vertices.size( ); v++ ) {
			if ( !included[v] && costs[v] < min ) {
				min = costs[v];
				idx = v;
			}
		}

		// Add lowest vertex
		included[idx] = true;

		// Update outgoing edges from this vertex
		auto& out_edges = vertices[idx].m_out_edges;
		for ( int j = 0; j < out_edges.size( ); j++ ) {
			int v = out_edges[j].m_target;
			if ( !included[v] && out_edges[j].get_property( ).weight < costs[v] ) {
				// Found a cheaper edge to v
				parents[v] = idx;
				costs[v] = out_edges[j].get_property( ).weight;
			}
		}
	}

	// Construct a tree
	Tree t;
	t.m_root = std::make_shared<Node>( Node{ _root, nullptr, {}, vertices[_root].m_property.current_position } );

	// Construct recursive lambda to create the tree
	std::function<void( std::shared_ptr<Node> )> make_tree = [&]( std::shared_ptr<Node> n ) {
		// Look through all vertices which have parent == n.vertex
		for ( int i = 0; i < vertices.size( ); i++ )
			if ( parents[i] == n->vertex ) {
				// Add node to n.children
				auto child = std::make_shared<Node>( Node{ i, n, {}, vertices[i].m_property.current_position } );
				n->children.push_back( child );
				make_tree( child );
			}
	};
	make_tree( t.m_root );

	// Construct recursive lambda to count all children in subtree and update node.subtree_count of each node in the tree
	//std::function<int( std::shared_ptr<Node> )> count_subtree;
	//count_subtree = [&]( std::shared_ptr<Node> n)
	//{
	//	int count = 1;
	//	for (auto child : n->children)
	//		count += count_subtree( child );
	//	n->subtree_count = count;
	//	return count;
	//};
	t.SetProperties();

	return t;
}

//int Tree::Extract::CountSubtree( std::shared_ptr<Node> n )
//{
//	int count = 1;
//	for ( auto child : n->children )
//		count += CountSubtree( child );
//	n->subtree_count = count;
//	return count;
//}

int Tree::Leaves( std::shared_ptr<Node> node )
{
	int leaves = 0;
	if ( node->children.size( ) == 0 ) leaves++;
	for ( auto& child : node->children ) leaves += Leaves( child );
	return leaves;
}

int Tree::Depth( std::shared_ptr<Node> node )
{
	int max = 0;
	for (auto& child : node->children)
	{
		int child_depth = Depth( child );
		if (child_depth > max) max = child_depth;
	}
	return max + 1;
}

std::shared_ptr<Tree::Node> Tree::Select( glm::vec3 _pos )
{
	// Loop through all nodes and find one within radius of pos
	std::function<std::shared_ptr<Node>( std::shared_ptr<Node> )> select_in_tree;
	select_in_tree = [&]( std::shared_ptr<Node> n ) {
		//printf( "Checking vertex %i\n", n->vertex );
		if ( glm::length( n->position - _pos ) < radius ) {
			//printf( "vertex pos: %f %f, mouse pos: %f %f\n", n->position.x, n->position.y, _pos.x, _pos.y );
			//printf( "FOUND AT VERTEX %i\n", n->vertex );
			return n;
		}
		//printf( "Looking through children\n" );
		for ( auto& child : n->children ) {
			//printf( "sub check child %i\n", child->vertex );
			std::shared_ptr<Node> selected = select_in_tree( child );
			if ( selected != nullptr ) return selected;
		}
		std::shared_ptr<Node> temp = nullptr;
		return temp;
	};
	return select_in_tree( m_root );
}

void Tree::SwapRoot( std::shared_ptr<Node> _new_root )
{
	std::shared_ptr<Node> node = _new_root;
	while (node != m_root)
	{
		node->children.push_back( node->parent );
		auto& parent_children = node->parent->children;
		for (int i = 0; i < parent_children.size(); i++)
		{
			if (parent_children[i] == node)
			{
				// replace node with final element
				parent_children[i] = parent_children[parent_children.size() - 1];
				parent_children.resize( parent_children.size() - 1 );
				break;
			}
		}
		// move to next node
		node = node->parent;
	}

	node = _new_root;
	std::shared_ptr<Node> prev = nullptr;
	while (node != nullptr)
	{
		auto next = node->parent;
		node->parent = prev;
		prev = node;
		node = next;
	}

	m_root = _new_root;
	SetProperties();
}

void Tree::SetProperties()
{
	//Extract::CountSubtree( m_root );
	leaves = Leaves(m_root);
	depth = Depth( m_root );
}

void Tree::Update()
{
	std::function<void( std::shared_ptr<Node> )> move = [&]( std::shared_ptr<Node> n ) {
		if (length( n->position - n->new_position ) < 2*speed) n->position = n->new_position;
		else n->position += normalize( n->new_position - n->position ) * speed;
		for (auto child : n->children) move( child );
	};
	move( m_root );
}

void Tree::Draw( )
{
	ofSetDrawBitmapMode( OF_BITMAPMODE_SIMPLE );
	
	// Draw radial circles
	ofNoFill();
	ofSetColor( 65 );
	for (int i = 0; i < depth - 1; i++)
	{
		ofDrawCircle( glm::vec3(0), 100 + i * 150);
	}

	// Draw nodes and edges
	ofFill( );
	ofSetColor( 255, 0, 0 );
	ofDrawCircle( m_root->position, radius );
	ofDrawBitmapStringHighlight( ofToString( m_root->vertex ), m_root->position + glm::vec3( 10, 10, -1 ) );
	ofSetColor( 255 );
	std::stack<std::shared_ptr<Node>> stack;
	for ( auto& child : m_root->children ) stack.push( child );

	while ( stack.empty( ) == false ) {
		auto node = stack.top( ); stack.pop( );
		auto parent = node->parent;
		static glm::vec3 sub = { 0, 0, -1 }; // To draw edge behind nodes
		ofSetColor( 123 );
		ofDrawLine( parent->position + sub, node->position + sub );

		ofSetColor( 255 );
		ofDrawCircle( node->position, radius );
		//ofDrawBitmapStringHighlight( ofToString( node->subtree_count ), node->position + glm::vec3( 10, 10, -1 ) );
		ofDrawBitmapStringHighlight( ofToString( node->vertex ), node->position + glm::vec3( 10, 10, -1 ) );

		for ( auto& child : node->children ) stack.push( child );
	}
}

} // DataVis
