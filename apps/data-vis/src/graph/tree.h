#pragma once

namespace DataVis
{
class Tree
{

public:
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

	Node& Root() { return m_root; }

private:
	Node m_root;
};

} // DataVis