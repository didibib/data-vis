#include "precomp.h"

namespace DataVis
{
	Tree Tree::Extractor::MSP( Graph& _graph, int _root )
	{
		// Implement Prim's Algorithm
		// https://www.wikiwand.com/en/Prim%27s_algorithm
		auto& vertices = _graph.Vertices();

		// Keep track of the parent of each vertex so we can construct a tree after
		std::vector<int> parents;
		// Keep track of which vertex are already processed
		std::vector<bool> included;
		// Keep track of the minimum edge cost of a vertex
		std::vector<float> costs;
		// Keep track of the actual minimum edge associated with the cost
		std::vector<Edge*> edges;
		parents.resize( vertices.size() );
		included.resize( vertices.size() );
		costs.resize( vertices.size() );
		// Initialize all but the root as infinite, so that the root is picked first
		// Make parent of root -1
		for (int i = 0; i < vertices.size(); i++)
		{
			if (i != _root) costs[i] = 1e30f;
			else parents[i] = -1;
		}

		// Graph will have |vertices| nodes
		for (int i = 0; i < vertices.size(); i++)
		{
			// Argmin for costs
			float min = 1e30;
			int idx;
			for (int v = 0; v < vertices.size(); v++)
			{
				if (!included[v] && costs[v] < min)
				{
					min = costs[v];
					idx = v;
				}
			}

			// Add lowest vertex
			included[idx] = true;

			// Update outgoing edges from this vertex
			auto& out_edges = vertices[idx].m_out_edges;
			for (int j = 0; j < out_edges.size(); j++)
			{
				int v = out_edges[j].m_target;
				if (!included[v] && out_edges[j].get_property().weight < costs[v])
				{
					// Found a cheaper edge to v
					parents[v] = idx;
					costs[v] = out_edges[j].get_property().weight;
				}
			}
		}

		// Construct a tree
		Tree t;
		t.m_root = std::make_shared<Node>( Node{ _root, 0, nullptr, {}, vertices[_root].m_property.current_position } );

		// Construct recursive lambda to create the tree
		std::function<void( std::shared_ptr<Node> )> make_tree;
		make_tree = [&]( std::shared_ptr<Node> n )
		{
			// Look through all vertices which have parent == n.vertex
			for (int i = 0; i < vertices.size(); i++)
				if (parents[i] == n->vertex)
				{
					// Add node to n.children
					auto child = std::make_shared<Node>( Node{ i, 0, n, {}, vertices[i].m_property.current_position });
					n->children.push_back( child );
					make_tree( child );
				}
		};
		make_tree( t.m_root );

		// Construct recursive lambda to count all children in subtree and update node.subtree_count of each node in the tree
		std::function<int( std::shared_ptr<Node> )> count_subtree;
		count_subtree = [&]( std::shared_ptr<Node> n)
		{
			int count = 1;
			for (auto child : n->children)
				count += count_subtree( child );
			n->subtree_count = count;
			return count;
		};
		count_subtree( t.m_root );

		return t;
	}

	void Tree::Draw()
	{
		ofFill();
		ofSetColor( 255 );
		ofSetDrawBitmapMode( OF_BITMAPMODE_SIMPLE );
		ofDrawCircle( m_root->position, radius );
		ofDrawBitmapStringHighlight( ofToString( m_root->subtree_count ), m_root->position + glm::vec3( 10, 10, -1 ) );
		std::stack<std::shared_ptr<Node>> stack;
		for (auto& child : m_root->children) stack.push( child );

		while (stack.empty() == false)
		{
			auto node = stack.top(); stack.pop();
			auto parent = node->parent;
			static glm::vec3 sub = { 0, 0, -1 }; // To draw edge behind nodes
			ofSetColor( 123 );
			ofDrawLine( parent->position + sub, node->position + sub );

			ofSetColor( 255 );
			ofDrawCircle( node->position, radius );
			ofDrawBitmapStringHighlight( ofToString( node->subtree_count ), node->position + glm::vec3( 10, 10, -1 ) );

			for (auto& child : node->children) stack.push( child );
		}
	}

} // DataVis
