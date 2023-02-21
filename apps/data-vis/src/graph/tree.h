#pragma once

namespace DataVis
{
class Tree
{

public:
	//--------------------------------------------------------------
	struct Node
	{
	public:
		int vertex;
		int subtree_count;
		std::shared_ptr <Node> parent;
		std::vector<std::shared_ptr<Node>> children;

		glm::vec3 position;
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
		auto& child0 = m_root->children[0];
		child0->children.push_back(std::make_shared<Node>());
		child0->children.push_back(std::make_shared<Node>());
		auto& child01 = child0->children[1];
		child01->children.push_back(std::make_shared<Node>());
		child01->children.push_back(std::make_shared<Node>());
		auto& child2 = m_root->children[2];
		child2->children.push_back(std::make_shared<Node>());
		child2->children.push_back(std::make_shared<Node>());
	}

	// This way we can iterate over the tree, without making a copy
	std::shared_ptr<Node> Root() { return m_root; }

private:
	std::shared_ptr<Node> m_root;
};

} // DataVis