#pragma once

namespace DataVis
{
class Tree
{

public:
	//--------------------------------------------------------------
	class Node
	{
	public:
		int vertex;
		int subtree_count;
		std::shared_ptr <Node> parent;
		std::vector<std::shared_ptr<Node>> children;
	};

	class Extractor
	{
		static Tree MSP(Graph&);
	};

	//--------------------------------------------------------------
	Tree() {
		m_root = std::make_shared<Node>();
		m_root->children.push_back(std::make_shared<Node>());
		m_root->children.push_back(std::make_shared<Node>());
		m_root->children.push_back(std::make_shared<Node>());
	}

	// This way we can iterate over the tree, without making a copy
	std::shared_ptr<Node> Root() { return m_root; }

private:
	std::shared_ptr<Node> m_root;
};

} // DataVis