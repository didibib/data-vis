#pragma once

namespace DataVis
{
class Tree
{
	class Node
	{
		int vertex;
		int subtree_count;
		std::shared_ptr <Node> parent;
		std::vector<std::shared_ptr<Node>> children;
	};

	class Extractor 
	{
		static Tree MSP(Graph&);
	};

	Node m_root;

public:
	Node& Root() { return m_root; }
};

} // DataVis